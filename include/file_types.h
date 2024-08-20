#ifndef TYPES
#define TYPES

#define REGULAR_FILE  1
#define	DIRECTORY 2
#define	SYMLINK 3
#define SOCKET 4
#define	SPECIAL_TYPE 5

#include <string>
#include <filesystem>
#include "log.h"

namespace fs = std::filesystem;

namespace status{
	unsigned short int local_types(fs::file_status entry);

	short int local_type(const std::string& path, const bool& cerr);
}

#endif // TYPES
