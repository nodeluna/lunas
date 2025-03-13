module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <cctype>
#	include <expected>
#	include <string>
#	include <algorithm>
#	include <print>
#	include <variant>
#	include <filesystem>
#	include <unordered_map>
#	include <functional>
#endif

export module lunas.config.filler;
import lunas.about;
import lunas.ipath;
import lunas.config.options;
import lunas.sftp;
import lunas.path;
import lunas.stdout;
import lunas.error;
import lunas.file_types;

export namespace lunas {
	namespace config {
		namespace filler {
			bool is_num(const std::string& x) {
				return std::all_of(x.begin(), x.end(), [](char c) { return std::isdigit(c); });
			}

			bool is_num_decimal(const std::string& x) {
				bool has_decimal = false;
				return std::all_of(x.begin(), x.end(), [&](char c) {
					if (std::isdigit(c))
						return true;
					else if (c == '.' && not has_decimal) {
						has_decimal = true;
						return true;
					} else
						return false;
				});
			}

			struct lunas::ipath::local_path fill_local_path(const std::string& argument, const lunas::ipath::srcdest& srcdest) {
				struct lunas::ipath::local_path local_path;
				auto				absolute_path = lunas::path::absolute(argument);
				if (absolute_path)
					local_path.path = absolute_path.value();
				else
					local_path.path = std::filesystem::absolute(argument);
				local_path.srcdest = srcdest;
				return local_path;
			}

			struct lunas::ipath::local_path lpath_srcdest(const std::string& data) {
				return fill_local_path(data, lunas::ipath::srcdest::srcdest);
			}

			struct lunas::ipath::local_path lpath_src(const std::string& data) {
				return fill_local_path(data, lunas::ipath::srcdest::src);
			}

			struct lunas::ipath::local_path lpath_dest(const std::string& data) {
				return fill_local_path(data, lunas::ipath::srcdest::dest);
			}

			lunas::ipath::srcdest rpath_srcdest(void) {
				return lunas::ipath::srcdest::srcdest;
			}

			lunas::ipath::srcdest rpath_src(void) {
				return lunas::ipath::srcdest::src;
			}

			lunas::ipath::srcdest rpath_dest(void) {
				return lunas::ipath::srcdest::dest;
			}

			std::expected<std::monostate, lunas::error> compression_level(
			    const std::string& data, lunas::config::options& options) {
				if (is_num(data) == false) {
					std::string err = "argument '" + data + "' for option compression-level isn't a number";
					return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
				}

				int level = std::stoi(data);
				if (level > 9 || level <= 0) {
					std::string err = "compression level must be between 1-9. provided level '" + data + "'";
					return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
				}
				options.compression_level = level;

				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> minimum_space(
			    const std::string& data, lunas::config::options& options) {
				constexpr std::uintmax_t KiB  = 1024;
				constexpr std::uintmax_t MiB  = 1024 * KiB;
				constexpr std::uintmax_t GiB  = 1024 * MiB;
				constexpr std::uintmax_t TiB  = 1024 * GiB;
				constexpr std::uintmax_t PiB  = 1024 * TiB;
				std::string		 temp = data;

				std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c) { return std::tolower(c); });
				if (temp.find("kib") != data.npos)
					temp.resize(temp.find("kib"));
				else if (temp.find("mib") != temp.npos)
					temp.resize(temp.find("mib"));
				else if (temp.find("gib") != temp.npos)
					temp.resize(temp.find("gib"));
				else if (temp.find("tib") != temp.npos)
					temp.resize(temp.find("tib"));
				else if (temp.find("pib") != temp.npos)
					temp.resize(temp.find("pib"));
				else
					temp = data;

				if (is_num_decimal(temp) == false) {
					std::string err = "argument '" + data + "' for option minimum-space isn't valid";
					return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
				}

				double num = std::stod(data);
				temp	   = data;
				std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c) { return std::tolower(c); });

				if (temp.find("kib") != data.npos)
					options.minimum_space = num * KiB;
				else if (temp.find("mib") != temp.npos)
					options.minimum_space = num * MiB;
				else if (temp.find("gib") != temp.npos)
					options.minimum_space = num * GiB;
				else if (temp.find("tib") != temp.npos)
					options.minimum_space = num * TiB;
				else if (temp.find("pib") != temp.npos)
					options.minimum_space = num * PiB;
				else
					options.minimum_space = num;

				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> exclude(const std::string& data, lunas::config::options& options) {
				std::string path = data;
				lunas::path::pop_seperator(path);
				options.exclude.insert(path);

				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> exclude_pattern(
			    const std::string& data, lunas::config::options& options) {
				std::string path = data;
				lunas::path::pop_seperator(path);
				options.exclude_pattern.insert(path);

				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> mkdir(const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.mkdir = true;
				else if (data == "off")
					options.mkdir = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> progress(const std::string& data, lunas::config::options& options) {
				if (data == "on") {
					options.progress_bar = true;
				} else if (data == "off") {
					options.progress_bar = false;
				} else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> dry_run(const std::string& data, lunas::config::options& options) {
				if (data == "on") {
					std::println("--> dry run is on\n");
					options.dry_run = true;
				} else if (data == "off")
					options.dry_run = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> quiet(const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.quiet = true;
				else if (data == "off")
					options.quiet = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> verbose(const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.verbose = true;
				else if (data == "off")
					options.verbose = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> follow_symlink(
			    const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.follow_symlink = lunas::follow_symlink::yes;
				else if (data == "off")
					options.follow_symlink = lunas::follow_symlink::no;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> no_broken_link(
			    const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.no_broken_symlink = true;
				else if (data == "off")
					options.no_broken_symlink = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> resume(const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.resume = true;
				else if (data == "off")
					options.resume = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> compression(const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.compression = true;
				else if (data == "off")
					options.compression = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> attributes_uid(
			    const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.attributes_uid = true;
				else if (data == "off") {
					options.attributes_uid	     = false;
					options.attributes_uid_value = std::nullopt;
				} else if (is_num(data)) {
					options.attributes_uid	     = true;
					options.attributes_uid_value = std::stoi(data);
				} else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> attributes_gid(
			    const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.attributes_gid = true;
				else if (data == "off") {
					options.attributes_gid	     = false;
					options.attributes_gid_value = std::nullopt;
				} else if (is_num(data)) {
					options.attributes_gid	     = true;
					options.attributes_gid_value = std::stoi(data);
				} else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> attributes_own(
			    const std::string& data, lunas::config::options& options) {
				if (data == "on") {
					options.attributes_uid = true;
					options.attributes_gid = true;
				} else if (data == "off") {
					options.attributes_uid	     = false;
					options.attributes_gid	     = false;
					options.attributes_uid_value = std::nullopt;
					options.attributes_gid_value = std::nullopt;
				} else if (is_num(data)) {
					options.attributes_uid	     = true;
					options.attributes_uid_value = std::stoi(data);
					options.attributes_gid	     = true;
					options.attributes_gid_value = std::stoi(data);
				} else {
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				}

				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> attributes_atime(
			    const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.attributes_atime = true;
				else if (data == "off")
					options.attributes_atime = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> attributes_mtime(
			    const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.attributes_mtime = true;
				else if (data == "off")
					options.attributes_mtime = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> attributes_utimes(
			    const std::string& data, lunas::config::options& options) {
				if (data == "on") {
					options.attributes_atime = true;
					options.attributes_mtime = true;
				} else if (data == "off") {
					options.attributes_atime = false;
					options.attributes_mtime = false;
				} else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> attributes_all(
			    const std::string& data, lunas::config::options& options) {
				if (data == "on") {
					options.attributes_atime = true;
					options.attributes_mtime = true;
					options.attributes_uid	 = true;
					options.attributes_gid	 = true;
				} else if (data == "off") {
					options.attributes_atime = false;
					options.attributes_mtime = false;
					options.attributes_uid	 = false;
					options.attributes_gid	 = false;
				} else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> attributes(const std::string& data, lunas::config::options& options) {
				std::string temp;
				std::unordered_map<std::string, std::function<std::expected<std::monostate, lunas::error>(
								    const std::string& data, lunas::config::options& options)>>
				    attributes_options = {
					{	 "own",	attributes_own},
					{	 "uid",	attributes_uid},
					{	 "gid",	attributes_gid},
					{ "atime",  attributes_atime},
					{ "mtime",  attributes_mtime},
					{"utimes", attributes_utimes},
					{	 "all",	attributes_all},
				    };

				for (auto it = data.begin(); it != data.end(); ++it) {
					if (*it != ',' && *it != '=') {
						temp += *it;
						if (std::next(it) != data.end())
							continue;
					}

					auto itr = attributes_options.find(temp);
					if (itr != attributes_options.end()) {
						temp = "";
						if (*it == ',' || std::next(it) == data.end())
							itr->second("on", options);
						else if (*it == '=') {
							std::string arg;
							bool	    found_seperator = false;
							std::ranges::for_each(++it, data.end(), [&](const char c) {
								if (c != '=' && c != ',' && not found_seperator) {
									if (std::next(it) != data.end())
										++it;
									arg += c;
								} else
									found_seperator = true;
							});
							if (not itr->second(arg, options)) {
								std::string err = "invalid argument '" + arg +
										  "' for option --attributes " + itr->first + "=" + arg +
										  ". ";
								err += "valid arguments: --attributes " + itr->first + "=on/off";
								return std::unexpected(
								    lunas::error(err, lunas::error_type::config_invalid_argument_type));
							}
						}
					} else {
						std::string err = "invalid argument '" + temp + "' for option --attributes. ";
						err += "valid arguments: --attributes atime,mtime,utimes,uid,gid,own,all";
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
					}
				}

				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> fsync(const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.fsync = true;
				else if (data == "off")
					options.fsync = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> remove_extra(const std::string& data, lunas::config::options& options) {
				if (data == "on")
					options.remove_extra = true;
				else if (data == "off")
					options.remove_extra = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> update(const std::string& data, lunas::config::options& options) {
				if (data == "on") {
					options.update	 = true;
					options.rollback = false;
				} else if (data == "off")
					options.update = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> rollback(const std::string& data, lunas::config::options& options) {
				if (data == "on") {
					options.rollback = true;
					options.update	 = false;
				} else if (data == "off")
					options.rollback = false;
				else
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_argument));
				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> ssh_log(const std::string& data, lunas::config::options& options) {
				if (data == "nolog")
					options.ssh_log_level = lunas::ssh_log_level::no_log;
				else if (data == "warning")
					options.ssh_log_level = lunas::ssh_log_level::warning;
				else if (data == "protocol")
					options.ssh_log_level = lunas::ssh_log_level::protocol;
				else if (data == "packet")
					options.ssh_log_level = lunas::ssh_log_level::packet;
				else if (data == "functions")
					options.ssh_log_level = lunas::ssh_log_level::functions;
				else {
					lunas::warn("invalid ssh log level '{}'. valid levels [nolog, warning, protocol, packet, "
						    "functions] default is nolog",
					    data);
					exit(1);
				}

				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> timeout(const std::string& data, lunas::config::options& options) {
				if (is_num(data) == false) {
					std::string err = "argument '" + data + "' for option timeout isn't a number";
					return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
				}

				time_t level = std::stoull(data);
				if (level < 0) {
					std::string err = "timeout must be between > 0. provided level '" + data + "'";
					return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
				}
				options.timeout_sec = level;

				return std::monostate();
			}
		}

		namespace info {
			void author(void) {
				std::println("{}", lunas::about::author);
				exit(0);
			}

			void version(void) {
				std::println("{}", lunas::about::version);
				exit(0);
			}

			void license(void) {
				std::println("{}", lunas::about::license);
				exit(0);
			}

			void help(void) {
				std::println("{}", lunas::about::help);
				exit(0);
			}

			void meow(void) {
				std::println("");
				std::println("  ~-~ MeooOOoooooOOOooowwwwwW ~-~");
				std::println("");
			}
		}
	}
}
