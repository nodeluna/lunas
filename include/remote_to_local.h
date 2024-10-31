#ifndef REMOTE_TO_LOCAL
#define REMOTE_TO_LOCAL

#include "config.h"

#ifdef REMOTE_ENABLED

#	include <libssh/sftp.h>
#	include <string>
#	include "copy.h"

namespace remote_to_local {
	struct syncstat copy(const std::string& src, const std::string& dest, const sftp_session& sftp, const short& type);
	struct syncstat rfile(const std::string& src, const std::string& dest, const sftp_session& sftp);
	struct syncstat mkdir(const std::string& src, const std::string& dest, const sftp_session& sftp);
	struct syncstat symlink(const std::string& src, const std::string& dest, const sftp_session& sftp);
}

#endif // REMOTE_ENABLED

#endif // REMOTE_TO_LOCAL
