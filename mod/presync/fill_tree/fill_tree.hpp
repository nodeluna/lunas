#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <filesystem>
#	include <memory>
#	include <exception>
#	include <string>
#	include <set>
#	include <expected>
#	include <variant>
#endif

#include "types.hpp"
#include "error/error.hpp"
#include "file/file.hpp"
#include "file_table/file_table.hpp"
#include "filter/filter.hpp"
#include "stdout/stdout.hpp"

namespace lunas
{
	namespace presync
	{
		std::expected<std::monostate, lunas::error> readdir(std::set<lunas::file_table>& content, const std::string& path,
								    const lunas::fill_tree_type& data);

		std::expected<std::monostate, lunas::error> input_directory_check(const lunas::fill_tree_type& data);
	}
}
