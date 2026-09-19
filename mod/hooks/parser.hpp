#pragma once

#include <expected>
#include <string>
#include <vector>

#include "error/error.hpp"

namespace lunas
{
	std::expected<std::vector<std::pair<size_t, size_t>>, lunas::error> hook_parser(const std::string& command);
}
