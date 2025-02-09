module;

#include <set>

export module lunas.content;
export import lunas.file_table;

export namespace lunas {
	struct content {
			std::set<lunas::file_table> files_table;
			size_t			    to_be_synced = 0;
	};
}
