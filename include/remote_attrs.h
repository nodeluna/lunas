#ifndef REMOTE_ATTRS
#define REMOTE_ATTRS

#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <libssh/sftp.h>

namespace remote_attrs {
	int sync_utimes(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type);
}

#endif // REMOTE_ENABLED

#endif // REMOTE_ATTRS
