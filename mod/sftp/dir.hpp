#pragma once

#include <libssh/sftp.h>

#include <string>
#include <memory>
#include <expected>

#include "attributes.hpp"
#include "error/error.hpp"

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
