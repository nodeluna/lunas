#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <set>
#endif

#include "file_table/file_table.hpp"

namespace lunas
{
	struct content {
			std::set<lunas::file_table> files_table;
			size_t			    to_be_synced = 1;
	};
}
