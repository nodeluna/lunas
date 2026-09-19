#pragma once

#include <fcntl.h>

#include <string>
#include <expected>
#include <memory>

#include "../types.hpp"
#include "sftp/sftp.hpp"

#ifdef REMOTE_ENABLED

namespace lunas
{
	namespace remote_to_remote
	{
		std::expected<struct syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
								  const std::unique_ptr<lunas::sftp>& src_sftp,
								  const std::unique_ptr<lunas::sftp>& dest_sftp,
								  const struct syncmisc&	      misc);

		std::expected<struct syncstat, lunas::error> link(const std::string& src, const std::string& dest,
								  const std::unique_ptr<lunas::sftp>& src_sftp,
								  const std::unique_ptr<lunas::sftp>& dest_sftp,
								  const struct syncmisc&	      misc);

		std::expected<struct syncstat, lunas::error> rfile(const std::string& src, const std::string& dest,
								   const std::unique_ptr<lunas::sftp>& src_sftp,
								   const std::unique_ptr<lunas::sftp>& dest_sftp,
								   const struct syncmisc&	       misc);

		std::expected<struct syncstat, lunas::error> mkdir(const std::string& src, const std::string& dest,
								   const std::unique_ptr<lunas::sftp>& src_sftp,
								   const std::unique_ptr<lunas::sftp>& dest_sftp,
								   const struct syncmisc&	       misc);

		std::expected<struct syncstat, lunas::error> symlink(const std::string& src, const std::string& dest,
								     const std::unique_ptr<lunas::sftp>& src_sftp,
								     const std::unique_ptr<lunas::sftp>& dest_sftp,
								     const struct syncmisc&		 misc);
	}
}

#endif // REMOTE_ENABLED
