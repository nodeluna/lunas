#include <cassert>

#include <string>
#include <expected>
#include <chrono>
#include <thread>

#include "remote_copy.hpp"
#include "../stdout.hpp"
#include "local_to_remote.hpp"
#include "remote_to_local.hpp"
#include "remote_to_remote.hpp"
#include "remote_attributes.hpp"

#ifdef REMOTE_ENABLED

namespace lunas
{
	namespace remote
	{
		std::expected<lunas::syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
								  const std::unique_ptr<lunas::sftp>& src_sftp,
								  const std::unique_ptr<lunas::sftp>& dest_sftp,
								  const lunas::syncmisc&	      misc)
		{
			assert(src_sftp != nullptr || dest_sftp != nullptr);

			lunas::print_sync(src, dest, misc);

			std::expected<struct syncstat, lunas::error> syncstat = std::unexpected(lunas::error());

			int					     retries  = misc.options.disconnected_server_retries;

			do
			{
				if (src_sftp != nullptr && dest_sftp == nullptr)
				{
					syncstat = lunas::remote_to_local::copy(src, dest, src_sftp, misc);
				}
				else if (src_sftp == nullptr && dest_sftp != nullptr)
				{
					syncstat = lunas::local_to_remote::copy(src, dest, dest_sftp, misc);
				}
				else
				{
					syncstat = lunas::remote_to_remote::copy(src, dest, src_sftp, dest_sftp, misc);
				}

				if (retries > 0 && not syncstat && server_maybe_disconnected(syncstat.error().value()))
				{
					retries--;
					std::this_thread::sleep_for(std::chrono::seconds(misc.options.disconnected_server_timeout));

					lunas::printerr("server '{}' maybe disconnected, retring in {} seconds. re-tries left: {}",
							(dest_sftp != nullptr ? dest_sftp->get_hostname() : src_sftp->get_hostname()),
							misc.options.disconnected_server_timeout, retries);
				}
				else
				{
					break;
				}
			} while (not syncstat);

			if (not syncstat)
			{
				return std::unexpected(syncstat.error());
			}

			if (misc.options.dry_run == false && syncstat.value().code == lunas::sync_code::success)
			{
				auto ok = lunas::utimes::remote(src, dest, src_sftp, dest_sftp, misc);
				if (not ok)
				{
					lunas::warnln("{}", ok.error().message());
				}
			}

			return syncstat;
		}
	}
}

#endif // REMOTE_ENABLED
