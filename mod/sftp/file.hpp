#pragma once

#include <libssh/sftp.h>

#include <expected>
#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#include "error/error.hpp"

namespace lunas
{
	class sftp_file;

	class sftp_aio {
		private:
			::sftp_aio		  aio		  = nullptr;
			int			  bytes_requested = 0;
			[[nodiscard]] ::sftp_aio* get_aio_handle();

			friend class lunas::sftp_file;

		public:
			sftp_aio();
			void set_bytes_requested(int nbytes);
			int  get_bytes_requested();
			~sftp_aio();
	};

	class sftp_file {
		private:
			::sftp_file file = nullptr;

		public:
			sftp_file(const sftp_session& sftp, const std::string& path, int access_type, mode_t mode);
			sftp_file(const sftp_session& sftp, const std::string& path, int access_type, std::filesystem::perms mode);
			~sftp_file();
			[[nodiscard]] std::expected<std::unique_ptr<lunas::sftp_aio>, lunas::error>
								       aio_begin_write(const std::vector<char>& buffer, size_t length);
			[[nodiscard]] std::expected<int, lunas::error> aio_wait_write(std::unique_ptr<lunas::sftp_aio>& aio);

			[[nodiscard]] std::expected<std::unique_ptr<lunas::sftp_aio>, lunas::error> aio_begin_read(size_t length);
			[[nodiscard]] std::expected<int, lunas::error> aio_wait_read(std::unique_ptr<lunas::sftp_aio>& aio,
										     std::vector<char>& buffer, size_t length);

			std::expected<std::monostate, lunas::error>    fsync();
			std::expected<std::monostate, lunas::error>    seek64(uint64_t new_offset);
	};
}
