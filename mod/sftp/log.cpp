#include <string_view>
#include <string>
#include <format>

#include "log.hpp"

namespace fmt
{
	std::string err_path(const std::string_view& error, const std::string_view& path)
	{
		return std::format("{} '{}'", error, path);
	}

	std::string err_path(const std::string_view& error, const std::string_view& path, const std::string_view& reason)
	{
		return std::format("\x1b[1;31m-[X] {} '{}', {}\x1b[0m\n", error, path, reason);
	}
}
