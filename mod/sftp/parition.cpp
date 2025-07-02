module;

#include <libssh/sftp.h>
#include <libssh/libssh.h>
#include <cassert>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <filesystem>
#endif

export module lunas.sftp:parition;

export namespace lunas
{
	class sftp_partition {
		private:
			sftp_statvfs_t statvfs = NULL;

		public:
			sftp_partition(const sftp_session& sftp, const std::filesystem::path& path);
			std::uintmax_t available();
			std::uintmax_t capacity();
			~sftp_partition();
	};
}

namespace lunas
{
	sftp_partition::sftp_partition(const sftp_session& sftp, const std::filesystem::path& path)
	{
		assert(sftp != NULL);
		statvfs = sftp_statvfs(sftp, path.string().c_str());
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
