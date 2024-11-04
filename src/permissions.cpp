#include "config.h"

#ifdef REMOTE_ENABLED
#	include <libssh/sftp.h>
#endif // REMOTE_ENABLED

#include <filesystem>
#include <system_error>
#include <fcntl.h>
#include <sys/stat.h>
#include <expected>
#include "permissions.h"
#include "log.h"

namespace fs = std::filesystem;

namespace permissions {
	bool is_local_readable(const std::string& path) {
		std::expected<std::filesystem::perms, std::error_code> perms = permissions::get_local(path);
		if (not perms)
			llog::ec(path, perms.error(), "couldn't check permission", EXIT_FAILURE);

		if ((perms.value() & fs::perms::owner_read) != fs::perms::none)
			return true;
		return false;
	}

	std::expected<std::filesystem::perms, std::error_code> get_local(const std::string& path) {
		std::error_code	       ec;
		std::filesystem::perms perms;
		if (options::follow_symlink)
			perms = std::filesystem::status(path, ec).permissions();
		else
			perms = std::filesystem::symlink_status(path, ec).permissions();
		if (ec.value() != 0)
			return std::unexpected(ec);
		return perms;
	}

	std::error_code set_local(const std::string& path, std::filesystem::perms permissions) {
		std::error_code ec;
		std::filesystem::permissions(path, permissions, std::filesystem::perm_options::replace, ec);
		return ec;
	}

#ifdef REMOTE_ENABLED
	unsigned int get_remote(const sftp_session& sftp, const std::string& path, int& rc) {
		sftp_attributes attributes;
		if (options::follow_symlink)
			attributes = sftp_stat(sftp, path.c_str());
		else
			attributes = sftp_lstat(sftp, path.c_str());
		if (attributes != NULL) {
			unsigned int perms = ( unsigned int ) attributes->permissions;
			sftp_attributes_free(attributes);
			rc = SSH_OK;
			return perms;
		}
		rc = -1;
		return (S_IRUSR | S_IWUSR | S_IRGRP);
	}

	/*int set_remote(const sftp_session& sftp, const std::string& path, unsigned int permissions){
		int rc = sftp_chmod(sftp, path.c_str(), permissions);
		return rc;
	}*/
#endif // REMOTE_ENABLED
}
