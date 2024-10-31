#include "config.h"

#ifdef REMOTE_ENABLED
#	include <libssh/sftp.h>
#	include <string>
#	include <queue>
#	include <fcntl.h>
#	include "remote_copy.h"
#	include "remote_attrs.h"
#	include "remote_to_remote.h"
#	include "file_types.h"
#	include "log.h"
#	include "raii_sftp.h"
#	include "base.h"
#	include "cliargs.h"
#	include "permissions.h"
#	include "progress.h"
#	include "resume.h"

namespace remote_to_remote {
	syncstat copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp,
	    const short& type) {
		struct syncstat syncstat;
		if (type == REGULAR_FILE)
			syncstat = remote_to_remote::rfile(src, dest, src_sftp, dest_sftp);
		else if (type == DIRECTORY)
			syncstat = remote_to_remote::mkdir(src, dest, src_sftp, dest_sftp);
		else if (type == SYMLINK)
			syncstat = remote_to_remote::symlink(src, dest, src_sftp, dest_sftp);
		else
			llog::error("can't sync special file '" + src + "'");

		return syncstat;
	}

	syncstat rfile(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp) {
		struct syncstat syncstat;
		sftp_file	src_file = sftp_open(src_sftp, src.c_str(), O_RDONLY, 0);
		if (src_file == NULL) {
			llog::error("couldn't open src '" + src + "', " + ssh_get_error(src_sftp->session));
			return syncstat;
		}
		raii::sftp::file file_obj_src = raii::sftp::file(&src_file, src);

		sftp_attributes attr = sftp_stat(src_sftp, src.c_str());
		if (attr == NULL) {
			llog::error("couldn't get src size '" + src + "', " + ssh_get_error(src_sftp->session));
			return syncstat;
		}

		std::uintmax_t src_size = 0, dest_size = 0;
		src_size	   = attr->size;
		unsigned int perms = attr->permissions;
		sftp_attributes_free(attr);
		syncstat.copied_size = src_size;

		if (options::dry_run) {
			syncstat.code = 1;
			return syncstat;
		}

		int access_type, rc;
		if (options::resume) {
			sftp_attributes attrs = sftp_stat(dest_sftp, dest.c_str());
			if (attrs == NULL && sftp_get_error(dest_sftp) != SSH_FX_NO_SUCH_FILE) {
				llog::error("couldn't check '" + dest + "', " + ssh_get_error(dest_sftp->session));
				return syncstat;
			} else if (attrs != NULL) {
				dest_size = attrs->size;
				sftp_attributes_free(attrs);
				rc = sftp_seek64(src_file, dest_size);
				if (llog::rc(src_sftp, src, rc, "couldn't move file pointer", NO_EXIT) == false)
					return syncstat;
			}
			access_type = O_WRONLY | O_CREAT | O_APPEND;
		} else
			access_type = O_WRONLY | O_CREAT | O_TRUNC;

		sftp_file dest_file = sftp_open(dest_sftp, (dest).c_str(), access_type, perms);
		if (dest_file == NULL) {
			llog::error("couldn't open dest '" + dest + "', " + ssh_get_error(dest_sftp->session));
			return syncstat;
		}
		raii::sftp::file file_obj_dest = raii::sftp::file(&dest_file, dest);

		if (not remote_attrs::sync_ownership(src, dest, src_sftp, dest_sftp)) {
			return syncstat;
		}

		sftp_limits_t limit	  = sftp_limits(dest_sftp);
		std::uint64_t buffer_size = limit->max_write_length;
		sftp_limits_free(limit);

		limit = sftp_limits(src_sftp);
		if (limit->max_read_length < buffer_size)
			buffer_size = limit->max_read_length;
		sftp_limits_free(limit);

		std::queue<buffque> rqueue;
		std::queue<buffque> wqueue;

		constexpr int	     max_requests	= 3;
		int		     read_requests_sent = 0, write_requests_sent = 0, bytes_written, bytes_requested;
		std::uintmax_t	     total_write_bytes_requested = dest_size, total_read_bytes_requested = dest_size;
		struct progress::obj _;

		while (dest_size < src_size) {
			while (read_requests_sent < max_requests && total_read_bytes_requested < src_size) {
				struct buffque bq(buffer_size);

				bytes_requested = sftp_aio_begin_read(src_file, buffer_size, &bq.aio);
				if (bytes_requested == SSH_ERROR) {
					llog::error("error reading '" + src + "', " + ssh_get_error(src_sftp->session));
					goto fail;
				} else if (bytes_requested == 0)
					break;

				read_requests_sent++;
				rqueue.push(bq);
				total_read_bytes_requested += bytes_requested;
			}

			if (rqueue.empty() && wqueue.empty())
				break;
			else if (rqueue.empty())
				goto write;

		read_again:
			rqueue.front().bytes_xfered = sftp_aio_wait_read(&rqueue.front().aio, rqueue.front().buffer.data(), buffer_size);
			if (rqueue.front().bytes_xfered == SSH_ERROR) {
				llog::error("error reading '" + src + "', " + ssh_get_error(src_sftp->session));
				goto fail;
			} else if (rqueue.front().bytes_xfered == SSH_AGAIN)
				goto read_again;

		write:
			if (write_requests_sent >= max_requests || (total_write_bytes_requested >= src_size && write_requests_sent > 0)) {
				bytes_written = sftp_aio_wait_write(&wqueue.front().aio);
				if (bytes_written == SSH_ERROR) {
					llog::error("error writing '" + dest + "', " + ssh_get_error(dest_sftp->session));
					goto fail;
				} else if (bytes_written != wqueue.front().bytes_xfered) {
					llog::error("error writing '" + dest + "', " + ssh_get_error(dest_sftp->session));
					goto fail;
				}
				write_requests_sent--;
				if (read_requests_sent > 0)
					read_requests_sent--;
				dest_size += bytes_written;
				progress::ingoing(src_size, dest_size);
				wqueue.pop();
			}

			if (total_write_bytes_requested >= src_size || rqueue.empty())
				continue;
			wqueue.push(rqueue.front());
			rqueue.pop();

			bytes_requested =
			    sftp_aio_begin_write(dest_file, wqueue.back().buffer.data(), wqueue.back().bytes_xfered, &wqueue.back().aio);
			if (bytes_requested == SSH_ERROR) {
				llog::error("error writing '" + dest + "', " + ssh_get_error(dest_sftp->session));
				goto fail;
			}
			total_write_bytes_requested += bytes_requested;
			write_requests_sent++;
			if (write_requests_sent >= read_requests_sent && read_requests_sent > 0)
				read_requests_sent--;
		}

		if (dest_size != src_size) {
			llog::error("error occured while syncing '" + dest + "', mismatch src/dest sizes");
			goto fail;
		}

		if (options::fsync) {
			rc = sftp_fsync(dest_file);
			if (rc != SSH_OK)
				llog::warn("couldn't fsync '" + dest + "', " + ssh_get_error(dest_sftp->session));
		}

		syncstat.code = 1;
		return syncstat;
	fail:
		while (rqueue.empty() != true) {
			sftp_aio_free(rqueue.front().aio);
			rqueue.pop();
		}
		while (wqueue.empty() != true) {
			sftp_aio_free(wqueue.front().aio);
			wqueue.pop();
		}

		return syncstat;
	}

	syncstat mkdir(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp) {
		struct syncstat syncstat;
		if (options::dry_run) {
			syncstat.code = 1;
			return syncstat;
		}
		int	     rc	   = 0;
		unsigned int perms = ( unsigned int ) permissions::get_remote(src_sftp, src, rc);
		if (llog::rc(src_sftp, src, rc, "couldn't get file permissions", NO_EXIT) == false)
			return syncstat;

		rc = sftp::mkdir(dest_sftp, dest, perms);
		if (llog::rc(dest_sftp, dest, rc, "couldn't make directory", NO_EXIT) == false)
			return syncstat;

		if (not remote_attrs::sync_ownership(src, dest, src_sftp, dest_sftp)) {
			resume::unlink(dest_sftp, dest, DIRECTORY);
			return syncstat;
		}

		syncstat.code = 1;
		return syncstat;
	}

	syncstat symlink(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp) {
		struct syncstat syncstat;
		if (options::dry_run) {
			syncstat.code = 1;
			return syncstat;
		}

		std::expected<std::string, SSH_STATUS> target = sftp::readlink(src_sftp->session, src, src);
		if (not target) {
			llog::rc(dest_sftp, dest, target.error(), "couldn't get symlink target", NO_EXIT);
			return syncstat;
		}

		int rc = sftp::symlink(dest_sftp, target.value(), dest);
		if (llog::rc(dest_sftp, dest, rc, "couldn't make symlink", NO_EXIT) == false)
			return syncstat;

		if (not remote_attrs::sync_ownership(src, dest, src_sftp, dest_sftp)) {
			resume::unlink(dest_sftp, dest, SYMLINK);
			return syncstat;
		}

		syncstat.code = 1;
		return syncstat;
	}
}

#endif // REMOTE_ENABLED
