#pragma once

#include <expected>

#include "input_path/input_path.hpp"
#include "error/error.hpp"

namespace lunas
{
	namespace config
	{
		std::expected<struct lunas::parsed_data, lunas::error> parse_cliarg(const int argc, const char* argv[]);
	}
}
