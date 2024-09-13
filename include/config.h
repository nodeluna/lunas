#ifndef CONFIG
#define CONFIG

#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>
#include "os.h"

#if (!defined(DISABLE_REMOTE))
	#define REMOTE_ENABLED
#endif
#if (!defined(DISABLE_CHECKSUM))
	#define CHECKSUM_ENABLED
#endif

namespace options{
	inline std::unordered_set<std::string> exclude;
	inline std::unordered_set<std::string> exclude_pattern;
	inline bool quiet = false;
	inline bool verbose = false;
	inline bool dry_run = false;
	inline bool mkdir = false;
	inline bool progress_bar = true;
	inline bool remove_extra = false;
	inline bool follow_symlink = false;
	inline bool fsync = false;
	inline bool mtime_count = true;
	inline bool no_broken_symlink = false;
	inline bool update = false;
	inline bool resume = true;
	inline bool rollback = false;
	inline bool checksum = false;
	inline bool compression = false;
	inline bool attributes_uid = false;
	inline bool attributes_gid = false;
	inline bool attributes_mtime = true;
	inline bool attributes_atime = false;
	inline int compression_level = 5;
	inline std::uintmax_t minimum_space = 1073741824;
	inline std::string color_error = "\x1b[1;31m";
	inline std::string reset_color = "\x1b[0m";
}

#endif // CONFIG
