#ifndef HANDLING_COPYING
#define HANDLING_COPYING


#include <string>
#include <cstdint>
#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED

typedef struct syncstat{
	int code = 0;
	std::uintmax_t copied_size = 0;
} syncstat;

namespace lunas{
#ifdef REMOTE_ENABLED
	syncstat copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type);
#else
	syncstat copy(const std::string& src, const std::string& dest, const short& type);
#endif // REMOTE_ENABLED

	std::string original_dest(const std::string& dest_with_hash);
}

#endif // HANDLING_COPYING
