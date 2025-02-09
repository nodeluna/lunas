module;

#include <expected>
#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include <queue>
#include <cstring>
#include <cerrno>
#include <fcntl.h>

export module lunas.sync:local_to_remote;
export import :types;
export import :misc;
export import :remote_attributes;

export import lunas.sftp;
export import lunas.attributes;
export import lunas.file_types;
import lunas.config.options;
import lunas.stdout;
import lunas.cppfs;

#ifdef REMOTE_ENABLED

export namespace lunas {
	namespace local_to_remote {
		std::expected<struct syncstat, lunas::error> copy(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> rfile(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);
		std::expected<struct syncstat, lunas::error> mkdir(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);
		std::expected<struct syncstat, lunas::error> symlink(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);
	}
}

namespace lunas {
	namespace local_to_remote {
		std::expected<struct syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc) {
			std::expected<struct syncstat, lunas::error> syncstat;

			if (misc.file_type == lunas::file_types::regular_file || misc.file_type == lunas::file_types::resume_regular_file) {
				auto func = [&](const std::string& dest_lspart) -> std::expected<struct syncstat, lunas::error> {
					auto syncstat = local_to_remote::rfile(src, dest_lspart, sftp, misc);
					if (syncstat)
						struct lunas::remote::original_name _(sftp, dest_lspart, dest, syncstat.value().code);
					return syncstat;
				};

				syncstat = regular_file_sync(src, dest, misc.src_mtime, func);
			} else if (misc.file_type == lunas::file_types::directory)
				syncstat = local_to_remote::mkdir(src, dest, sftp, misc);
			else if (misc.file_type == lunas::file_types::symlink)
				syncstat = local_to_remote::symlink(src, dest, sftp, misc);
			else
				std::unexpected(
				    lunas::error("can't sync special file '" + src + "'", lunas::error_type::sync_special_file_ignored));

			return syncstat;
		}

		std::expected<struct syncstat, lunas::error> rfile(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc) {
			struct syncstat syncstat;

			std::unique_ptr<std::fstream> src_file = std::make_unique<std::fstream>(src, std::ios::in | std::ios::binary);
			if (not src_file->is_open()) {
				std::unexpected(lunas::error(
				    "couldn't open src '" + src + "', " + std::strerror(errno), lunas::error_type::sync_is_open));
			}

			std::uintmax_t dest_size = 0, src_size = 0;
			{
				auto file_size = lunas::cppfs::file_size(src);
				if (not file_size)
					std::unexpected(file_size.error());
				src_size = file_size.value();
			}

			syncstat.copied_size = src_size;

			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}

			int access_type;
			if (misc.file_type == lunas::file_types::resume_regular_file && misc.options.resume) {
				auto attrs = sftp->attributes(dest, lunas::follow_symlink::yes);
				if (not attrs && attrs.error().value() != lunas::error_type::sftp_no_such_file) {
					return std::unexpected(attrs.error());
				} else if (not attrs) {
					dest_size = attrs.value()->file_size();
					syncstat.copied_size -= dest_size;
				}
				access_type = O_WRONLY | O_CREAT | O_APPEND;
			} else
				access_type = O_WRONLY | O_CREAT | O_TRUNC;

			auto perms = lunas::permissions::get(src, lunas::follow_symlink::yes);
			if (not perms)
				return std::unexpected(perms.error());

			std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error> dest_file =
			    sftp->openfile(dest, access_type, perms.value());
			if (not dest_file)
				return std::unexpected(dest_file.error());

			{
				auto ok = lunas::ownership::local_to_remote(src, dest, sftp, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}

			auto limits = sftp->limits();
			if (not limits)
				return std::unexpected(limits.error());
			const std::uint64_t buffer_size = limits.value()->max_write_length();

			std::queue<buffque> queue;

			constexpr int			 max_requests  = 5;
			int				 requests_sent = 0;
			std::expected<int, lunas::error> bytes_written;
			std::uintmax_t			 total_bytes_requested = dest_size, position = dest_size;
			class progress_bar		 progress_bar(misc.options.progress_bar, misc.options.quiet);

			while (dest_size < src_size) {
				{
					struct buffque bq(buffer_size);
					if (src_file->eof())
						goto done_reading;

					src_file->seekg(position);
					if (src_file->bad() || src_file->fail()) {
						std::string err = "error reading '" + src + "', " + std::strerror(errno);
						return std::unexpected(lunas::error(err, lunas::error_type::sync_error_reading));
					}
					src_file->read(bq.buffer.data(), buffer_size);
					if (src_file->bad()) {
						std::string err = "error reading '" + src + "', " + std::strerror(errno);
						return std::unexpected(lunas::error(err, lunas::error_type::sync_error_reading));
					}
					bq.bytes_xfered = src_file->gcount();
					position += bq.bytes_xfered;
					queue.push(std::move(bq));
				}
			done_reading:

				if (requests_sent >= max_requests || (total_bytes_requested >= src_size && requests_sent > 0)) {
					bytes_written = dest_file.value()->aio_wait_write(queue.front().aio.value());
					if (not bytes_written) {
						std::string err = "error writing '" + dest + "', " + sftp->get_str_error();
						return std::unexpected(lunas::error(err, lunas::error_type::sync_error_writing));
					} else if (bytes_written.value() != queue.front().bytes_xfered) {
						std::string err = "error writing '" + dest +
								  "', sftp server: mismatch in the number of requested and written bytes";
						return std::unexpected(lunas::error(err, lunas::error_type::sync_error_writing));
					}
					requests_sent--;
					dest_size += bytes_written.value();
					progress_bar.ingoing(src_size, dest_size);
					queue.pop();
				}

				if (total_bytes_requested >= src_size)
					continue;

				queue.back().aio = dest_file.value()->aio_begin_write(queue.back().buffer, queue.back().bytes_xfered);
				if (not queue.back().aio)
					return std::unexpected(queue.back().aio.error());

				total_bytes_requested += queue.back().aio.value()->get_bytes_requested();
				requests_sent++;
			}

			if (dest_size != src_size) {
				std::string err = "error occured while syncing '" + dest + "', mismatch src/dest sizes";
				return std::unexpected(lunas::error(err, lunas::error_type::sync_size_mismatch));
			}

			if (misc.options.fsync) {
				auto ok = dest_file.value()->fsync();
				if (not ok)
					lunas::warn("couldn't fsync '{}', ", dest, ok.error().message());
			}

			syncstat.code = lunas::sync_code::success;
			return syncstat;
		}

		std::expected<struct syncstat, lunas::error> mkdir(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc) {
			struct syncstat syncstat;
			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}

			auto perms = lunas::permissions::get(src, misc.options.follow_symlink);
			if (not perms) {
				return std::unexpected(perms.error());
			}

			auto ok = sftp->mkdir(dest, perms.value());
			if (not ok)
				return std::unexpected(ok.error());

			auto ok2 = lunas::ownership::local_to_remote(src, dest, sftp, misc);
			if (not ok2)
				return std::unexpected(ok2.error());

			syncstat.code = lunas::sync_code::success;
			return syncstat;
		}

		std::expected<struct syncstat, lunas::error> symlink(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc) {
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

			{
				auto ok = sftp->symlink(target, dest);
				if (not ok)
					return std::unexpected(ok.error());
			}

			{
				auto ok = lunas::ownership::local_to_remote(src, dest, sftp, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}

			syncstat.code = lunas::sync_code::success;
			return syncstat;
		}
	}

}
#endif // REMOTE_ENABLED
