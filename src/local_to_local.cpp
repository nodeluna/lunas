#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <cstring>
#include <cerrno>
#include <cstdint>
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

void fstream_aio_read_begin(std::fstream* src_file, std::queue<lbuffque>* queue, std::uintmax_t position, size_t queue_limit) {
	const std::uint64_t buffer_size = LOCAL_BUFFER_SIZE;
	while (not src_file->eof()) {
		std::unique_lock<std::mutex> lock_file(file_mutex);
		if (queue->size() >= queue_limit)
			cv.wait(lock_file, [&queue, &queue_limit]() { return queue->size() < queue_limit; });

		{
			struct lbuffque bq(buffer_size);
			queue->push(std::move(bq));
		}

		src_file->seekg(position);
		if (src_file->fail() || src_file->bad()) {
			queue->back().bytes_read = -1;
			goto done;
		}

		src_file->read(queue->back().buffer.data(), queue->back().buffer.size());
		if (src_file->bad()) {
			queue->back().bytes_read = -1;
			goto done;
		}

		queue->back().bytes_read = src_file->gcount();
		position += queue->back().bytes_read;
	done:
		lock_file.unlock();
		cv.notify_one();
	}
}

struct lbuffque fstream_aio_wait_read(std::queue<lbuffque>& queue) {
	std::unique_lock<std::mutex> lock(file_mutex);
	cv.wait(lock, [&queue]() { return !queue.empty(); });

	struct lbuffque lbuffque = std::move(queue.front());
	queue.pop();

	lock.unlock();
	cv.notify_one();
	return lbuffque;
}

namespace local_to_local {
	struct syncstat copy(const std::string& src, const std::string& dest, const struct syncmisc& misc) {
		struct syncstat syncstat;

		if (misc.file_type == REGULAR_FILE) {
			auto func = [&](const std::string& dest_lspart) -> struct syncstat {
				struct syncstat		       st = local_to_local::rfile(src, dest_lspart);
				struct fs_local::original_name _(dest_lspart, dest, st.code);
				return st;
			};

			syncstat = regular_file_sync(src, dest, misc.src_mtime, func);
		} else if (misc.file_type == DIRECTORY)
			syncstat = local_to_local::mkdir(src, dest);
		else if (misc.file_type == SYMLINK)
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
		std::uintmax_t	src_size = std::filesystem::file_size(src, ec), dest_size = 0;
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

		int		     bytes_written;
		std::uintmax_t	     position = dest_size;
		std::queue<lbuffque> queue;
		const size_t	     queue_limit = 3;
		auto		     thread	 = std::jthread(fstream_aio_read_begin, &src_file, &queue, position, queue_limit);
		struct progress::obj _;

		while (dest_size < src_size) {
			struct lbuffque lbuffque = fstream_aio_wait_read(queue);

			if (lbuffque.bytes_read == -1) {
				llog::error("error reading '" + src + "', " + std::strerror(errno));
				goto fail;
			} else if (lbuffque.bytes_read == 0)
				break;

			std::fpos pos = dest_file.tellp();
			dest_file.write(lbuffque.buffer.data(), lbuffque.bytes_read);
			bytes_written = dest_file.tellp() - pos;
			if (bytes_written != lbuffque.bytes_read || dest_file.bad() || dest_file.fail()) {
				llog::error("error writing '" + dest + "', " + std::strerror(errno));
				goto fail;
			}

			dest_size += bytes_written;
			progress::ingoing(src_size, dest_size);
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
