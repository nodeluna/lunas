module;

#include <system_error>
#include <fcntl.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <fstream>
#	include <expected>
#	include <memory>
#	include <cstring>
#	include <cerrno>
#	include <queue>
#	include <vector>
#	include <filesystem>
#endif

export module lunas.sync:remote_to_local;
export import :types;
export import :misc;
export import :remote_attributes;

export import lunas.error;
export import lunas.sftp;
export import lunas.file_types;
export import lunas.cppfs;
export import lunas.stdout;

export namespace lunas {
#ifdef REMOTE_ENABLED
	namespace remote_to_local {
		std::expected<struct syncstat, lunas::error> copy(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> rfile(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> mkdir(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> symlink(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);
	}
#endif // REMOTE_ENABLED
}

namespace lunas {
#ifdef REMOTE_ENABLED
	namespace remote_to_local {
		std::expected<struct syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc) {

			std::expected<struct syncstat, lunas::error> syncstat;

			if (misc.file_type == lunas::file_types::regular_file || misc.file_type == lunas::file_types::resume_regular_file) {
				auto func = [&](const std::string& dest_lspart) -> std::expected<struct syncstat, lunas::error> {
					auto syncstat = remote_to_local::rfile(src, dest_lspart, sftp, misc);
					if (syncstat)
						struct lunas::local::original_name _(
						    dest_lspart, dest, syncstat.value().code, misc.options.dry_run);
					return syncstat;
				};

				syncstat = regular_file_sync(src, dest, misc.src_mtime, func);
			} else if (misc.file_type == lunas::file_types::directory)
				syncstat = remote_to_local::mkdir(src, dest, sftp, misc);
			else if (misc.file_type == lunas::file_types::symlink)
				syncstat = remote_to_local::symlink(src, dest, sftp, misc);
			else
				std::unexpected(
				    lunas::error("can't sync special file '" + src + "'", lunas::error_type::sync_special_file_ignored));

			return syncstat;
		}

		std::expected<struct syncstat, lunas::error> rfile(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc) {
			struct syncstat syncstat;

			std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error> src_file = sftp->openfile(src, O_RDONLY, 0);
			if (not src_file)
				return std::unexpected(src_file.error());

			std::uintmax_t	       src_size = 0, dest_size = 0;
			std::filesystem::perms perms;

			{
				auto attr = sftp->attributes(src, lunas::follow_symlink::yes);
				if (not attr)
					return std::unexpected(attr.error());

				src_size = attr.value()->file_size();
				perms	 = ( std::filesystem::perms ) attr.value()->permissions();
			}

			syncstat.copied_size = src_size;

			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}

			std::error_code	   ec;
			std::ios::openmode openmode;

			if (misc.options.resume) {
				auto file_size = lunas::cppfs::file_size(dest);
				if (not file_size && file_size.error().value() != lunas::error_type::no_such_file)
					return std::unexpected(file_size.error());
				if (file_size) {
					dest_size = file_size.value();
					syncstat.copied_size -= dest_size;

					auto ok = src_file.value()->seek64(dest_size);
					if (not ok)
						return std::unexpected(ok.error());
				}

				openmode = std::ios::out | std::ios::binary | std::ios::app;
			} else {
				openmode = std::ios::out | std::ios::binary;
			}

			std::unique_ptr<std::fstream> dest_file = std::make_unique<std::fstream>(dest, openmode);
			if (dest_file->is_open() == false)
				return std::unexpected(lunas::error(
				    "couldn't open dest '" + dest + "', " + std::strerror(errno), lunas::error_type::sync_is_open));

			{
				auto ok = lunas::permissions::set(dest, perms, lunas::follow_symlink::yes);
				if (not ok)
					return std::unexpected(ok.error());
			}
			{
				auto ok = lunas::ownership::remote_to_local(src, dest, sftp, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}

			std::uint64_t buffer_size = 0;
			{
				auto limits = sftp->limits();
				if (not limits)
					return std::unexpected(limits.error());
				buffer_size = limits.value()->max_read_length();
			}

			std::queue<buffque> queue;

			constexpr int	   max_requests		 = 5;
			int		   requests_sent	 = 0, bytes_written;
			std::uintmax_t	   total_bytes_requested = dest_size;
			class progress_bar progress_bar(misc.options.progress_bar, misc.options.quiet);

			while (dest_size < src_size) {
				while (requests_sent < max_requests && total_bytes_requested < src_size) {
					struct buffque bq(buffer_size);

					bq.aio = src_file.value()->aio_begin_read(buffer_size);
					if (not bq.aio) {
						return std::unexpected(bq.aio.error());
					} else if (bq.aio.value()->get_bytes_requested() == 0)
						break;

					requests_sent++;
					total_bytes_requested += bq.aio.value()->get_bytes_requested();
					queue.push(std::move(bq));
				}
			read_again:
				if (queue.empty())
					break;

				auto read_done =
				    src_file.value()->aio_wait_read(queue.front().aio.value(), queue.front().buffer, buffer_size);
				if (not read_done && read_done.error().value() == lunas::error_type::ssh_again)
					goto read_again;
				else if (not read_done) {
					std::string err = "error reading '" + src + "', " + sftp->get_str_error();
					return std::unexpected(lunas::error(err, lunas::error_type::sync_error_reading));
				}

				queue.front().bytes_xfered = read_done.value();

				std::fpos pos = dest_file->tellp();
				dest_file->write(queue.front().buffer.data(), queue.front().bytes_xfered);
				bytes_written = dest_file->tellp() - pos;
				if (bytes_written != queue.front().bytes_xfered || dest_file->bad() || dest_file->fail()) {
					std::string err = "error writing '" + dest + "', " + std::strerror(errno);
					return std::unexpected(lunas::error(err, lunas::error_type::sync_error_writing));
				}

				dest_size += bytes_written;
				progress_bar.ingoing(src_size, dest_size);
				queue.pop();
				requests_sent--;
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

		std::expected<struct syncstat, lunas::error> mkdir(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc) {
			struct syncstat syncstat;
			if (misc.options.dry_run) {
				syncstat.code = lunas::sync_code::success;
				return syncstat;
			}

			{
				auto ok = lunas::cppfs::mkdir(dest, misc.options.dry_run);
				if (not ok)
					return std::unexpected(ok.error());
			}
			{
				auto ok = lunas::permissions::remote_to_local(src, dest, sftp, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}
			{
				auto ok = lunas::ownership::remote_to_local(src, dest, sftp, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}

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

			auto target = sftp->readlink(src);
			if (not target)
				return std::unexpected(target.error());

			{
				auto ok = lunas::cppfs::symlink(target.value(), dest, misc.options.dry_run);
				if (not ok)
					return std::unexpected(ok.error());
			}

			{
				auto ok = lunas::ownership::remote_to_local(src, dest, sftp, misc);
				if (not ok)
					return std::unexpected(ok.error());
			}

			syncstat.code = lunas::sync_code::success;
			return syncstat;
		}
	}

#endif // REMOTE_ENABLED
}
