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
import lunas.config.options.functions;
import lunas.config.options;
import lunas.config.filler;
import lunas.cppfs;
import lunas.ipath;
import lunas.stdout;
import lunas.error;

import luco;

export namespace lunas
{
	namespace config_file
	{
		std::string				    config_dir = std::getenv("HOME") + std::string("/.config/lunas/");
		std::string				    file_name  = std::string("lunas.luco");
		std::expected<std::monostate, lunas::error> make_demo_config(lunas::config::options& options);
		std::expected<std::monostate, lunas::error>
		preset(const std::string& name, lunas::config::options& options,
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

namespace lunas
{
	namespace config_file
	{
		std::expected<std::monostate, lunas::error> make_demo_config(lunas::config::options& options)
		{
			std::string	config_file = config_dir + file_name;
			std::error_code ec;
			if (fs::exists(config_dir, ec) != true)
			{
				auto ok = lunas::cppfs::mkdir(config_dir, options.dry_run);
				if (not ok)
				{
					std::string err = "couldn't create config dir '" + config_dir + "', " + ec.message();
					return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
				}
			}

			if (fs::exists(config_file, ec) == true)
			{
				std::string err = "couldn't create a demo config, a config file already exists";
				return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
			}

			if (ec.value() != 0)
			{
				std::string err = "couldn't check config file '" + config_file + "', " + ec.message();
				return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
			}

			std::fstream file;
			file.open(config_file, std::ios::out);
			if (file.is_open() == false)
			{
				std::string err = "couldn't open config file '" + config_file + "', " + ec.message();
				return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
			}

			file << DEMO_CONFIG;
			if (file.bad() == true)
			{
				std::string err = "error writing config file '" + config_file + "', " + ec.message();
				return std::unexpected(lunas::error(err, lunas::error_type::config_demo_config_error));
			}

			file.close();
			if (file.is_open() == true)
			{
				lunas::warnln("couldn't close config file '{}', {}", config_file, ec.message());
			}

			std::print(":: wrote a demo config file to '{}'", config_file);
			return std::monostate();
		}

		bool is_num(const std::string& x)
		{
			return std::all_of(x.begin(), x.end(),
					   [](char c)
					   {
						   return std::isdigit(c);
					   });
		}

#ifdef REMOTE_ENABLED
		std::expected<struct lunas::ipath::remote_path, lunas::error> fill_remote_path(const luco::node&  nest,
											       const std::string& name)
		{
			struct lunas::ipath::remote_path remote_path;
			auto				 rpaths_options = config::get_rpaths_options();

			for (const auto& [option, value] : *nest.as_object())
			{
				if (auto itr1 = rpaths_options.find("-" + option); itr1 != rpaths_options.end())
				{
					remote_path.srcdest	    = itr1->second();
					remote_path.session_data.ip = value.stringify();
				}
				else if (option == "N" || option == "port")
				{
					if (value.is_number())
					{
						remote_path.session_data.port = value.as_number();
					}
					else
					{
						std::string err =
						    "nest '" + name + "': port '" + value.stringify() + "' isn't a valid number";
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
					}
				}
				else if (option == "pw" || option == "password")
				{
					remote_path.session_data.pw = value.stringify();
				}
			}

			if (remote_path.session_data.ip.empty())
			{
				std::string err = "remote path for nest '" + name + "' isn't provided";
				return std::unexpected(lunas::error(err, lunas::error_type::config_missing_option));
			}

			return remote_path;
		}
#endif // REMOTE_ENABLED

		std::expected<std::monostate, lunas::error>
		config_fill(const luco::node& nest, const std::string& name, lunas::config::options& options,
			    std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>& ipaths)
		{
			auto lpaths_options = config::get_lpaths_options();
			auto rpaths_options = config::get_rpaths_options();
			auto onoff_options  = config::get_onoff_options();
			auto misc_options   = config::get_misc_options();
			auto info	    = config::get_info_options();

			for (const auto& [key, value] : *nest.as_object())
			{
				if (auto itr = onoff_options.find("-" + key); itr != onoff_options.end())
				{
					if (not value.is_boolean())
					{
						std::string err = "wrong value '" + value.stringify() + "' for on/off option '" + key + "'";
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument));
					}

					itr->second(value.as_boolean() ? "on" : "off", options);
				}
				else if (auto itr1 = lpaths_options.find("-" + key); itr1 != lpaths_options.end())
				{
					if (value.is_array())
					{
						for (const auto& val : *value.as_array())
						{
							ipaths.emplace_back(itr1->second(val.stringify()));
						}
					}
					else if (value.is_object())
					{
						std::string err =
						    "wrong value type for option '" + key + "' expected array or simple value";
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument));
					}
					else
					{
						ipaths.emplace_back(itr1->second(value.stringify()));
					}
				}
				else if (auto itr2 = misc_options.find("-" + key); itr2 != misc_options.end())
				{
					auto ok = itr2->second(value.stringify(), options);
					if (not ok)
					{
						return std::unexpected(ok.error());
					}
				}
#ifdef REMOTE_ENABLED
				else if (auto itr3 = rpaths_options.find("-" + key); itr3 != rpaths_options.end())
				{
					if (value.is_array())
					{
						for (const auto& val : *value.as_array())
						{
							struct lunas::ipath::remote_path remote_path;
							remote_path.session_data.ip = val.stringify();
							remote_path.srcdest	    = itr3->second();
							ipaths.emplace_back(std::move(remote_path));
						}
					}
					else if (value.is_object())
					{
						std::string err =
						    "wrong value type for option '" + key + "' expected array or simple value";
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument));
					}
					else
					{
						struct lunas::ipath::remote_path remote_path;
						remote_path.session_data.ip = value.stringify();
						remote_path.srcdest	    = itr3->second();
						ipaths.emplace_back(std::move(remote_path));
					}
				}
				else if (key == "remote")
				{
					auto handle_rpath = [&](const luco::node& val) -> std::expected<std::monostate, lunas::error>
					{
						auto remote_path = fill_remote_path(val, name);
						if (remote_path)
						{
							ipaths.emplace_back(std::move(remote_path.value()));
							return std::monostate();
						}
						else
						{
							return std::unexpected(remote_path.error());
						}
					};

					if (value.is_array())
					{
						for (const auto& val : *value.as_array())
						{
							if (not val.is_object())
							{
								std::string err = "wrong value type for option '" + key +
										  "' expected objects inside the array";
								return std::unexpected(
								    lunas::error(err, lunas::error_type::config_invalid_argument));
							}
							auto ok = handle_rpath(val);
							if (not ok)
							{
								return ok;
							}
						}
					}
					else if (value.is_object())
					{
						auto ok = handle_rpath(value);
						if (not ok)
						{
							return ok;
						}
					}
					else
					{
						std::string err = "wrong type '" + value.type_name() + "' for option '" + key +
								  "'. expected array or object";
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument));
					}
				}
#endif // REMOTE_ENABLED
				else if (auto itr4 = info.find("-" + key); itr4 != info.end())
				{
					itr4->second();
				}
				else
				{
					return std::unexpected(lunas::error("nest '" + name + "': wrong option '" + key + "'",
									    lunas::error_type::config_invalid_option));
				}
			}

			return std::monostate();
		}

		std::expected<std::monostate, lunas::error>
		preset(const std::string& name, lunas::config::options& options,
		       std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>& ipaths)
		{
			if (name == "DEMO_CONFIG")
			{
				return make_demo_config(options);
			}

			try
			{
				luco::node node = luco::parser::parse(std::filesystem::path(config_dir + file_name));
				luco::expected<std::reference_wrapper<luco::node>, luco::error> nest = node.try_at(name);
				if (not nest)
				{
					std::string err = "preset '" + name + "' doesn't exist";
					return std::unexpected(lunas::error(err, lunas::error_type::config_preset_name_doesnt_exists));
				}

				lunas::println(options.quiet, ":: running preset '{}'", name);

				return config_fill(nest.value().get(), name, options, ipaths);
			}
			catch (const std::exception& e)
			{
				return std::unexpected(lunas::error(e.what(), lunas::error_type::config_file_parsing_error));
			}
			catch (const luco::error& e)
			{
				return std::unexpected(lunas::error(e.what(), lunas::error_type::config_file_parsing_error));
			}
		}
	}
}
