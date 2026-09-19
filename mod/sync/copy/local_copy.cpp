#include <expected>
#include <string>

#include "local_copy.hpp"

namespace lunas
{
	namespace local
	{
		std::expected<lunas::syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
								  const lunas::syncmisc& misc)
		{
			lunas::print_sync(src, dest, misc);

			auto syncstat = local_to_local::copy(src, dest, misc);
			if (not syncstat)
			{
				return std::unexpected(syncstat.error());
			}

			if (misc.options.dry_run == false && syncstat.value().code == lunas::sync_code::success)
			{
				auto ok = lunas::utimes::local(src, dest, misc);
				if (not ok)
				{
					return std::unexpected(ok.error());
				}
			}

			return syncstat;
		}
	}
}
