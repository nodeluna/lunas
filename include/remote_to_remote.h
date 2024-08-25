#ifndef REMOTE_TO_REMOTE
#define REMOTE_TO_REMOTE

#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include "copy.h"

namespace remote_to_remote {
	syncstat copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type);
	syncstat rfile(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
	syncstat mkdir(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
	syncstat symlink(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
}


#endif // REMOTE_ENABLED

#endif // REMOTE_TO_REMOTE
