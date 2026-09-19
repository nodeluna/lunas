#pragma once

#include <libssh/sftp.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string_view>
#	include <string>
#	include <cstdint>
#endif

#include "file_types/file_types.hpp"

namespace lunas
{
	class sftp_attributes {
		private:
			::sftp_attributes attr = NULL;
			std::string	  file_path;

		public:
			sftp_attributes(const sftp_session& sftp, const std::string& path, follow_symlink follow);
			sftp_attributes(const ::sftp_attributes& attribute);
			sftp_attributes(const ::sftp_attributes& attribute, const std::string& path);

			sftp_attributes()
			{
			}

			lunas::file_types file_type();
			std::string	  file_type_name();
			bool		  exists();
			sftp_attributes	  release();
			std::string	  path();

			std::string	  name();
			std::string	  longname();
			uint32_t	  flags();
			uint8_t		  type();
			std::uintmax_t	  file_size();
			uint32_t	  uid();
			uint32_t	  gid();
			std::string	  owner();
			std::string	  group();
			uint32_t	  permissions();
			uint64_t	  atime64();
			uint32_t	  atime();
			uint32_t	  atime_nseconds();
			uint64_t	  createtime();
			uint32_t	  createtime_nseconds();
			uint64_t	  mtime64();
			uint32_t	  mtime();
			uint32_t	  mtime_nseconds();
			std::string	  acl();
			uint32_t	  extended_count();
			std::string	  extended_type();
			std::string	  extended_data();

			~sftp_attributes();
	};
}
