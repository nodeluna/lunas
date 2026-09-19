#include <libssh/sftp.h>

#include <stdexcept>
#include <string>

#include "limits.hpp"

namespace lunas
{
	sftp_limits::sftp_limits(const sftp_session& sftp)
	{
		limits = ::sftp_limits(sftp);
		if (limits == NULL)
		{
			throw std::runtime_error(std::string("couldn't get sftp_limits, ") + ssh_get_error(sftp->session));
		}
	}

	uint64_t sftp_limits::max_packet_length()
	{
		return limits->max_packet_length;
	}

	uint64_t sftp_limits::max_read_length()
	{
		return limits->max_read_length;
	}

	uint64_t sftp_limits::max_write_length()
	{
		return limits->max_write_length;
	}

	uint64_t sftp_limits::max_open_handles()
	{
		return limits->max_open_handles;
	}

	sftp_limits::~sftp_limits()
	{
		if (limits != NULL)
		{
			sftp_limits_free(limits);
		}
	}
}
