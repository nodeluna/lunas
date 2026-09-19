#pragma once

#include <expected>
#include <variant>

#include "error/error.hpp"
#include "input_path/input_path.hpp"
#include "content/content.hpp"

namespace lunas
{
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data);
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data, lunas::content& content);
}
