#ifndef PERMISSIONS
#define PERMISSIONS

#include <filesystem>
#include <string>
#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED


namespace permissions{
	bool is_local_readable(const std::string& path);
	std::filesystem::perms get_local(const std::string& path, std::error_code& ec);
	std::error_code set_local(const std::string& path, std::filesystem::perms permissions);

#ifdef REMOTE_ENABLED
	unsigned int get_remote(const sftp_session& sftp, const std::string& path, int& rc);
	int set_remote(const sftp_session& sftp, const std::string& path, unsigned int permissions);
#endif // REMOTE_ENABLED
}

#endif // PERMISSIONS
