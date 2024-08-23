#ifndef REMOTE_TO_REMOTE
#define REMOTE_TO_REMOTE

#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>

namespace remote_to_remote {
	int copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type);
	int rfile(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
	int mkdir(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
	int symlink(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
}


#endif // REMOTE_ENABLED

#endif // REMOTE_TO_REMOTE
