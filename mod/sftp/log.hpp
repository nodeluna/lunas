#pragma once

#include <string_view>
#include <string>

namespace fmt
{
	std::string err_path(const std::string_view& error, const std::string_view& path);
	std::string err_path(const std::string_view& error, const std::string_view& path, const std::string_view& reason);
}
