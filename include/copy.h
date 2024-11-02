#ifndef HANDLING_COPYING
#define HANDLING_COPYING

#include <string>
#include <cstdint>
#include <functional>
#include "config.h"
#include "resume.h"

#ifdef REMOTE_ENABLED
#	include <libssh/sftp.h>
#endif // REMOTE_ENABLED

typedef struct syncstat {
		int	       code	   = 0;
		std::uintmax_t copied_size = 0;
} syncstat;

typedef struct syncmisc {
		time_t src_mtime = 0;
		short  file_type = 0;
} syncmisc;

inline auto regular_file_sync = [](const std::string& src, const std::string& dest, const time_t& src_mtime,
				    std::function<syncstat(const std::string&)> func) -> syncstat {
	size_t	    src_quick_hash = resume::get_src_hash(src, src_mtime);
	std::string dest_lspart	   = dest + "." + std::to_string(src_quick_hash) + ".ls.part";
	return func(dest_lspart);
};

namespace lunas {
#ifdef REMOTE_ENABLED
	syncstat copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp,
	    const struct syncmisc& misc);
#else
	syncstat copy(const std::string& src, const std::string& dest, const struct syncmisc& misc);
#endif // REMOTE_ENABLED

	std::string original_dest(const std::string& dest_with_hash);
}

#endif // HANDLING_COPYING
