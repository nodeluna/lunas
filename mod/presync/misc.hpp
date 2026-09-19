#pragma once

#include <expected>
#include <vector>
#include <variant>
#include <set>

#include "error/error.hpp"
#include "input_path/input_path.hpp"
#include "file_table/file_table.hpp"

namespace lunas
{
	namespace presync
	{
		std::expected<std::monostate, lunas::error>
		       input_paths_are_different(const std::vector<struct lunas::ipath::input_path>& ipaths);

		size_t to_be_synced_counter(const std::set<file_table>& conent);
	}
}
