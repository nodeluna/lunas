#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <set>
#	include <expected>
#	include <variant>
#	include <ctime>
#	include <string>
#	include <cstddef>
#	include <exception>
#	include <filesystem>
#	include <algorithm>
#	include <type_traits>
#endif

#include "types.hpp"
#include "copy/copy.hpp"
#include "checks.hpp"
#include "updating.hpp"
#include "remove.hpp"

#include "error/error.hpp"
#include "input_path/input_path.hpp"
#include "content/content.hpp"
#include "file_table/file_table.hpp"
#include "file_types/file_types.hpp"
#include "file/file.hpp"
#include "filter/filter.hpp"
#include "hooks/hooks.hpp"
#include "stdout/stdout.hpp"

namespace lunas
{
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data);
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data, lunas::content& content);
}
