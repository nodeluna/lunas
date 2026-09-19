#pragma once

#include <cassert>

#include <string>
#include <expected>
#include <memory>

#include "../types.hpp"

#include "sftp/sftp.hpp"
#include "error/error.hpp"
#include "stdout/stdout.hpp"

#ifdef REMOTE_ENABLED

namespace lunas
{
	namespace remote
	{
		std::expected<lunas::syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
								  const std::unique_ptr<lunas::sftp>& src_sftp,
								  const std::unique_ptr<lunas::sftp>& dest_sftp,
								  const lunas::syncmisc&	      misc);
	}
}

#endif // REMOTE_ENABLED
