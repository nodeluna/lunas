#ifndef REMOTE_TO_LOCAL
#define REMOTE_TO_LOCAL

#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>

namespace remote_to_local {
	int copy(const std::string& src, const std::string& dest, const sftp_session& sftp, const short& type);
	int rfile(const std::string& src, const std::string& dest, const sftp_session& sftp);
	int mkdir(const std::string& src, const std::string& dest, const sftp_session& sftp);
	int symlink(const std::string& src, const std::string& dest, const sftp_session& sftp);
}


#endif // REMOTE_ENABLED

#endif // REMOTE_TO_LOCAL
