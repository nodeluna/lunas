#pragma once

#include <system_error>
#include <fcntl.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <expected>
#	include <memory>
#	include <string>
#	include <fstream>
#	include <filesystem>
#	include <queue>
#	include <cstring>
#	include <cerrno>
#endif

#include "../types.hpp"
#include "misc.hpp"
#include "remote_attributes.hpp"

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
