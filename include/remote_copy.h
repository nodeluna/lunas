#ifndef HANDLING_COPYING_REMOTE
#define HANDLING_COPYING_REMOTE

#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <libssh/sftp.h>

namespace fs_remote {
	void copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type);
}


#endif // REMOTE_ENABLED

#endif // HANDLING_COPYING_REMOTE
