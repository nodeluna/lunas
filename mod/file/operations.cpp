#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <expected>
#	include <variant>
#	include <memory>
#	include <string>
#endif

#include "operations.hpp"

namespace lunas
{
	namespace file_operations
	{
		std::expected<std::monostate, lunas::error> mkdir(const std::unique_ptr<lunas::sftp>& sftp, const std::string& path,
								  bool dry_run)
		{
			if (sftp != nullptr)
			{
				auto ok = sftp->mkdir(path);
				if (not ok)
				{
					return std::unexpected(ok.error());
				}
			}
			else
			{
				auto ok = lunas::cppfs::mkdir(path, dry_run);
				if (not ok)
				{
					return std::unexpected(ok.error());
				}
			}

			return std::monostate();
		}
	}
}
