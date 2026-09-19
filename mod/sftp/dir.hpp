#pragma once

#include <libssh/sftp.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <memory>
#	include <exception>
#	include <expected>
#	include <filesystem>
#endif

#include "attributes.hpp"
#include "error.hpp"
#include "log.hpp"

namespace lunas
{
	class sftp_dir {
		private:
			::sftp_dir m_dir = NULL;
			bool	   m_eof = false;

		public:
			sftp_dir(const sftp_session& sftp, const std::string& path);
			std::expected<std::unique_ptr<lunas::sftp_attributes>, lunas::error> read(const sftp_session& sftp);
			std::expected<bool, lunas::error> eof(const sftp_session& sftp, const std::string& path);
			~sftp_dir();
	};
}
