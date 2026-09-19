#pragma once

#include <set>

#include "file_table/file_table.hpp"

namespace lunas
{
	struct content {
			std::set<lunas::file_table> files_table;
			size_t			    to_be_synced = 1;
	};
}
