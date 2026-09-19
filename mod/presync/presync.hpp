#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <expected>
#	include <vector>
#	include <variant>
#	include <set>
#endif

#include "file_table/file_table.hpp"
#include "content/content.hpp"
#include "error/error.hpp"
#include "input_path/input_path.hpp"
#include "stdout/stdout.hpp"
#include "misc.hpp"
#include "fill_tree/fill_tree.hpp"

namespace lunas
{
	std::expected<std::variant<lunas::content, std::monostate>, lunas::error> presync_operations(const lunas::parsed_data& cliopts);
}
