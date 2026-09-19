#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <expected>
#	include <string>
#	include <vector>
#endif

#include "error/error.hpp"

namespace lunas
{
	std::expected<std::vector<std::pair<size_t, size_t>>, lunas::error> hook_parser(const std::string& command);
}
