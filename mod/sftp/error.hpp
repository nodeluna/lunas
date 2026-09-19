#pragma once

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <string>

#include "error/error.hpp"

namespace lunas
{
	lunas::error ssh_error(const sftp_session& sftp);
	lunas::error ssh_error(const sftp_session& sftp, const std::string& message);
	lunas::error ssh_error(const ssh_session& ssh);
	lunas::error ssh_error(const ssh_session& ssh, const std::string& message);
	lunas::error ssh_error(const std::string& err_msg);
}
