#ifndef FS_LOCAL
#define FS_LOCAL

#include <filesystem>
#include <string>
#include "cliargs.h"

namespace fs_local {
	int list_tree(struct input_path& local_path, const unsigned long int& index_path);

	std::expected<std::uintmax_t, std::error_code> available_space(struct input_path& local_path);
}

#endif // FS_LOCAL
