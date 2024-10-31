#include <string>
#include <expected>
#include <optional>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

#include "config.h"

#ifdef REMOTE_ENABLED
#	include <libssh/sftp.h>
#	include "raii_sftp.h"
#endif // REMOTE_ENABLED

#include "ownership.h"
#include "log.h"

namespace ownership {
	std::expected<struct own, std::error_code> get_local(const std::string& path) {
		struct own  own;
		struct stat stats;
		int	    rc = 0;
		if (options::follow_symlink)
			rc = stat(path.c_str(), &stats);
		else
			rc = lstat(path.c_str(), &stats);
		if (rc != 0) {
			std::error_code ec = std::make_error_code(( std::errc ) errno);
			return std::unexpected(ec);
		}
		own.uid = stats.st_uid;
		own.gid = stats.st_gid;
		return own;
	}

	std::optional<std::error_code> set_local(const std::string& path, const struct own& own) {
		int rc = 0;

		if (options::follow_symlink == true)
			rc = chown(path.c_str(), own.uid, own.gid);
		else
			rc = lchown(path.c_str(), own.uid, own.gid);

		if (rc != 0) {
			std::error_code ec = std::make_error_code(( std::errc ) errno);
			return ec;
		}

		return std::nullopt;
	}

#ifdef REMOTE_ENABLED
	std::expected<struct own, SSH_STATUS> get_remote(const sftp_session& sftp, const std::string& path) {
		struct own	own;
		sftp_attributes attributes = NULL;
		if (options::follow_symlink)
			attributes = sftp_stat(sftp, path.c_str());
		else
			attributes = sftp_lstat(sftp, path.c_str());

		if (attributes == NULL)
			return std::unexpected(sftp_get_error(sftp));

		own.uid = attributes->uid;
		own.gid = attributes->gid;
		sftp_attributes_free(attributes);
		return own;
	}

	std::optional<SSH_STATUS> set_remote(const sftp_session& sftp, const std::string& path, const struct own& own) {
		struct sftp_attributes_struct attributes;
		attributes.flags = SSH_FILEXFER_ATTR_UIDGID;
		attributes.uid	 = own.uid;
		attributes.gid	 = own.gid;

		sftp_attributes attrs = NULL;
		if (own.uid == -1 || own.gid == -1) {
			if (options::follow_symlink)
				attrs = sftp_stat(sftp, path.c_str());
			else
				attrs = sftp_lstat(sftp, path.c_str());
			if (attrs == NULL)
				return sftp_get_error(sftp);
		}
		raii::sftp::attributes attrs_obj = raii::sftp::attributes(&attrs);

		if (own.uid == -1)
			attributes.uid = attrs->uid;
		if (own.gid == -1)
			attributes.gid = attrs->gid;

		int rc;
		if (options::follow_symlink)
			rc = sftp_setstat(sftp, path.c_str(), &attributes);
		else
			rc = sftp_lsetstat(sftp, path.c_str(), &attributes);

		if (rc != SSH_OK)
			return sftp_get_error(sftp);

		return std::nullopt;
	}
#endif // REMOTE_ENABLED
}
