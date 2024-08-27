#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED

#include <filesystem>
#include <fcntl.h>

#include "permissions.h"

namespace permissions{
	std::filesystem::perms get_local(const std::string& path, std::error_code& ec){
		if(ec.value() == 0)
			return std::filesystem::symlink_status(path, ec).permissions();
		return (std::filesystem::perms) (S_IRUSR | S_IWUSR | S_IRGRP);
	}

	std::error_code set_local(const std::string& path, std::filesystem::perms permissions){
		std::error_code ec;
		std::filesystem::permissions(path, permissions, std::filesystem::perm_options::replace, ec);
		return ec;
	}

#ifdef REMOTE_ENABLED
	unsigned int get_remote(const sftp_session& sftp, const std::string& path, int& rc){
		sftp_attributes attributes = sftp_lstat(sftp, path.c_str());
		if(attributes != NULL){
			unsigned int perms = (unsigned int)attributes->permissions;
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
