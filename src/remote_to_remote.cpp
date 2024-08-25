#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include <queue>
#include <fcntl.h>
#include "remote_copy.h"
#include "remote_to_remote.h"
#include "file_types.h"
#include "log.h"
#include "raii_sftp.h"


namespace remote_to_remote {
	int copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
		if(type == REGULAR_FILE)
			return remote_to_remote::rfile(src, dest, src_sftp, dest_sftp);
		else if(type == DIRECTORY)
			return remote_to_remote::mkdir(src, dest, src_sftp, dest_sftp);
		else if(type == SYMLINK)
			return remote_to_remote::symlink(src, dest, src_sftp, dest_sftp);
		else
			llog::error("can't sync special file '" + src + "'");
		return 0;
	}

	int rfile(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp){
		sftp_file src_file = sftp_open(src_sftp, src.c_str(), O_RDONLY, 0);
		if(src_file == NULL){
			llog::error("couldn't open src '" + src + "', " + ssh_get_error(src_sftp->session));
			return 0;
		}
		raii::sftp::file file_obj_src = raii::sftp::file(&src_file, src);

		int access_type = O_WRONLY | O_CREAT | O_TRUNC;
		int perms = S_IRWXU;
		sftp_file dest_file = sftp_open(dest_sftp, dest.c_str(), access_type, perms);
		if(dest_file == NULL){
			llog::error("couldn't open dest '" + dest + "', " + ssh_get_error(dest_sftp->session));
			return 0;
		}
		raii::sftp::file file_obj_dest = raii::sftp::file(&dest_file, dest);

		sftp_limits_t limit = sftp_limits(dest_sftp);
		int buffer_size = limit->max_write_length;
		sftp_limits_free(limit);

		limit = sftp_limits(src_sftp);
		if(limit->max_read_length < buffer_size)
			buffer_size = limit->max_read_length;
		sftp_limits_free(limit);

		std::queue<buffque> rqueue;
		std::queue<buffque> wqueue;

		constexpr int max_requests = 3;
		int read_requests_sent = 0, write_requests_sent = 0, bytes_written, bytes_requested;
		std::uintmax_t src_size = 0, dest_size = 0, total_write_bytes_requested = 0, total_read_bytes_requested = 0;

		sftp_attributes attr = sftp_stat(src_sftp, src.c_str());
		if(attr == NULL){
			llog::error("couldn't get src size '" + src + "', " + ssh_get_error(src_sftp->session));
			goto fail;
		}
		src_size = attr->size;
		sftp_attributes_free(attr);

		while(dest_size < src_size){
			while(read_requests_sent < max_requests && total_read_bytes_requested < src_size){
				struct buffque bq(buffer_size);

				bytes_requested = sftp_aio_begin_read(src_file, buffer_size, &bq.aio);
				if(bytes_requested == SSH_ERROR){
					llog::error("error reading '" + src + "', " + ssh_get_error(src_sftp->session));
					goto fail;
				}else if(bytes_requested == 0)
					break;

				read_requests_sent++;
				rqueue.push(bq);
				total_read_bytes_requested += bytes_requested;
			}

			if(rqueue.empty() && wqueue.empty())
				break;
			else if(rqueue.empty())
				goto write;

read_again:
			rqueue.front().bytes_xfered = sftp_aio_wait_read(&rqueue.front().aio, rqueue.front().buffer.data(), buffer_size);
			if(rqueue.front().bytes_xfered == SSH_ERROR){
				llog::error("error reading '" + src + "', " + ssh_get_error(src_sftp->session));
				goto fail;
			}else if(rqueue.front().bytes_xfered == SSH_AGAIN)
				goto read_again;

write:
			if(write_requests_sent >= max_requests || (total_write_bytes_requested  >= src_size && write_requests_sent > 0)){
				bytes_written = sftp_aio_wait_write(&wqueue.front().aio);
				if(bytes_written == SSH_ERROR){
					llog::error("error writing '" + dest + "', " + ssh_get_error(dest_sftp->session));
					goto fail;
				}else if(bytes_written != wqueue.front().bytes_xfered){
					llog::error("error writing '" + dest + "', " + ssh_get_error(dest_sftp->session));
					goto fail;
				}
				write_requests_sent--;
				if(read_requests_sent > 0)
					read_requests_sent--;
				dest_size += bytes_written;
				wqueue.pop();
			}

			if(total_write_bytes_requested >= src_size || rqueue.empty())
				continue;
			wqueue.push(rqueue.front());
			rqueue.pop();

			bytes_requested = sftp_aio_begin_write(dest_file, wqueue.back().buffer.data(), wqueue.back().bytes_xfered, &wqueue.back().aio);
			if(bytes_requested == SSH_ERROR){
				llog::error("error writing '" + dest + "', " + ssh_get_error(dest_sftp->session));
				goto fail;
			}
			total_write_bytes_requested += bytes_requested;
			write_requests_sent++;
			if(write_requests_sent >= read_requests_sent && read_requests_sent > 0)
				read_requests_sent--;
		}

		if(dest_size != src_size){
			llog::error("error occured while syncing '" + dest + "', mismatch src/dest sizes");
			goto fail;
		}

		return 1;
fail:
		while(rqueue.empty() != true){
			sftp_aio_free(rqueue.front().aio);
			rqueue.pop();
		}
		while(wqueue.empty() != true){
			sftp_aio_free(wqueue.front().aio);
			wqueue.pop();
		}

		return 0;
	}

	int mkdir(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp){
		int rc = sftp::mkdir(dest_sftp, dest);
		if(llog::rc(dest_sftp, dest, rc, "couldn't make directory", NO_EXIT) == false)
			return 0;

		return 1;
	}

	int symlink(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp){
		char* target = sftp_readlink(src_sftp, src.c_str());
		if(target == NULL){
			llog::error("couldn't read symlink '" + src + "', " + ssh_get_error(src_sftp->session));
			return 0;
		}
		raii::sftp::link_target link_obj = raii::sftp::link_target(&target);

		int rc = sftp::symlink(dest_sftp, target, dest);
		if(llog::rc(dest_sftp, dest, rc, "couldn't make symlink", NO_EXIT) == false)
			return 0;

		return 1;
	}
}

#endif // REMOTE_ENABLED
