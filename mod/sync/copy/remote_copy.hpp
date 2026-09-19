#pragma once

#include <cassert>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <memory>
#	include <string>
#	include <expected>
#	include <thread>
#	include <chrono>
#endif

#include "../types.hpp"
#include "../stdout.hpp"
#include "local_to_remote.hpp"
#include "remote_to_local.hpp"
#include "remote_to_remote.hpp"
#include "remote_attributes.hpp"

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
