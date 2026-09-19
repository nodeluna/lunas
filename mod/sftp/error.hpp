#pragma once

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <typeinfo>
#endif

#include "error/error.hpp"

namespace lunas
{
	lunas::error ssh_error(const sftp_session& sftp);
	lunas::error ssh_error(const sftp_session& sftp, const std::string& message);
	lunas::error ssh_error(const ssh_session& ssh);
	lunas::error ssh_error(const ssh_session& ssh, const std::string& message);
	lunas::error ssh_error(const std::string& err_msg);
}
