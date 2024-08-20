#ifndef FS
#define FS

#include <filesystem>
#include <string>
#include "cliargs.h"

int list_tree(const struct input_path& local_path, const unsigned long int& index_path);

#endif // FS
