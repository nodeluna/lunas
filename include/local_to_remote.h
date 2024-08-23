#ifndef LOCAL_TO_REMOTE
#define LOCAL_TO_REMOTE

#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>

namespace local_to_remote {
	int copy(const std::string& src, const std::string& dest, const sftp_session& sftp, const short& type);
	int rfile(const std::string& src, const std::string& dest, const sftp_session& sftp);
	int mkdir(const std::string& src, const std::string& dest, const sftp_session& sftp);
	int symlink(const std::string& src, const std::string& dest, const sftp_session& sftp);
}


#endif // REMOTE_ENABLED

#endif // LOCAL_TO_REMOTE
