#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <array>
#include "remote_to_local.h"
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

		int bytes_read = 1, bytes_written, bytes_requested;
		std::array<char, REMOTE_BUFFER_SIZE> buffer;
		sftp_aio aio;

		while(bytes_read > 0){
			bytes_requested = sftp_aio_begin_read(src_file, sizeof(buffer), &aio);
			if(bytes_requested == SSH_ERROR){
				llog::error("error reading '" + src + "', " + ssh_get_error(sftp->session));
				break;
			}
read_again:
			bytes_read = sftp_aio_wait_read(&aio, buffer.data(), sizeof(buffer));
			if(bytes_read == SSH_ERROR){
				sftp_aio_free(aio);
				llog::error("error reading '" + src + "', " + ssh_get_error(sftp->session));
				goto fail;
			}else if(bytes_read == SSH_AGAIN){
				goto read_again;
			}

			std::fpos pos = dest_file.tellp();
			dest_file.write(buffer.data(), bytes_read);
			bytes_written = dest_file.tellp() - pos;
			if(bytes_written != bytes_read || dest_file.bad() || dest_file.fail()){
				llog::error("error writing '" + dest + "', " + std::strerror(errno));
				goto fail;
			}
		}

		return 1;
fail:
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
