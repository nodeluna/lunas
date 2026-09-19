#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <expected>
#	include <vector>
#	include <string>
#	include <print>
#	include <variant>
#	include <set>
#	include <cstdint>
#endif

#include "error/error.hpp"
#include "stdout/stdout.hpp"
#include "input_path/input_path.hpp"
#include "sftp/sftp.hpp"
#include "file_table/file_table.hpp"
#include "file_types/file_types.hpp"

namespace lunas
{
	namespace presync
	{
		std::expected<std::monostate, lunas::error>
		       input_paths_are_different(const std::vector<struct lunas::ipath::input_path>& ipaths);

		size_t to_be_synced_counter(const std::set<file_table>& conent);
	}
}
