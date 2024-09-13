#ifndef OWNERSHIP
#define OWNERSHIP

#include "config.h"
#include <expected>
#include <optional>
#include <system_error>

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#include "raii_sftp.h"
#endif // REMOTE_ENABLED

struct own {
	int uid = -1;
	int gid = -1;
};

namespace ownership{
	std::expected<struct own, std::error_code> get_local(const std::string& path);
	std::optional<std::error_code> set_local(const std::string& path, const struct own& own);

#ifdef REMOTE_ENABLED
	std::expected<struct own, SSH_STATUS> get_remote(const sftp_session& sftp, const std::string& path);
	std::optional<SSH_STATUS> set_remote(const sftp_session& sftp, const std::string& path, const struct own& own);
#endif // REMOTE_ENABLED
}

#endif // OWNERSHIP
