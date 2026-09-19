#pragma once

#include <cassert>
#include <thread>

#include <expected>
#include <string>
#include <memory>

#include "../types.hpp"
#include "sftp/sftp.hpp"

#include "error/error.hpp"

namespace lunas
{
#ifdef REMOTE_ENABLED
	std::expected<syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
						   const std::unique_ptr<lunas::sftp>& src_sftp,
						   const std::unique_ptr<lunas::sftp>& dest_sftp, const struct syncmisc& misc);

#else
	std::expected<syncstat, lunas::error> copy(const std::string& src, const std::string& dest, const struct syncmisc& misc);

#endif
}
