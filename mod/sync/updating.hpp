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
#include "hooks/hook.hpp"

namespace lunas
{
	using _file_metadata = struct file_metadata<std::filesystem::path>;

	std::expected<std::monostate, lunas::error> check_dest_and_sync(const _file_metadata& src, const _file_metadata& dest,
									struct lunas::parsed_data& data,
									struct progress_stats&	   progress_stats,
									const struct lunas::hooks& hooks);

	std::expected<std::monostate, lunas::error> updating(const lunas::file_table& file_table, const size_t src_index,
							     struct lunas::parsed_data& data, struct progress_stats& progress_stats,
							     struct lunas::hooks& hooks);
}
