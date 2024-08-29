#ifndef CONFIG
#define CONFIG

#include <string>
#include <vector>
#include "os.h"

#if (!defined(DISABLE_REMOTE))
	#define REMOTE_ENABLED
#endif
#if (!defined(DISABLE_CHECKSUM))
	#define CHECKSUM_ENABLED
#endif

namespace options{
	inline std::vector<std::string> exclude;
	inline bool quiet = false;
	inline bool verbose = false;
	inline bool dry_run = false;
	inline bool mkdir = false;
	inline bool progress_bar = false;
	inline bool remove_extra = false;
	inline bool follow_symlink = false;
	inline bool fsync = false;
	inline bool mtime_count = true;
	inline bool no_broken_symlink = false;
	inline bool update = false;
	inline bool resume = false;
	inline bool rollback = false;
	inline bool checksum = false;
	inline bool compression = false;
	inline int compression_level = 5;
	inline std::string color_error = "\x1b[1;31m";
	inline std::string reset_color = "\x1b[0m";
}

#endif // CONFIG
