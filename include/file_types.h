#ifndef TYPES
#define TYPES

#define REGULAR_FILE  1
#define	DIRECTORY 2
#define	SYMLINK 3
#define	BROKEN_SYMLINK -3
#define SOCKET 4
#define	SPECIAL_TYPE 5

#include <string>
#include <filesystem>
#include <expected>
#include "log.h"
#include "config.h"

namespace fs = std::filesystem;

std::string get_type_name(const short& type);

namespace status{
	unsigned short int local_types(fs::file_status entry);

	short int local_type(const std::string& path, const bool& cerr);

	std::expected<bool, std::error_code> is_broken_link(const std::string& path);
#ifdef REMOTE_ENABLED
	short int remote_type(const sftp_attributes& attributes);

	short int remote_type2(const sftp_session& sftp, const std::string& path, bool cerr);
#endif // REMOTE_ENABLED
}

namespace condition{
	bool is_src(const short& type);

	bool is_dest(const short& type);
}

#endif // TYPES
