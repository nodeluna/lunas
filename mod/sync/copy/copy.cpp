module;

#include <cassert>
#include <thread>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <expected>
#	include <string>
#	include <memory>
#	include <any>
#	include <cstddef>
#endif

export module lunas.sync:copy;
export import :types;
export import :remote_copy;
export import :local_copy;
import :remove;
import :stdout;
export import lunas.sftp;

import lunas.error;

export namespace lunas
{
#ifdef REMOTE_ENABLED
	std::expected<syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
						   const std::unique_ptr<lunas::sftp>& src_sftp,
						   const std::unique_ptr<lunas::sftp>& dest_sftp, const struct syncmisc& misc)
	{
#else
	std::expected<syncstat, lunas::error> copy(const std::string& src, const std::string& dest, const struct syncmisc& misc)
	{
#endif
		if (misc.options.remove_before && misc.is_dest_regular_file)
		{
			auto ok = remove(dest_sftp, dest, misc.file_type, misc.options.dry_run);
			if (not ok && ok.error().value() != lunas::error_type::no_such_file)
			{
				lunas::warnln("[remove-before] {}", ok.error().message());
			}
		}

		std::expected<syncstat, lunas::error> ok;
#ifdef REMOTE_ENABLED
		if (src_sftp != nullptr || dest_sftp != nullptr)
		{
			ok = remote::copy(src, dest, src_sftp, dest_sftp, misc);
		}
		else
		{
#endif
			ok = local::copy(src, dest, misc);
		}

		if (misc.options.remove_partials && misc.is_dest_regular_file && not ok && sync_interrupted_error(ok.error().value()))
		{
			assert(ok.error().get_user_data().type() == typeid(size_t));
			const size_t hash = std::any_cast<size_t>(ok.error().get_user_data());

			const auto   ok2  = remove(dest_sftp, make_dest_lspart(dest, hash), misc.file_type, misc.options.dry_run);
			if (not ok2 && ok2.error().value() != lunas::error_type::no_such_file)
			{
				lunas::warnln("[remove-partials] {}", ok2.error().message());
			}
		}

		return ok;
	}
}
