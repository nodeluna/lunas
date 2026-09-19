#pragma once

#include <cassert>

#include <expected>
#include <variant>
#include <ctime>
#include <filesystem>

#include "types.hpp"
#include "error/error.hpp"
#include "input_path/input_path.hpp"
#include "file_table/file_table.hpp"

namespace lunas
{
	std::expected<size_t, lunas::error>	    get_src(const lunas::file_table& file_table, const struct lunas::parsed_data& data);

	std::expected<std::monostate, lunas::error> check_dest(const struct file_metadata<std::filesystem::path>& src,
							       const struct file_metadata<std::filesystem::path>& dest,
							       struct lunas::parsed_data&			  data);
}
