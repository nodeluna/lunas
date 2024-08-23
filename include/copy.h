#ifndef HANDLING_COPYING
#define HANDLING_COPYING


#include <string>
#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED

namespace lunas{
#ifdef REMOTE_ENABLED
	void copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type);
#else
	void copy(const std::string& src, const std::string& dest, const short& type);
#endif // REMOTE_ENABLED
}

#endif // HANDLING_COPYING
