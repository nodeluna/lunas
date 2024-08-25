#ifndef LOCAL_TO_REMOTE
#define LOCAL_TO_REMOTE

#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include "copy.h"

namespace local_to_remote {
	struct syncstat copy(const std::string& src, const std::string& dest, const sftp_session& sftp, const short& type);
	struct syncstat rfile(const std::string& src, const std::string& dest, const sftp_session& sftp);
	struct syncstat mkdir(const std::string& src, const std::string& dest, const sftp_session& sftp);
	struct syncstat symlink(const std::string& src, const std::string& dest, const sftp_session& sftp);
}


#endif // REMOTE_ENABLED

#endif // LOCAL_TO_REMOTE
