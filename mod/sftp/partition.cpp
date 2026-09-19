#include <libssh/sftp.h>
#include <libssh/libssh.h>
#include <cassert>

#include <string>
#include <filesystem>

#include "partition.hpp"

namespace lunas
{
	sftp_partition::sftp_partition(const sftp_session& sftp, const std::filesystem::path& path)
	{
		assert(sftp != NULL);
		statvfs = sftp_statvfs(sftp, path.string().c_str());
		if (statvfs == NULL)
		{
			throw std::runtime_error("couldn't get partition info for '" + path.string() + "', " +
						 ssh_get_error(sftp->session));
		}
	}

	std::uintmax_t sftp_partition::available()
	{
		return statvfs->f_bavail * statvfs->f_frsize;
	}

	std::uintmax_t sftp_partition::capacity()
	{
		return statvfs->f_bfree * statvfs->f_frsize;
	}

	sftp_partition::~sftp_partition()
	{
		if (statvfs != NULL)
		{
			sftp_statvfs_free(statvfs);
		}
	}
}
