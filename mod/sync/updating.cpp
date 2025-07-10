module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <set>
#	include <expected>
#	include <variant>
#	include <vector>
#	include <ctime>
#	include <string>
#	include <cstdint>
#	include <optional>
#endif

export module lunas.sync:updating;
export import :types;
export import :checks;
export import :copy;

export import lunas.error;
export import lunas.ipath;
export import lunas.file_table;
export import lunas.file_types;
export import lunas.stdout;
export import lunas.file;

export namespace lunas
{
	std::expected<std::monostate, lunas::error> check_dest_and_sync(const struct file_metadata src, const struct file_metadata& dest,
									struct lunas::parsed_data& data,
									struct progress_stats&	   progress_stats)
	{

		const auto& ipaths = data.get_ipaths();

		bool	    sync   = false;

		if (auto ok = check_dest(src, dest, data); not ok)
		{
			return std::unexpected(ok.error());
		}

		if (data.options.update && src.metadata.mtime > dest.metadata.mtime)
		{
			sync = true;
		}
		else if (data.options.rollback && src.metadata.mtime < dest.metadata.mtime)
		{
			sync = true;
		}
		else if (dest.metadata.file_type == lunas::file_types::not_found ||
			 dest.metadata.file_type == lunas::file_types::resume_regular_file)
		{
			sync = true;
		}

		if (sync)
		{
			struct syncmisc misc = {
			    .src_mtime	    = src.metadata.mtime,
			    .file_type	    = dest.metadata.file_type == lunas::file_types::resume_regular_file ? dest.metadata.file_type
														: src.metadata.file_type,
			    .options	    = data.options,
			    .progress_stats = progress_stats,
			};
#ifdef REMOTE_ENABLED
			auto syncstat = lunas::copy(src.path, dest.path, ipaths.at(src.index).sftp, ipaths.at(dest.index).sftp, misc);
#else
			auto syncstat = lunas::copy(src.path, dest.path, misc);
#endif // REMOTE_ENABLED

			if (not syncstat)
			{
				return std::unexpected(syncstat.error());
			}
			else
			{
				lunas::register_synced_stats(syncstat.value(), misc.file_type, data.get_ipath(dest.index), progress_stats);
			}
		}

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> updating(const lunas::file_table& file_table, const size_t src_index,
							     struct lunas::parsed_data& data, struct progress_stats& progress_stats)
	{

		const auto&		      src_metadata = file_table.metadatas.at(src_index);
		const auto&		      ipaths	   = data.get_ipaths();
		size_t			      dest_index   = 0;
		std::optional<std::uintmax_t> src_size	   = std::nullopt;
		const std::string	      src	   = ipaths.at(src_index).path + file_table.path;

		if (data.options.minimum_space && src_metadata.file_type == lunas::file_types::regular_file)
		{
			auto attr = lunas::get_attributes(ipaths.at(src_index).sftp, src, data.options.follow_symlink);
			if (not attr)
			{
				return std::unexpected(attr.error());
			}
			src_size = attr.value()->file_size();
		}

		for (const auto& dest_metadata : file_table.metadatas)
		{
			const std::string dest = ipaths.at(dest_index).path + file_table.path;

			auto		  ok   = check_dest_and_sync(file_metadata(src, src_metadata, src_index, src_size),
								     file_metadata(dest, dest_metadata, dest_index), data, progress_stats);
			if (not ok)
			{
				if (ok.error().value() != lunas::error_type::dest_check_skip_sync)
				{
					lunas::printerr("{}", ok.error().message());
				}
			}
			dest_index++;
		}

		return std::monostate();
	}
}
