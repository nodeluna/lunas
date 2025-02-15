module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string>
#	include <print>
#	include <string>
#	include <fstream>
#	include <filesystem>
#	include <expected>
#	include <cstdlib>
#	include <cstring>
#	include <cerrno>
#	include <unordered_map>
#	include <map>
#	include <variant>
#	include <algorithm>
#endif

export module lunas.config.file:manager;
import :luco;
import lunas.config.options.functions;
import lunas.config.options;
import lunas.config.filler;
import lunas.cppfs;
import lunas.ipath;
import lunas.stdout;
import lunas.error;

export namespace lunas {
	namespace config_file {
		std::string				    config_dir = std::getenv("HOME") + std::string("/.config/lunas/");
		std::string				    file_name  = std::string("lunas.luco");
		std::expected<std::monostate, lunas::error> make_demo_config(lunas::config::options& options);
		std::expected<std::monostate, lunas::error> preset(const std::string& name, lunas::config::options& options,
		    std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>& ipaths);
	}
}

#define DEMO_CONFIG                 \
	"global{\n"                 \
	"\t#mkdir = on\n"           \
	"\t#compression = on\n"     \
	"\t#resume = on\n"          \
	"\t#progress = on\n"        \
	"\t#update = on\n"          \
	"\t#minimum-space = 1gib\n" \
	"\t#attributes = mtime\n"   \
	"}\n"                       \
	"luna {\n"                  \
	"\tpath = /path/to/dir1\n"  \
	"\tpath = /path/to/dir2\n"  \
	"\tpath = /path/to/dir3\n"  \
	"\tremote {\n"              \
	"\t\tr = user@ip:dir\n"     \
	"\t\tport = 22\n"           \
	"\t}\n"                     \
	"}\n"

namespace fs = std::filesystem;

namespace lunas {
	namespace config_file {
		std::expected<std::monostate, lunas::error> make_demo_config(lunas::config::options& options) {
			std::string	config_file = config_dir + file_name;
			std::error_code ec;
			if (fs::exists(config_dir, ec) != true) {
				auto ok = lunas::cppfs::mkdir(config_dir, options.dry_run);
				if (not ok) {
					std::string err = "couldn't create config dir '" + config_dir + "', " + ec.message();
					return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
				}
			}

			if (fs::exists(config_file, ec) == true) {
				std::string err = "couldn't create a demo config, a config file already exists";
				return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
			}

			if (ec.value() != 0) {
				std::string err = "couldn't check config file '" + config_file + "', " + ec.message();
				return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
			}

			std::fstream file;
			file.open(config_file, std::ios::out);
			if (file.is_open() == false) {
				std::string err = "couldn't open config file '" + config_file + "', " + ec.message();
				return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
			}

			file << DEMO_CONFIG;
			if (file.bad() == true) {
				std::string err = "error writing config file '" + config_file + "', " + ec.message();
				return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
			}

			file.close();
			if (file.is_open() == true)
				lunas::warn("couldn't close config file '{}', {}", config_file, ec.message());

			std::print(":: wrote a demo config file to '{}'", config_file);
			return std::monostate();
		}

#ifdef REMOTE_ENABLED
		bool is_num(const std::string& x) {
			return std::all_of(x.begin(), x.end(), [](char c) { return std::isdigit(c); });
		}

		std::expected<struct lunas::ipath::remote_path, lunas::error> fill_remote_path(
		    const std::multimap<std::string, std::string>& nest, std::multimap<std::string, std::string>::iterator& it,
		    const std::string& name) {
			std::string			 nest_path = it->first;
			size_t				 nest_size = nest_path.size();
			struct lunas::ipath::remote_path remote_path;

			auto rpaths_options = config::get_rpaths_options();

			while (it != nest.end() && it->first.find(nest_path) != it->first.npos) {
				std::string option = it->first.substr(nest_size, it->first.size());
				if (auto itr1 = rpaths_options.find(option); itr1 != rpaths_options.end()) {
					remote_path.srcdest	    = itr1->second();
					remote_path.session_data.ip = it->second;
				} else if (option == "N" || option == "port") {
					if (is_num(it->second))
						remote_path.session_data.port = std::stoi(it->second);
					else {
						std::string err = "nest '" + name + "': port '" + it->second + "' isn't a valid number";
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
					}
				} else if (option == "pw" || option == "password")
					remote_path.session_data.pw = it->second;
				++it;
			}

			if (it != nest.begin())
				--it;
			if (remote_path.session_data.ip.empty()) {
				std::string err = "remote path for nest'" + name + "' isn't provided";
				return std::unexpected(lunas::error(err, lunas::error_type::config_missing_option));
			}

			return remote_path;
		}
#endif // REMOTE_ENABLED

		std::expected<std::monostate, lunas::error> config_fill(std::multimap<std::string, std::string>& nest,
		    const std::string& name, lunas::config::options& options,
		    std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>& ipaths) {
			auto lpaths_options = config::get_lpaths_options();
			auto rpaths_options = config::get_rpaths_options();
			auto onoff_options  = config::get_onoff_options();
			auto misc_options   = config::get_misc_options();
			auto info	    = config::get_info_options();

			for (auto it = nest.begin(); it != nest.end(); ++it) {
				if (auto itr = onoff_options.find(it->first); itr != onoff_options.end()) {
					if (not itr->second(it->second, options)) {
						std::string err = "wrong value '" + it->second + "' for on/off option '" + it->first + "'";
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument));
					}
				} else if (auto itr1 = lpaths_options.find(it->first); itr1 != lpaths_options.end()) {
					ipaths.emplace_back(itr1->second(it->second));
				} else if (auto itr2 = misc_options.find(it->first); itr2 != misc_options.end()) {
					auto ok = itr2->second(it->second, options);
					if (not ok)
						return std::unexpected(ok.error());
				}
#ifdef REMOTE_ENABLED
				else if (auto itr3 = rpaths_options.find(it->first); itr3 != rpaths_options.end()) {
					struct lunas::ipath::remote_path remote_path;
					remote_path.session_data.ip = it->second;
					remote_path.srcdest	    = itr3->second();
					ipaths.emplace_back(std::move(remote_path));
				} else if (it->first.find("remote::") != it->first.npos && it->first.front() != '#') {
					auto remote_path = fill_remote_path(nest, it, name);
					if (remote_path)
						ipaths.emplace_back(std::move(remote_path.value()));
					else
						return std::unexpected(remote_path.error());
				}
#endif // REMOTE_ENABLED
				else if (auto itr4 = info.find(it->first); itr4 != info.end())
					itr4->second();
				else if (it->first.front() != '#')
					return std::unexpected(lunas::error("nest '" + name + "': wrong option '" + it->first + "'",
					    lunas::error_type::config_invalid_option));
			}

			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> preset(const std::string& name, lunas::config::options& options,
		    std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>& ipaths) {
			if (name == "DEMO_CONFIG")
				return make_demo_config(options);

			try {
				luco nests = luco(config_dir + file_name);
				nests.parse();
				auto itr = nests.find_preset(name);
				if (itr == nests.get_map().end()) {
					std::string err = "preset '" + name + "' doesn't exist";
					return std::unexpected(lunas::error(err, lunas::error_type::config_preset_name_doesnt_exists));
				}

				lunas::println(options.quiet, ":: running preset '{}'", name);

				auto nest = nests.preset(name);
				return config_fill(nest, name, options, ipaths);
			} catch (const std::exception& e) {
				return std::unexpected(lunas::error(e.what(), lunas::error_type::config_file_parsing_error));
			}
		}
	}
}
