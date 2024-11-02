#include "config.h"

#ifdef REMOTE_ENABLED
#	include <libssh/sftp.h>
#	include <string>
#	include <fstream>
#	include <cstring>
#	include <cerrno>
#	include <fcntl.h>
#	include <queue>
#	include <vector>
#	include <filesystem>
#	include "remote_to_local.h"
#	include "remote_copy.h"
#	include "local_copy.h"
#	include "file_types.h"
#	include "log.h"
#	include "cppfs.h"
#	include "raii_sftp.h"
#	include "raii_fstream.h"
#	include "permissions.h"
#	include "remote_attrs.h"
#	include "progress.h"
#	include "resume.h"

namespace remote_to_local {
	struct syncstat copy(const std::string& src, const std::string& dest, const sftp_session& sftp, const struct syncmisc& misc) {
		struct syncstat syncstat;
		if (misc.file_type == REGULAR_FILE) {
			auto func = [&](const std::string& dest_lspart) -> struct syncstat {
					struct syncstat		       st = remote_to_local::rfile(src, dest_lspart, sftp);
					struct fs_local::original_name _(dest_lspart, dest, st.code);
					return st;
			};

			syncstat = regular_file_sync(src, dest, misc.src_mtime, func);
		} else if (misc.file_type == DIRECTORY)
			syncstat = remote_to_local::mkdir(src, dest, sftp);
		else if (misc.file_type == SYMLINK)
			syncstat = remote_to_local::symlink(src, dest, sftp);
		else
			llog::error("can't sync special file '" + src + "'");

		return syncstat;
	}

	struct syncstat rfile(const std::string& src, const std::string& dest, const sftp_session& sftp) {
		struct syncstat syncstat;

		sftp_file src_file = sftp_open(sftp, src.c_str(), O_RDONLY, 0);
		if (src_file == NULL) {
			llog::error("couldn't open src '" + src + "', " + ssh_get_error(sftp->session));
			return syncstat;
		}
		raii::sftp::file file_obj_src = raii::sftp::file(&src_file, src);

		sftp_attributes attr = sftp_stat(sftp, src.c_str());
		if (attr == NULL) {
			llog::error("couldn't get src size '" + src + "', " + ssh_get_error(sftp->session));
			return syncstat;
		}
		std::uintmax_t src_size = 0, dest_size = 0;
		src_size		     = attr->size;
		std::filesystem::perms perms = ( std::filesystem::perms ) attr->permissions;
		sftp_attributes_free(attr);
		syncstat.copied_size = src_size;

		if (options::dry_run) {
			syncstat.code = 1;
			return syncstat;
		}

		std::error_code	   ec;
		std::ios::openmode openmode;

		if (options::resume) {
			dest_size = std::filesystem::file_size(dest, ec);
			if (llog::ec(dest, ec, "couldn't get dest size", NO_EXIT) == false)
				return syncstat;

			int rc = sftp_seek64(src_file, dest_size);
			if (llog::rc(sftp, src, rc, "couldn't move file pointer", NO_EXIT) == false)
				return syncstat;

			openmode = std::ios::out | std::ios::binary | std::ios::app;
		} else {
			openmode = std::ios::out | std::ios::binary;
		}

		std::fstream dest_file(dest, openmode);
		if (dest_file.is_open() == false) {
			llog::error("couldn't open dest '" + dest + "', " + std::strerror(errno));
			return syncstat;
		}
		raii::fstream::file file_obj_dest = raii::fstream::file(&dest_file, dest);

		ec = permissions::set_local(dest, perms);
		if (llog::ec(dest, ec, "couldn't set file permissions", NO_EXIT) == false)
			return syncstat;
		else if (not remote_attrs::sync_ownership(src, dest, sftp, nullptr))
			return syncstat;

		sftp_limits_t	    limit	= sftp_limits(sftp);
		const std::uint64_t buffer_size = limit->max_read_length;
		sftp_limits_free(limit);

		std::queue<buffque> queue;

		constexpr int	     max_requests	   = 5;
		int		     requests_sent	   = 0, bytes_written, bytes_requested;
		std::uintmax_t	     total_bytes_requested = dest_size;
		struct progress::obj _;

		while (dest_size < src_size) {
			while (requests_sent < max_requests && total_bytes_requested < src_size) {
				struct buffque bq(buffer_size);

				bytes_requested = sftp_aio_begin_read(src_file, buffer_size, &bq.aio);
				if (bytes_requested == SSH_ERROR) {
					llog::error("error reading '" + src + "', " + ssh_get_error(sftp->session));
					goto fail;
				} else if (bytes_requested == 0)
					break;

				requests_sent++;
				queue.push(bq);
				total_bytes_requested += bytes_requested;
			}
		read_again:
			if (queue.empty())
				break;

			queue.front().bytes_xfered = sftp_aio_wait_read(&queue.front().aio, queue.front().buffer.data(), buffer_size);
			if (queue.front().bytes_xfered == SSH_ERROR) {
				llog::error("error reading '" + src + "', " + ssh_get_error(sftp->session));
				goto fail;
			} else if (queue.front().bytes_xfered == SSH_AGAIN)
				goto read_again;

			std::fpos pos = dest_file.tellp();
			dest_file.write(queue.front().buffer.data(), queue.front().bytes_xfered);
			bytes_written = dest_file.tellp() - pos;
			if (bytes_written != queue.front().bytes_xfered || dest_file.bad() || dest_file.fail()) {
				llog::error("error writing '" + dest + "', " + std::strerror(errno));
				goto fail;
			}

			dest_size += bytes_written;
			progress::ingoing(src_size, dest_size);
			queue.pop();
			requests_sent--;
		}

		if (dest_size != src_size) {
			llog::error("error occured while syncing '" + dest + "', mismatch src/dest sizes");
			goto fail;
		}

		if (options::fsync) {
			int rc = dest_file.sync();
			if (rc != 0)
				llog::warn("couldn't fsync '" + dest + "', " + std::strerror(errno));
		}

		syncstat.code = 1;
		return syncstat;
	fail:
		while (queue.empty() != true) {
			sftp_aio_free(queue.front().aio);
			queue.pop();
		}

		return syncstat;
	}

	struct syncstat mkdir(const std::string& src, const std::string& dest, const sftp_session& sftp) {
		struct syncstat syncstat;
		if (options::dry_run) {
			syncstat.code = 1;
			return syncstat;
		}

		std::error_code ec;
		cppfs::mkdir(dest, ec);
		if (llog::ec(dest, ec, "couldn't make directory", NO_EXIT) == false)
			return syncstat;

		int		       rc    = 0;
		std::filesystem::perms perms = ( std::filesystem::perms ) permissions::get_remote(sftp, src, rc);
		if (llog::rc(sftp, src, rc, "couldn't get file permissions", NO_EXIT) == false)
			return syncstat;

		ec = permissions::set_local(dest, perms);
		if (llog::ec(dest, ec, "couldn't set file permissions", NO_EXIT) == false) {
			cppfs::remove(dest);
			return syncstat;
		} else if (not remote_attrs::sync_ownership(src, dest, sftp, nullptr)) {
			resume::unlink(sftp, dest, DIRECTORY);
			return syncstat;
		}

		syncstat.code = 1;
		return syncstat;
	}

	struct syncstat symlink(const std::string& src, const std::string& dest, const sftp_session& sftp) {
		struct syncstat syncstat;
		if (options::dry_run) {
			syncstat.code = 1;
			return syncstat;
		}

		std::error_code			       ec;
		std::expected<std::string, SSH_STATUS> target = sftp::readlink(sftp->session, src, src);
		if (not target) {
			llog::rc(sftp, src, target.error(), "couldn't get symlink target", NO_EXIT);
			return syncstat;
		}

		cppfs::symlink(target.value(), dest, ec);
		if (llog::ec(dest, ec, "couldn't make symlink", NO_EXIT) == false)
			return syncstat;

		if (not remote_attrs::sync_ownership(src, dest, sftp, nullptr)) {
			resume::unlink(sftp, dest, SYMLINK);
			return syncstat;
		}

		syncstat.code = 1;
		return syncstat;
	}
}

#endif // REMOTE_ENABLED
