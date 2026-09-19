#pragma once

#include <libssh/sftp.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <stdexcept>
#	include <string>
#endif

namespace lunas
{
	class sftp_limits {
		private:
			::sftp_limits_t limits = nullptr;

		public:
			sftp_limits(const sftp_session& sftp);
			~sftp_limits();
			uint64_t max_packet_length();
			uint64_t max_read_length();
			uint64_t max_write_length();
			uint64_t max_open_handles();
	};
}
