module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <cctype>
#	include <cstdlib>
#	include <iostream>
#	include <string>
#	include <vector>
#	include <functional>
#	include <optional>
#	include <unordered_map>
#	include <algorithm>
#	include <expected>
#	include <variant>
#	include <print>
#endif

export module lunas.config.cliarg;
import lunas.config.options;
import lunas.config.options.functions;
import lunas.about;
import lunas.sftp;
import lunas.ipath;
import lunas.error;

export namespace lunas {
	namespace cliarg {
		struct cliopts {
				std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>> ipaths;
				lunas::config::options									     options;
				std::vector<std::string>								     presets;
		};

		using expect = std::expected<std::monostate, lunas::error>;
		using options = lunas::config::options;
		using paths_vec = std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>;

		std::expected<struct cliopts, lunas::error> fillopts(const int& argc, const char* argv[],
				std::function<expect(const std::string&, options&, paths_vec&)> config_file_preset);
	}
}

namespace lunas {
	namespace cliarg {
		bool is_num(const std::string& x) {
			return std::all_of(x.begin(), x.end(), [](char c) { return std::isdigit(c); });
		}

		std::expected<std::monostate, lunas::error> next_arg_exists(const int& argc, const char* argv[], int i) {
			if ((argc - 1) == i)
				return std::unexpected(
				    lunas::error(std::format("argument for option '{}' wasn't provided, exiting", argv[i]),
					lunas::error_type::config_missing_argument));

			if (argv[i + 1][0] == '-')
				return std::unexpected(
				    lunas::error(std::format("invalid argument '{}' for option '{}', exiting", argv[i + 1], argv[i]),
					lunas::error_type::config_invalid_argument));

			return std::monostate();
		}

#ifdef REMOTE_ENABLED
		std::expected<struct lunas::ipath::remote_path, lunas::error> fill_remote_path(
		    const int& argc, const char* argv[], int& index, const lunas::ipath::srcdest& srcdest) {
			struct lunas::ipath::remote_path rpath;
			rpath.srcdest	      = srcdest;
			rpath.session_data.ip = argv[index + 1];

			index++;
			while (index++ != (argc - 1)) {
				std::string option = argv[index];
				if (option.find('=') != option.npos)
					option.resize(option.find('='));
				if (option[0] == '-') {
					index--;
					break;
				}
				std::string argument = argv[index];
				argument	     = argument.substr(argument.find('=') + 1, argument.size());

				if ((option == "N" || option == "port") && is_num(argument) == false) {
					std::string err = std::format("argument '{}' for option '{}' isn't a number", argument, option);
					return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
				} else if (option == "N" || option == "port") {
					int port = std::stoi(argument);
					if (port < 0) {
						std::string err = std::format("port number '{}' for '{}' can't be negative ",
						    std::to_string(port), rpath.session_data.ip);
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument_type));
					}
					rpath.session_data.port = port;
				} else if (option == "pw" || option == "password") {
					rpath.session_data.pw = argument;
				} else {
					std::string err = std::format("option '{}' isn't recognized", option);
					return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_option));
				}
			}

			return rpath;
		}
#endif // REMOTE_ENABLED
		using expect = std::expected<std::monostate, lunas::error>;
		using options = lunas::config::options;
		using paths_vec = std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>;
		std::expected<struct cliopts, lunas::error> fillopts(const int& argc, const char* argv[],
				std::function<expect(const std::string&, options&, paths_vec&)> config_file_preset) {

			auto	       lpaths_options = lunas::config::get_lpaths_options();
			auto	       rpaths_options = lunas::config::get_rpaths_options();
			auto	       onoff_options  = lunas::config::get_onoff_options();
			auto	       misc_options   = lunas::config::get_misc_options();
			auto	       info	      = lunas::config::get_info_options();
			struct cliopts cliopts;

			auto ok = config_file_preset("global", cliopts.options, cliopts.ipaths);
			if (not ok)
				return std::unexpected(ok.error());

			for (int index = 1; index < argc; index++) {
				std::string option = argv[index];

				if (option == "-c" || option == "--config") {
					{
						auto ok = next_arg_exists(argc, argv, index);
						if (not ok)
							return std::unexpected(ok.error());
					}
					{
						std::string argument = argv[index + 1];
						auto ok = config_file_preset(argument, cliopts.options, cliopts.ipaths);
						if (not ok)
							return std::unexpected(ok.error());
					}
					index++;
				} else if (auto itr0 = lpaths_options.find(option); itr0 != lpaths_options.end()) {
					auto ok = next_arg_exists(argc, argv, index);
					if (not ok)
						return std::unexpected(ok.error());
					std::string argument = argv[index + 1];
					cliopts.ipaths.push_back(itr0->second(argument));
					index++;
#ifdef REMOTE_ENABLED
				} else if (auto itr1 = rpaths_options.find(option); itr1 != rpaths_options.end()) {
					auto ok = next_arg_exists(argc, argv, index);
					if (not ok)
						return std::unexpected(ok.error());
					lunas::ipath::srcdest srcdest	  = itr1->second();
					auto		      remote_path = fill_remote_path(argc, argv, index, srcdest);
					if (not remote_path)
						return std::unexpected(remote_path.error());
					cliopts.ipaths.push_back(std::move(remote_path.value()));
#endif // REMOTE_ENABLED
				} else if (auto itr2 = onoff_options.find(option); itr2 != onoff_options.end()) {
					auto	    ok = next_arg_exists(argc, argv, index);
					std::string argument;
					if (not ok)
						argument = "on";
					else {
						argument = argv[index + 1];
						index++;
					}
					auto rv = itr2->second(argument, cliopts.options);
					if (not rv) {
						std::string err =
						    std::format("wrong argument '{}' for on/off option '{}'", argument, option);
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument));
					}
				} else if (auto itr3 = misc_options.find(option); itr3 != misc_options.end()) {
					auto ok = next_arg_exists(argc, argv, index);
					if (not ok)
						return std::unexpected(ok.error());
					std::string argument = argv[index + 1];
					auto	    ok2	     = itr3->second(argument, cliopts.options);
					if (not ok2) {
						return std::unexpected(ok2.error());
					}
					index++;
				} else if (auto itr4 = info.find(option); itr4 != info.end())
					itr4->second();
				else {
					std::string err = std::format("option '{}' wasn't recognized, read the man page", option);
					return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_option));
				}
			}

			return cliopts;
		}
	}
}
