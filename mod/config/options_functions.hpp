#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <unordered_map>
#	include <functional>
#	include <expected>
#	include <variant>
#endif

#include "filler.hpp"
#include "input_path/input_path.hpp"
#include "options.hpp"
#include "error/error.hpp"

namespace lunas
{
	namespace config
	{
		using onoff_func = std::function<std::expected<std::monostate, lunas::error>(std::string, lunas::config::options&)>;

		std::unordered_map<std::string, onoff_func>						     get_onoff_options();

		std::unordered_map<std::string, std::function<struct lunas::ipath::local_path(std::string)>> get_lpaths_options();

		std::unordered_map<std::string, std::function<lunas::ipath::srcdest(void)>>		     get_rpaths_options();

		using misc_func = std::function<std::expected<std::monostate, lunas::error>(std::string, lunas::config::options&)>;

		std::unordered_map<std::string, misc_func>		   get_misc_options();

		std::unordered_map<std::string, std::function<void(void)>> get_info_options();
	}
}
