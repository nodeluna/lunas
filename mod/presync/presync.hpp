#pragma once

#include <expected>
#include <variant>

#include "content/content.hpp"
#include "error/error.hpp"
#include "input_path/input_path.hpp"

namespace lunas
{
	std::expected<std::variant<lunas::content, std::monostate>, lunas::error> presync_operations(const lunas::parsed_data& cliopts);
}
