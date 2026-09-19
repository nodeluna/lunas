#pragma once

#include <fcntl.h>

#include <string>
#include <expected>

#include "../types.hpp"

#include "sftp/sftp.hpp"
#include "file_types/file_types.hpp"
#include "config/options.hpp"
#include "stdout/stdout.hpp"
#include "cppfs/cppfs.hpp"

#ifdef REMOTE_ENABLED

namespace lunas
{
	namespace local_to_remote
	{
		std::expected<struct syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
								  const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> rfile(const std::string& src, const std::string& dest,
								   const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> mkdir(const std::string& src, const std::string& dest,
								   const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> symlink(const std::string& src, const std::string& dest,
								     const std::unique_ptr<lunas::sftp>& sftp, const struct syncmisc& misc);
	}
}

#endif // REMOTE_ENABLED
