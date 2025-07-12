module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <expected>
#	include <string>
#	include <memory>
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
#ifdef REMOTE_ENABLED
		if (src_sftp != nullptr || dest_sftp != nullptr)
		{
			return remote::copy(src, dest, src_sftp, dest_sftp, misc);
		}
		else
#endif
			return local::copy(src, dest, misc);
	}
}
