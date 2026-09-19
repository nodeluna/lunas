#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <expected>
#	include <print>
#	include <variant>
#endif

#include "file/config_manager.hpp"
#include "input_path/input_path.hpp"
#include "stdout/stdout.hpp"
#include "error/error.hpp"
#include "path/path.hpp"
#include "cliarg/cliarg.hpp"

namespace lunas
{
	namespace config
	{
		std::expected<struct lunas::parsed_data, lunas::error> parse_cliarg(const int argc, const char* argv[]);
	}
}
