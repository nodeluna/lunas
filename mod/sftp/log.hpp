#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string_view>
#	include <string>
#	include <print>
#	include <format>
#endif

namespace fmt
{
	std::string err_path(const std::string_view& error, const std::string_view& path);
	std::string err_path(const std::string_view& error, const std::string_view& path, const std::string_view& reason);
}
