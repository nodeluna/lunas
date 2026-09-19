#pragma once

#include <fcntl.h>

#include <string>
#include <expected>
#include <memory>

#include "../types.hpp"

#include "error/error.hpp"
#include "sftp/sftp.hpp"
#include "file_types/file_types.hpp"
#include "cppfs/cppfs.hpp"
#include "stdout/stdout.hpp"

namespace lunas
{
#ifdef REMOTE_ENABLED
	namespace remote_to_local
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
#endif // REMOTE_ENABLED
}
