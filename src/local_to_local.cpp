#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <future>
#include <cstring>
#include <cerrno>
#include <mutex>
#include <condition_variable>
#include "local_to_local.h"
#include "file_types.h"
#include "copy.h"
#include "local_copy.h"
#include "cppfs.h"
#include "log.h"
#include "raii_fstream.h"
#include "local_attrs.h"
#include "progress.h"

std::mutex		file_mutex;
std::condition_variable cv;

std::future<void> fstream_aio_read_begin(std::fstream& src_file, std::queue<lbuffque>* queue, unsigned long int& position) {
	return std::async(std::launch::async, [&src_file, queue, &position]() {
		std::unique_lock<std::mutex> lock_file(file_mutex);

		if (src_file.eof()) {
			queue->back().bytes_read = 0;
			goto done;
		}

		src_file.seekg(position);
		if (src_file.fail() || src_file.bad()) {
			queue->back().bytes_read = -1;
			goto done;
		}

		src_file.read(queue->back().buffer.data(), queue->back().buffer.size());
		if (src_file.bad()) {
			queue->back().bytes_read = -1;
			goto done;
		}

		position += src_file.gcount();
		queue->back().bytes_read = src_file.gcount();
	done:
		cv.notify_one();
	});
}

void fstream_aio_wait_read(std::queue<lbuffque>& queue) {
	std::unique_lock<std::mutex> lock(file_mutex);
	cv.wait(lock, [&queue]() { return !queue.empty(); });
}

namespace local_to_local {
	struct syncstat copy(const std::string& src, const std::string& dest, const short& type) {
		struct syncstat syncstat;

		if (type == REGULAR_FILE) {
			syncstat = local_to_local::rfile(src, dest);
		} else if (type == DIRECTORY)
			syncstat = local_to_local::mkdir(src, dest);
		else if (type == SYMLINK)
			syncstat = local_to_local::symlink(src, dest);
		else {
			llog::error("can't copy special file '" + src + "'");
			return syncstat;
		}

		return syncstat;
	}

	struct syncstat rfile(const std::string& src, const std::string& dest) {
		struct syncstat syncstat;
		std::error_code ec;
		std::uintmax_t	src_size = std::filesystem::file_size(src, ec), dest_size = 0, total_bytes_requested = 0;
		if (llog::ec(src, ec, "couldn't get src size", NO_EXIT) == false)
			return syncstat;
		syncstat.copied_size = src_size;
		if (options::dry_run) {
			syncstat.code = 1;
			return syncstat;
		}

		std::fstream src_file(src, std::ios::in | std::ios::binary);
		if (src_file.is_open() == false) {
			llog::error("couldn't open src '" + src + "', " + std::strerror(errno));
			return syncstat;
		}
		raii::fstream::file file_obj_src = raii::fstream::file(&src_file, src);

		std::ios::openmode openmode;

		if (options::resume) {
			dest_size = std::filesystem::file_size(dest, ec);
			if (llog::ec(dest, ec, "couldn't get dest size", NO_EXIT) == false)
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

		if (local_attrs::sync_permissions(src, dest) != 1)
			return syncstat;
		else if (not local_attrs::sync_ownership(src, dest))
			return syncstat;

		int		     max_requests = 5, requests_sent = 0, bytes_written;
		const std::uint64_t  buffer_size = LOCAL_BUFFER_SIZE;
		std::uintmax_t	     position	 = dest_size;
		std::queue<lbuffque> queue;

		struct progress::obj _;

		while (dest_size < src_size) {
			while (requests_sent < max_requests && total_bytes_requested < src_size) {
				struct lbuffque bq(buffer_size);
				queue.push(std::move(bq));
				fstream_aio_read_begin(src_file, &queue, position);
				requests_sent++;
				total_bytes_requested += buffer_size;
			}

			fstream_aio_wait_read(queue);

			if (queue.front().bytes_read == -1) {
				llog::error("error reading '" + src + "', " + std::strerror(errno));
				goto fail;
			} else if (queue.front().bytes_read == 0)
				break;

			std::fpos pos = dest_file.tellp();
			dest_file.write(queue.front().buffer.data(), queue.front().bytes_read);
			bytes_written = dest_file.tellp() - pos;
			if (bytes_written != queue.front().bytes_read || dest_file.bad() || dest_file.fail()) {
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
			queue.pop();
		}

		return syncstat;
	}

	struct syncstat mkdir(const std::string& src, const std::string& dest) {
		struct syncstat syncstat;
		if (options::dry_run) {
			syncstat.code = 1;
			return syncstat;
		}
		std::error_code ec;
		cppfs::mkdir(dest, ec);
		if (llog::ec(dest, ec, "couldn't make directory", NO_EXIT) == false)
			return syncstat;
		int rv = local_attrs::sync_permissions(src, dest);
		if (rv != 1) {
			cppfs::remove(dest);
			return syncstat;
		} else if (not local_attrs::sync_ownership(src, dest)) {
			cppfs::remove(dest);
			return syncstat;
		}
		syncstat.code = 1;
		return syncstat;
	}

	struct syncstat symlink(const std::string& src, const std::string& dest) {
		struct syncstat syncstat;
		if (options::dry_run) {
			syncstat.code = 1;
			return syncstat;
		}
		std::error_code ec;
		std::string	target = fs::read_symlink(src, ec);
		if (llog::ec(src, ec, "couldn't read symlink", NO_EXIT) == false)
			return syncstat;

		cppfs::symlink(target, dest, ec);
		if (llog::ec(dest, ec, "couldn't make symlink", NO_EXIT) == false)
			return syncstat;

		if (not local_attrs::sync_ownership(src, dest)) {
			cppfs::remove(dest);
			return syncstat;
		}
		syncstat.code = 1;
		return syncstat;
	}
}
