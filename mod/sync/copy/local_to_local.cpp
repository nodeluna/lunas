module;

#include <string>
#include <fstream>
#include <filesystem>
#include <queue>
#include <expected>
#include <thread>
#include <mutex>
#include <cstring>
#include <cerrno>
#include <cstdint>
#include <condition_variable>
#include <memory>
#include <utility>

export module lunas.sync:local_to_local;
export import :types;
export import :misc;
export import :local_attributes;
export import lunas.file_types;
export import lunas.error;
export import lunas.config.options;
export import lunas.cppfs;
export import lunas.stdout;

#define LOCAL_BUFFER_SIZE 262144

struct lbuffque {
		std::vector<char> buffer;
		long int	  bytes_read = 0;

		explicit lbuffque(std::uint64_t size) : buffer(size) {
		}
};

export namespace lunas {
	class jthread {
		private:
			std::thread thread;

		public:
			jthread(const jthread&)		   = delete;
			jthread& operator=(const jthread&) = delete;

			template<typename function, typename... args_t>
			jthread(function&& func, args_t&&... args) : thread(std::forward<function>(func), std::forward<args_t>(args)...) {
			}

			~jthread() {
				if (thread.joinable())
					thread.join();
			}
	};

	void fstream_aio_read_begin(
	    std::unique_ptr<std::fstream>& src_file, std::queue<lbuffque>* queue, std::uintmax_t position, size_t queue_limit);
	struct lbuffque fstream_aio_wait_read(std::queue<lbuffque>& queue);

	namespace local_to_local {
		std::expected<struct syncstat, lunas::error> copy(
		    const std::string& src, const std::string& dest, const struct syncmisc& misc);
		std::expected<struct syncstat, lunas::error> rfile(
		    const std::string& src, const std::string& dest, const struct syncmisc& misc);
		std::expected<struct syncstat, lunas::error> mkdir(
		    const std::string& src, const std::string& dest, const struct syncmisc& misc);
		std::expected<struct syncstat, lunas::error> symlink(
		    const std::string& src, const std::string& dest, const struct syncmisc& misc);

	}
}

namespace lunas {
	namespace local_to_local {
		std::mutex		file_mutex;
		std::condition_variable cv;

		void fstream_aio_read_begin(
		    std::unique_ptr<std::fstream>& src_file, std::queue<lbuffque>* queue, std::uintmax_t position, size_t queue_limit) {
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

		std::expected<struct syncstat, lunas::error> copy(
		    const std::string& src, const std::string& dest, const struct syncmisc& misc) {
			std::expected<struct syncstat, lunas::error> syncstat;

			if (misc.file_type == lunas::file_types::regular_file || misc.file_type == lunas::file_types::resume_regular_file) {
				auto func = [&](const std::string& dest_lspart) -> std::expected<struct syncstat, lunas::error> {
					auto syncstat = local_to_local::rfile(src, dest_lspart, misc);
					if (syncstat)
						struct lunas::local::original_name _(
						    dest_lspart, dest, syncstat.value().code, misc.options.dry_run);
					return syncstat;
				};

				syncstat = regular_file_sync(src, dest, misc.src_mtime, func);
			} else if (misc.file_type == lunas::file_types::directory)
				syncstat = local_to_local::mkdir(src, dest, misc);
			else if (misc.file_type == lunas::file_types::symlink)
				syncstat = local_to_local::symlink(src, dest, misc);
			else
				return std::unexpected(
				    lunas::error("can't sync special file '" + src + "'", lunas::error_type::sync_special_file_ignored));

			return syncstat;
		}

		std::expected<struct syncstat, lunas::error> rfile(
		    const std::string& src, const std::string& dest, const struct syncmisc& misc) {
			struct syncstat syncstat;
			std::error_code ec;
			std::uintmax_t	src_size = std::filesystem::file_size(src, ec), dest_size = 0;
			if (ec.value() != 0) {
				std::string err = "couldn't get src size '" + src + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::sync_get_file_size));
			}

			syncstat.copied_size = src_size;
			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}

			std::unique_ptr<std::fstream> src_file = std::make_unique<std::fstream>(src, std::ios::in | std::ios::binary);
			if (src_file->is_open() == false) {
				std::string err = "couldn't open src '" + src + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::sync_is_open));
			}

			std::ios::openmode openmode;

			if (misc.options.resume) {
				auto file_size = lunas::cppfs::file_size(dest);
				if (not file_size && file_size.error().value() != lunas::error_type::no_such_file)
					return std::unexpected(file_size.error());
				if (file_size) {
					dest_size = file_size.value();
					syncstat.copied_size -= dest_size;
				}
				openmode = std::ios::out | std::ios::binary | std::ios::app;
			} else {
				openmode = std::ios::out | std::ios::binary;
			}

			std::unique_ptr<std::fstream> dest_file = std::make_unique<std::fstream>(dest, openmode);
			if (dest_file->is_open() == false) {
				std::string err = "couldn't open dest '" + dest + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::sync_is_open));
			}

			{
				auto ok = lunas::permissions::local_to_local(src, dest, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}
			{
				auto ok = lunas::ownership::local_to_local(src, dest, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}

			int		     bytes_written;
			std::uintmax_t	     position = dest_size;
			std::queue<lbuffque> queue;
			const size_t	     queue_limit = 3;
			lunas::jthread thread = lunas::jthread(fstream_aio_read_begin, std::ref(src_file), &queue, position, queue_limit);
			class progress_bar progress_bar(misc.options.progress_bar, misc.options.quiet);

			while (dest_size < src_size) {
				struct lbuffque lbuffque = fstream_aio_wait_read(queue);

				if (lbuffque.bytes_read == -1) {
					std::string err = "error reading '" + src + "', " + std::strerror(errno);
					return std::unexpected(lunas::error(err, lunas::error_type::sync_error_reading));
				} else if (lbuffque.bytes_read == 0)
					break;

				std::fpos pos = dest_file->tellp();
				dest_file->write(lbuffque.buffer.data(), lbuffque.bytes_read);
				bytes_written = dest_file->tellp() - pos;

				if (bytes_written != lbuffque.bytes_read || dest_file->bad() || dest_file->fail()) {
					std::string err = "error writing '" + dest + "', " + std::strerror(errno);
					return std::unexpected(lunas::error(err, lunas::error_type::sync_error_writing));
				}

				dest_size += bytes_written;
				progress_bar.ingoing(src_size, dest_size);
			}

			if (dest_size != src_size) {
				std::string err = "error occured while syncing '" + dest + "', mismatch src/dest sizes";
				return std::unexpected(lunas::error(err, lunas::error_type::sync_size_mismatch));
			}

			if (misc.options.fsync) {
				int rc = dest_file->sync();
				if (rc != 0)
					lunas::warn("couldn't fsync '{}', ", dest, std::strerror(errno));
			}

			syncstat.code = lunas::sync_code::success;
			return syncstat;
		}

		std::expected<struct syncstat, lunas::error> mkdir(
		    const std::string& src, const std::string& dest, const struct syncmisc& misc) {
			struct syncstat syncstat;
			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}
			std::error_code ec;
			auto		ok = lunas::cppfs::mkdir(dest, misc.options.dry_run);
			if (not ok)
				return std::unexpected(ok.error());

			{
				auto ok = lunas::permissions::local_to_local(src, dest, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}
			{
				auto ok = lunas::ownership::local_to_local(src, dest, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}
			syncstat.code = lunas::sync_code::success;
			return syncstat;
		}

		std::expected<struct syncstat, lunas::error> symlink(
		    const std::string& src, const std::string& dest, const struct syncmisc& misc) {
			struct syncstat syncstat;
			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}
			std::error_code ec;
			std::string	target = std::filesystem::read_symlink(src, ec);
			if (ec.value() != 0) {
				std::string err = "couldn't read symlink '" + src + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::sync_read_symlink));
			}

			auto ok = lunas::cppfs::symlink(target, dest, misc.options.dry_run);
			if (not ok)
				return std::unexpected(ok.error());

			{
				auto ok = lunas::ownership::local_to_local(src, dest, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}
			syncstat.code = lunas::sync_code::success;
			return syncstat;
		}
	}

}
