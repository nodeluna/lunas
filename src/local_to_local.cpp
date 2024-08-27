#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <future>
#include <cstring>
#include <cerrno>
#include <mutex>
#include "local_to_local.h"
#include "file_types.h"
#include "copy.h"
#include "local_copy.h"
#include "cppfs.h"
#include "log.h"
#include "raii_fstream.h"
#include "local_attrs.h"

std::mutex file_mutex;

std::future<std::pair<std::vector<char>*, long int>> fstream_aio_read_begin(std::fstream& src_file, std::vector<char>* buffer, unsigned long int &position){
	return std::async(std::launch::async, [&src_file, buffer, &position](){
			std::lock_guard<std::mutex>lock_file(file_mutex);

			if(src_file.eof())
				return std::make_pair(buffer, (long int) 0);

			src_file.seekg(position);
			if(src_file.fail() || src_file.bad())
				return std::make_pair(buffer, (long int)-1);

			src_file.read(buffer->data(), buffer->size());
			if(src_file.bad())
				return std::make_pair(buffer, (long int)-1);
			position += src_file.gcount();

			return std::make_pair(buffer, src_file.gcount());
		});
}


namespace local_to_local {
	struct syncstat copy(const std::string& src, const std::string& dest, const short& type){
		struct syncstat syncstat;

		if(type == REGULAR_FILE){
			syncstat = local_to_local::rfile(src, dest);
		}else if(type == DIRECTORY)
			syncstat = local_to_local::mkdir(src, dest);
		else if(type == SYMLINK)
			syncstat = local_to_local::symlink(src, dest);
		else{
			llog::error("can't copy special file '" + src + "'");
			return syncstat;
		}

		return syncstat;
	}

	struct syncstat rfile(const std::string& src, const std::string& dest){
		struct syncstat syncstat;
		std::error_code ec;
		std::uintmax_t src_size = std::filesystem::file_size(src, ec), dest_size = 0, total_bytes_requested = 0;
		if(llog::ec(src, ec, "couldn't get src size", NO_EXIT) == false)
			return syncstat;
		syncstat.copied_size = src_size;

		std::fstream src_file(src, std::ios::in | std::ios::binary);
		if(src_file.is_open() == false){
			llog::error("couldn't open src '" + src + "', " + std::strerror(errno));
			return syncstat;
		}

		raii::fstream::file file_obj_src = raii::fstream::file(&src_file, src);
		std::fstream dest_file(dest, std::ios::out | std::ios::binary);
		if(dest_file.is_open() == false){
			llog::error("couldn't open dest '" + dest + "', " + std::strerror(errno));
			return syncstat;
		}
		raii::fstream::file file_obj_dest = raii::fstream::file(&dest_file, dest);

		if(local_attrs::sync_permissions(src, dest) != 1)
			return syncstat;

		int max_requests = 5, requests_sent = 0, bytes_written;
		const std::uint64_t buffer_size = LOCAL_BUFFER_SIZE;
		unsigned long int position = 0;
		std::queue<lbuffque> queue;


		while(dest_size < src_size){
			while(requests_sent < max_requests && total_bytes_requested < src_size){
				struct lbuffque bq(buffer_size);
				queue.push(std::move(bq));
				queue.back().future = fstream_aio_read_begin(src_file, &queue.back().buffer, position);
				requests_sent++;
				total_bytes_requested += buffer_size;
			}

			if(queue.empty())
				break;
				
			auto result = queue.front().future.get();
			if(result.second == -1){
				llog::error("error reading '" + src + "', " + std::strerror(errno));
				goto fail;
			}else if(result.second == 0){
				break;
			}

			std::fpos pos = dest_file.tellp();
			dest_file.write(queue.front().buffer.data(), result.second);
			bytes_written = dest_file.tellp() - pos;
			if(bytes_written != result.second || dest_file.bad() || dest_file.fail()){
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
		
		syncstat.code = 1;
		return syncstat;
fail:
		while(queue.empty() != true){
			queue.pop();
		}

		return syncstat;
	}
	struct syncstat mkdir(const std::string& src, const std::string& dest){
		struct syncstat syncstat;
		std::error_code ec;
		cppfs::mkdir(dest, ec);
		if(llog::ec(dest, ec, "couldn't make directory", NO_EXIT) == false)
			return syncstat;
		int rv = local_attrs::sync_permissions(src, dest);
		if(rv != 1){
			cppfs::remove(dest);
			return syncstat;
		}
		syncstat.code = 1;
		return syncstat;
	}

	struct syncstat symlink(const std::string& src, const std::string& dest){
		struct syncstat syncstat;
		std::error_code ec;
		cppfs::copy(src, dest, ec);
		if(llog::ec(dest, ec, "couldn't make symlink", NO_EXIT) == false)
			return syncstat;
		syncstat.code = 1;
		return syncstat;
	}
}
