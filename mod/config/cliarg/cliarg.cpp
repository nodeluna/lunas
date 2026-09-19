#include <cctype>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <expected>
#include <variant>

#include "cliarg.hpp"
#include "../options.hpp"
#include "../options_functions.hpp"
#include "input_path/input_path.hpp"
#include "error/error.hpp"
#include "kvoption.hpp"

namespace lunas
{
	namespace cliarg
	{
		bool is_num(const std::string& x)
		{
			return std::all_of(x.begin(), x.end(),
					   [](char c)
					   {
						   return std::isdigit(c);
					   });
		}

		std::expected<std::monostate, lunas::error> next_arg_exists(const int& argc, const char* argv[], int i)
		{
			if ((argc - 1) == i)
			{
				return std::unexpected(
				    lunas::error(std::format("argument for option '{}' wasn't provided, exiting", argv[i]),
						 lunas::error_type::config_missing_argument));
			}

			if (argv[i + 1][0] == '-')
			{
				return std::unexpected(
				    lunas::error(std::format("invalid argument '{}' for option '{}', exiting", argv[i + 1], argv[i]),
						 lunas::error_type::config_invalid_argument));
			}

			return std::monostate();
		}

#ifdef REMOTE_ENABLED
		std::expected<struct lunas::ipath::remote_path, lunas::error>
		fill_remote_path(const int& argc, const char* argv[], int& index, const lunas::ipath::srcdest& srcdest)
		{
			struct lunas::ipath::remote_path rpath;
			rpath.srcdest	      = srcdest;
			rpath.session_data.ip = argv[index + 1];

			index++;
			std::string options;
			while (index++ != (argc - 1))
			{
				if (options[0] == '-')
				{
					index--;
					break;
				}

				options += argv[index];
				options += " ";
			}

			std::map<std::string, luco::node> kv_map = {
			    {	     "N",		  luco::node(0)},
			    {    "port",	     luco::node(0)},
			    {	     "pw", luco::node(std::string())},
			    {"password", luco::node(std::string())},
			};

			std::expected<std::vector<std::pair<std::string, luco::node>>, lunas::error> kv_mapped =
			    kvoption_parser(options, kv_map);

			if (not kv_mapped)
			{
				return std::unexpected(kv_mapped.error());
			}

			for (auto& [k, v] : kv_mapped.value())
			{
				if (k == "N" || k == "port")
				{
					rpath.session_data.port = v.as_number();
				}
				else if (k == "pw" || k == "password")
				{
					rpath.session_data.pw = v.as_string();
				}
			}

			return rpath;
		}
#endif // REMOTE_ENABLED
		using expect	= std::expected<std::monostate, lunas::error>;
		using options	= lunas::config::options;
		using paths_vec = std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>;

		std::expected<struct cliopts, lunas::error>
		fillopts(const int& argc, const char* argv[],
			 std::function<expect(const std::string&, options&, paths_vec&)> config_file_preset)
		{

			auto	       lpaths_options = lunas::config::get_lpaths_options();
			auto	       rpaths_options = lunas::config::get_rpaths_options();
			auto	       onoff_options  = lunas::config::get_onoff_options();
			auto	       misc_options   = lunas::config::get_misc_options();
			auto	       info	      = lunas::config::get_info_options();
			struct cliopts cliopts;

			auto	       ok = config_file_preset("global", cliopts.options, cliopts.ipaths);
			if (not ok)
			{
				return std::unexpected(ok.error());
			}

			for (int index = 1; index < argc; index++)
			{
				std::string option = argv[index];
				if (option.size() >= 2 && option.substr(0, 2) == "--")
				{
					option = option.substr(1, option.size());
				}

				if (option == "-c" || option == "-config")
				{
					{
						auto ok = next_arg_exists(argc, argv, index);
						if (not ok)
						{
							return std::unexpected(ok.error());
						}
					}
					{
						std::string argument = argv[index + 1];
						auto	    ok	     = config_file_preset(argument, cliopts.options, cliopts.ipaths);
						if (not ok)
						{
							return std::unexpected(ok.error());
						}
					}
					index++;
				}
				else if (auto itr0 = lpaths_options.find(option); itr0 != lpaths_options.end())
				{
					auto ok = next_arg_exists(argc, argv, index);
					if (not ok)
					{
						return std::unexpected(ok.error());
					}
					std::string argument = argv[index + 1];
					cliopts.ipaths.emplace_back(itr0->second(argument));
					index++;
#ifdef REMOTE_ENABLED
				}
				else if (auto itr1 = rpaths_options.find(option); itr1 != rpaths_options.end())
				{
					auto ok = next_arg_exists(argc, argv, index);
					if (not ok)
					{
						return std::unexpected(ok.error());
					}
					lunas::ipath::srcdest srcdest	  = itr1->second();
					auto		      remote_path = fill_remote_path(argc, argv, index, srcdest);
					if (not remote_path)
					{
						return std::unexpected(remote_path.error());
					}
					cliopts.ipaths.emplace_back(std::move(remote_path.value()));
#endif // REMOTE_ENABLED
				}
				else if (auto itr2 = onoff_options.find(option); itr2 != onoff_options.end())
				{
					auto	    ok = next_arg_exists(argc, argv, index);
					std::string argument;
					if (not ok)
					{
						argument = "on";
					}
					else
					{
						argument = argv[index + 1];
						index++;
					}
					auto rv = itr2->second(argument, cliopts.options);
					if (not rv)
					{
						std::string err =
						    std::format("wrong argument '{}' for on/off option '{}'", argument, option);
						return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_argument));
					}
				}
				else if (auto itr3 = misc_options.find(option); itr3 != misc_options.end())
				{
					auto ok = next_arg_exists(argc, argv, index);
					if (not ok)
					{
						return std::unexpected(ok.error());
					}
					std::string argument = argv[index + 1];
					auto	    ok2	     = itr3->second(argument, cliopts.options);
					if (not ok2)
					{
						return std::unexpected(ok2.error());
					}
					index++;
				}
				else if (auto itr4 = info.find(option); itr4 != info.end())
				{
					itr4->second();
				}
				else
				{
					std::string err = std::format("option '{}' wasn't recognized, read the man page", argv[index]);
					return std::unexpected(lunas::error(err, lunas::error_type::config_invalid_option));
				}
			}

			return cliopts;
		}
	}
}
