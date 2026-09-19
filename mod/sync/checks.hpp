#pragma once

#include <cassert>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <set>
#	include <expected>
#	include <variant>
#	include <vector>
#	include <ctime>
#	include <string>
#	include <format>
#	include <filesystem>
#endif

#include "types.hpp"
#include "copy/copy.hpp"
#include "error/error.hpp"
#include "input_path/input_path.hpp"
#include "file_table/file_table.hpp"
#include "file_types/file_types.hpp"
#include "stdout/stdout.hpp"
#include "file/file.hpp"
#include "stats/stats.hpp"

namespace lunas
{
	std::expected<size_t, lunas::error>	    get_src(const lunas::file_table& file_table, const struct lunas::parsed_data& data);

	std::expected<std::monostate, lunas::error> check_dest(const struct file_metadata<std::filesystem::path>& src,
							       const struct file_metadata<std::filesystem::path>& dest,
							       struct lunas::parsed_data&			  data);
}
