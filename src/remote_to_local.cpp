#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <queue>
#include <vector>
#include "remote_to_local.h"
#include "remote_copy.h"
#include "file_types.h"
#include "log.h"
#include "cppfs.h"
#include "raii_sftp.h"
#include "raii_fstream.h"


namespace remote_to_local {
	int copy(const std::string& src, const std::string& dest, const sftp_session& sftp, const short& type){
		if(type == REGULAR_FILE)
			return remote_to_local::rfile(src, dest, sftp);
		else if(type == DIRECTORY)
			return remote_to_local::mkdir(src, dest, sftp);
		else if(type == SYMLINK)
			return remote_to_local::symlink(src, dest, sftp);
		else
			llog::error("can't sync special file '" + src + "'");
		return 0;
	}

	int rfile(const std::string& src, const std::string& dest, const sftp_session& sftp){
		sftp_file src_file = sftp_open(sftp, src.c_str(), O_RDONLY, 0);
		if(src_file == NULL){
			llog::error("couldn't open src '" + src + "', " + ssh_get_error(sftp->session));
			return 0;
		}
		raii::sftp::file file_obj_src = raii::sftp::file(&src_file, src);

		std::fstream dest_file;
		dest_file.open(dest, std::ios::out | std::ios::binary);
		if(dest_file.is_open() == false){
			llog::error("couldn't open dest '" + dest + "', " + std::strerror(errno));
			return 0;
		}
		raii::fstream::file file_obj_dest = raii::fstream::file(&dest_file, dest);

		sftp_limits_t limit = sftp_limits(sftp);
		const int buffer_size = limit->max_read_length;
		sftp_limits_free(limit);

		std::queue<buffque> queue;

		constexpr int max_requests = 5;
		int requests_sent = 0, bytes_written, bytes_requested;
		std::uintmax_t src_size = 0, dest_size = 0, total_bytes_read = 0, total_bytes_requested = 0;

		sftp_attributes attr = sftp_stat(sftp, src.c_str());
		if(attr == NULL){
			llog::error("couldn't get src size '" + src + "', " + ssh_get_error(sftp->session));
			goto fail;
		}
		src_size = attr->size;
		sftp_attributes_free(attr);

		while(dest_size < src_size){
			while(requests_sent < max_requests && total_bytes_requested < src_size){
				struct buffque bq(buffer_size);

				bytes_requested = sftp_aio_begin_read(src_file, buffer_size, &bq.aio);
				if(bytes_requested == SSH_ERROR){
					llog::error("error reading '" + src + "', " + ssh_get_error(sftp->session));
					goto fail;
				}else if(bytes_requested == 0)
					break;

				requests_sent++;
				queue.push(bq);
				total_bytes_requested += bytes_requested;
			}
read_again:
			if(queue.empty())
				break;

			queue.front().bytes_xfered = sftp_aio_wait_read(&queue.front().aio, queue.front().buffer.data(), buffer_size);
			if(queue.front().bytes_xfered == SSH_ERROR){
				llog::error("error reading '" + src + "', " + ssh_get_error(sftp->session));
				goto fail;
			}else if(queue.front().bytes_xfered == SSH_AGAIN)
				goto read_again;

			total_bytes_read += queue.front().bytes_xfered;

			std::fpos pos = dest_file.tellp();
			dest_file.write(queue.front().buffer.data(), queue.front().bytes_xfered);
			bytes_written = dest_file.tellp() - pos;
			if(bytes_written != queue.front().bytes_xfered || dest_file.bad() || dest_file.fail()){
				llog::error("error writing '" + dest + "', " + std::strerror(errno));
				goto fail;
			}

			dest_size += bytes_written;
			queue.pop();
			requests_sent--;
		}

		if(dest_size != src_size){
			llog::error("error occured while syncing '" + dest + "', mismatch src/dest sizes");
			goto fail;
		}

		return 1;
fail:
		while(queue.empty() != true){
			sftp_aio_free(queue.front().aio);
			queue.pop();
		}

		return 0;
	}

	int mkdir(const std::string& src, const std::string& dest, const sftp_session& sftp){
		std::error_code ec;
		cppfs::mkdir(dest, ec);
		if(llog::ec(dest, ec, "couldn't make directory", NO_EXIT) == false)
			return 0;

		return 1;
	}
	int symlink(const std::string& src, const std::string& dest, const sftp_session& sftp){
		std::error_code ec;
		char* target = sftp_readlink(sftp, src.c_str());
		raii::sftp::link_target link_obj = raii::sftp::link_target(&target);
		cppfs::symlink(target, dest, ec);
		if(llog::ec(dest, ec, "couldn't make symlink", NO_EXIT) == false)
			return 0;

		return 1;
	}
}

#endif // REMOTE_ENABLED
