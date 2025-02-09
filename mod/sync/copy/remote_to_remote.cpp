module;

#include <string>
#include <expected>
#include <queue>
#include <memory>
#include <fcntl.h>

export module lunas.sync:remote_to_remote;
export import :types;
export import :misc;
export import :remote_attributes;

export import lunas.sftp;
export import lunas.attributes;
export import lunas.file_types;
import lunas.config.options;
import lunas.stdout;

#ifdef REMOTE_ENABLED
export namespace lunas {
	namespace remote_to_remote {
		std::expected<struct syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp,
		    const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> rfile(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp,
		    const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> mkdir(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp,
		    const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> symlink(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp,
		    const struct syncmisc& misc);
	}
}

namespace lunas {
	namespace remote_to_remote {
		std::expected<struct syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp,
		    const struct syncmisc& misc) {

			std::expected<struct syncstat, lunas::error> syncstat;

			if (misc.file_type == lunas::file_types::regular_file || misc.file_type == lunas::file_types::resume_regular_file) {
				auto func = [&](const std::string& dest_lspart) -> std::expected<struct syncstat, lunas::error> {
					std::expected<struct syncstat, lunas::error> syncstat =
					    remote_to_remote::rfile(src, dest_lspart, src_sftp, dest_sftp, misc);
					if (syncstat)
						struct lunas::remote::original_name _(dest_sftp, dest_lspart, dest, syncstat.value().code);
					return syncstat;
				};

				syncstat = regular_file_sync(src, dest, misc.src_mtime, func);
			} else if (misc.file_type == lunas::file_types::directory)
				syncstat = remote_to_remote::mkdir(src, dest, src_sftp, dest_sftp, misc);
			else if (misc.file_type == lunas::file_types::symlink)
				syncstat = remote_to_remote::symlink(src, dest, src_sftp, dest_sftp, misc);
			else
				std::unexpected(
				    lunas::error("can't sync special file '" + src + "'", lunas::error_type::sync_special_file_ignored));

			return syncstat;
		}

		std::expected<struct syncstat, lunas::error> rfile(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp,
		    const struct syncmisc& misc) {
			struct syncstat syncstat;

			std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error> src_file = src_sftp->openfile(src, O_RDONLY, 0);
			if (not src_file)
				return std::unexpected(src_file.error());

			std::uintmax_t src_size = 0, dest_size = 0;
			unsigned int   perms = 0;
			{
				auto attr = src_sftp->attributes(src, lunas::follow_symlink::yes);
				if (not attr)
					return std::unexpected(attr.error());

				src_size = attr.value()->file_size();
				perms	 = attr.value()->permissions();
			}

			syncstat.copied_size = src_size;

			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}

			int access_type;
			if (misc.file_type == lunas::file_types::resume_regular_file && misc.options.resume) {
				auto attrs = dest_sftp->attributes(dest, lunas::follow_symlink::yes);
				if (not attrs && attrs.error().value() != lunas::error_type::sftp_no_such_file) {
					return std::unexpected(attrs.error());
				} else if (not attrs) {
					dest_size = attrs.value()->file_size();
					syncstat.copied_size -= dest_size;
					auto ok = src_file.value()->seek64(dest_size);
					if (not ok)
						return std::unexpected(ok.error());
				}

				access_type = O_WRONLY | O_CREAT | O_APPEND;
			} else
				access_type = O_WRONLY | O_CREAT | O_TRUNC;

			std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error> dest_file =
			    dest_sftp->openfile(dest, access_type, perms);
			if (not dest_file)
				return std::unexpected(dest_file.error());

			{
				auto ok = lunas::ownership::remote_to_remote(src, dest, src_sftp, dest_sftp, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}

			std::uint64_t buffer_size = 0;
			{
				auto dest_limits = dest_sftp->limits();
				if (not dest_limits)
					return std::unexpected(dest_limits.error());
				buffer_size = dest_limits.value()->max_write_length();
			}
			{
				auto src_limits = src_sftp->limits();
				if (not src_limits)
					return std::unexpected(src_limits.error());

				if (src_limits.value()->max_read_length() < buffer_size)
					buffer_size = src_limits.value()->max_read_length();
			}

			std::queue<buffque> rqueue;
			std::queue<buffque> wqueue;

			constexpr int			 max_requests	    = 3;
			int				 read_requests_sent = 0, write_requests_sent = 0;
			std::uintmax_t			 total_write_bytes_requested = dest_size, total_read_bytes_requested = dest_size;
			std::expected<int, lunas::error> bytes_written;
			std::expected<int, lunas::error> read_done;

			class progress_bar progress_bar(misc.options.progress_bar, misc.options.quiet);

			while (dest_size < src_size) {
				while (read_requests_sent < max_requests && total_read_bytes_requested < src_size) {
					struct buffque bq(buffer_size);

					bq.aio = src_file.value()->aio_begin_read(buffer_size);
					if (not bq.aio) {
						return std::unexpected(bq.aio.error());
					} else if (bq.aio.value()->get_bytes_requested() == 0)
						break;

					read_requests_sent++;
					total_read_bytes_requested += bq.aio.value()->get_bytes_requested();
					rqueue.push(std::move(bq));
				}

				if (rqueue.empty() && wqueue.empty())
					break;
				else if (rqueue.empty())
					goto write;

			read_again:
				read_done = src_file.value()->aio_wait_read(rqueue.front().aio.value(), rqueue.front().buffer, buffer_size);
				if (not read_done && read_done.error().value() == lunas::error_type::ssh_again)
					goto read_again;
				else if (not read_done) {
					std::string err = "error reading '" + src + "', " + src_sftp->get_str_error();
					return std::unexpected(lunas::error(err, lunas::error_type::sync_error_reading));
				}

				rqueue.front().bytes_xfered = read_done.value();

			write:
				if (write_requests_sent >= max_requests ||
				    (total_write_bytes_requested >= src_size && write_requests_sent > 0)) {
					bytes_written = dest_file.value()->aio_wait_write(wqueue.front().aio.value());
					if (not bytes_written) {
						std::string err = "error writing '" + dest + "', " + dest_sftp->get_str_error();
						return std::unexpected(lunas::error(err, lunas::error_type::sync_error_writing));
					} else if (bytes_written.value() != wqueue.front().bytes_xfered) {
						std::string err = "error writing '" + dest +
								  "', sftp server: mismatch in the number of requested and written bytes";
						return std::unexpected(lunas::error(err, lunas::error_type::sync_error_writing));
					}

					write_requests_sent--;
					if (read_requests_sent > 0)
						read_requests_sent--;
					dest_size += bytes_written.value();
					progress_bar.ingoing(src_size, dest_size);
					wqueue.pop();
				}

				if (total_write_bytes_requested >= src_size || rqueue.empty())
					continue;
				wqueue.push(std::move(rqueue.front()));
				rqueue.pop();

				wqueue.back().aio = dest_file.value()->aio_begin_write(wqueue.back().buffer, wqueue.back().bytes_xfered);
				if (not wqueue.back().aio)
					return std::unexpected(wqueue.back().aio.error());

				total_write_bytes_requested += wqueue.back().aio.value()->get_bytes_requested();
				write_requests_sent++;
				if (write_requests_sent >= read_requests_sent && read_requests_sent > 0)
					read_requests_sent--;
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
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp,
		    const struct syncmisc& misc) {
			struct syncstat syncstat;
			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}

			auto perms = src_sftp->get_permissions(src, misc.options.follow_symlink);
			if (not perms)
				return std::unexpected(perms.error());

			{
				auto ok = dest_sftp->mkdir(dest, perms.value());
				if (not ok)
					return std::unexpected(ok.error());
			}

			{
				auto ok = lunas::ownership::remote_to_remote(src, dest, src_sftp, dest_sftp, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}

			syncstat.code = lunas::sync_code::success;
			return syncstat;
		}

		std::expected<struct syncstat, lunas::error> symlink(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp,
		    const struct syncmisc& misc) {
			struct syncstat syncstat;
			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}

			auto target = src_sftp->readlink(src);
			if (not target)
				return std::unexpected(target.error());

			{
				auto ok = dest_sftp->symlink(target.value(), dest);
				if (not ok)
					return std::unexpected(ok.error());
			}
			{
				auto ok = lunas::ownership::remote_to_remote(src, dest, src_sftp, dest_sftp, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}

			syncstat.code = lunas::sync_code::success;
			return syncstat;
		}
	}

}
#endif // REMOTE_ENABLED
