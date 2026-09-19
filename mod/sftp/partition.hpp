#pragma once

#include <libssh/sftp.h>
#include <libssh/libssh.h>

#include <filesystem>

namespace lunas
{
	class sftp_partition {
		private:
			sftp_statvfs_t statvfs = NULL;

		public:
			sftp_partition(const sftp_session& sftp, const std::filesystem::path& path);
			std::uintmax_t available();
			std::uintmax_t capacity();
			~sftp_partition();
	};
}
