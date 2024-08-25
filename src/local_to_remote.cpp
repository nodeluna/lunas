#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <queue>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include "copy.h"
#include "remote_copy.h"
#include "local_to_remote.h"
#include "log.h"
#include "file_types.h"
#include "raii_sftp.h"
#include "raii_fstream.h"


namespace fs = std::filesystem;

namespace local_to_remote {
	struct syncstat copy(const std::string& src, const std::string& dest, const sftp_session& sftp, const short& type){
		struct syncstat syncstat;

		if(type == REGULAR_FILE)
			syncstat = local_to_remote::rfile(src, dest, sftp);
		else if(type == DIRECTORY)
			syncstat = local_to_remote::mkdir(src, dest, sftp);
		else if(type == SYMLINK)
			syncstat = local_to_remote::symlink(src, dest, sftp);
		else
			llog::error("can't sync special file '" + src + "'");

		return syncstat;
	}

	struct syncstat rfile(const std::string& src, const std::string& dest, const sftp_session& sftp){
		std::fstream src_file(src, std::ios::in | std::ios::binary);
		struct syncstat syncstat;
		if(!src_file.is_open()){
			llog::error("couldn't open src '" + src + "', " + std::strerror(errno));
			return syncstat;
		}
		raii::fstream::file file_obj_src = raii::fstream::file(&src_file, src);

		std::uintmax_t dest_size = 0, src_size = fs::file_size(src), total_bytes_requested = 0, position = 0;
		syncstat.copied_size = src_size;
		if(options::dry_run){
			syncstat.code = 1;
			return syncstat;
		}

		int access_type = O_WRONLY | O_CREAT | O_TRUNC;
		int perms = S_IRWXU;
		sftp_file dest_file = sftp_open(sftp, (dest+".ls.part").c_str(), access_type, perms);
		if(dest_file == NULL){
			llog::error("couldn't open dest '" + dest + "', " + ssh_get_error(sftp->session));
			return syncstat;
		}
		raii::sftp::file file_obj_dest = raii::sftp::file(&dest_file, dest);
		int rc;

		sftp_limits_t limit = sftp_limits(sftp);
		const std::uint64_t buffer_size = limit->max_write_length;
		sftp_limits_free(limit);

		std::queue<buffque> queue;

		constexpr int max_requests = 5;
		int requests_sent = 0, bytes_written, bytes_requested;

		while(dest_size < src_size){
			struct buffque bq(buffer_size);
			if(src_file.eof())
				goto done_reading;

			src_file.seekg(position);
			if(src_file.bad() || src_file.fail()){
				llog::error("error reading '" + src + "', " + std::strerror(errno));
				goto fail;
			}
			src_file.read(bq.buffer.data(), buffer_size);
			if(src_file.bad()){
				llog::error("error reading '" + src + "', " + std::strerror(errno));
				goto fail;
			}
			bq.bytes_xfered = src_file.gcount();
			position += src_file.gcount();
			queue.push(bq);
done_reading:

			if(requests_sent >= max_requests || (total_bytes_requested  >= src_size && requests_sent > 0)){
				bytes_written = sftp_aio_wait_write(&queue.front().aio);
				if(bytes_written == SSH_ERROR){
					llog::error("error writing '" + dest + "', " + ssh_get_error(sftp->session));
					goto fail;
				}else if(bytes_written != queue.front().bytes_xfered){
					llog::error("error writing '" + dest + "', " + ssh_get_error(sftp->session));
					goto fail;
				}
				requests_sent--;
				dest_size += bytes_written;
				queue.pop();
			}

			if(total_bytes_requested >= src_size)
				continue;

			bytes_requested = sftp_aio_begin_write(dest_file, queue.back().buffer.data(), queue.back().bytes_xfered, &queue.back().aio);
			if(bytes_requested == SSH_ERROR){
				llog::error("error writing '" + dest + "', " + ssh_get_error(sftp->session));
				goto fail;
			}
			total_bytes_requested += bytes_requested;
			requests_sent++;
		}

		if(dest_size != src_size){
			llog::error("error occured while syncing '" + dest + "', mismatch src/dest sizes");
			goto fail;
		}

		rc = sftp_rename(sftp, (dest+".ls.part").c_str(), dest.c_str());
		if(llog::rc(sftp, dest, rc, "couldn't rename file to its original name", NO_EXIT) == false)
			return syncstat;
		syncstat.code = 1;
		return syncstat;
fail:
		while(!queue.empty()){
			sftp_aio_free(queue.front().aio);
			queue.pop();
		}
		return syncstat;
	}

	struct syncstat mkdir(const std::string& src, const std::string& dest, const sftp_session& sftp){
		struct syncstat syncstat;
		if(options::dry_run){
			syncstat.code = 1;
			return syncstat;
		}

		int rc = sftp::mkdir(sftp, dest);
		if(llog::rc(sftp, dest, rc, "couldn't make directory", NO_EXIT) == false)
			return syncstat;

		syncstat.code = 1;
		return syncstat;
	}

	struct syncstat symlink(const std::string& src, const std::string& dest, const sftp_session& sftp){
		struct syncstat syncstat;
		if(options::dry_run){
			syncstat.code = 1;
			return syncstat;
		}

		std::error_code ec;
		std::string target = fs::read_symlink(src, ec);
		if(llog::ec(src, ec, "couldn't read symlink", NO_EXIT) == false)
			return syncstat;

		int rc = sftp::symlink(sftp, target, dest);
		if(llog::rc(sftp, dest, rc, "couldn't make symlink", NO_EXIT) == false)
			return syncstat;

		syncstat.code = 1;
		return syncstat;
	}
}


#endif // REMOTE_ENABLED
