module;

#include <cassert>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <set>
#	include <expected>
#	include <variant>
#	include <vector>
#	include <ctime>
#	include <string>
#endif

export module lunas.sync:checks;
export import :types;
export import :copy;

export import lunas.error;
export import lunas.ipath;
export import lunas.file_table;
export import lunas.file_types;
export import lunas.stdout;
export import lunas.file;
import lunas.stats;

export namespace lunas {
	std::expected<size_t, lunas::error> get_src(const lunas::file_table& file_table, const struct lunas::parsed_data& data)
	{
		time_t src_mtime	   = 0;
		size_t src_index	   = 0;
		size_t potential_src_index = 0;

		const auto& ipaths = data.get_ipaths();

		bool first = true;
		for (const auto& potential_src_metadata : file_table.metadatas)
		{
			if (not ipaths.at(potential_src_index).is_src() || potential_src_metadata.file_type == lunas::file_types::not_found)
			{
				goto end;
			}
			if (first)
			{
				first = false;
				goto assign;
			}
			else if (not data.options.rollback && src_mtime > potential_src_metadata.mtime)
			{
				goto end;
			}
			else if (data.options.rollback && src_mtime < potential_src_metadata.mtime)
			{
				goto end;
			}
			else if (src_mtime == potential_src_metadata.mtime)
			{
				if (ipaths.at(src_index).is_remote() && not ipaths.at(potential_src_index).is_remote())
				{
					goto assign;
				}
				else
				{
					goto end;
				}
			}

		assign:
			src_mtime = potential_src_metadata.mtime;
			src_index = potential_src_index;
		end:
			potential_src_index++;
		}

		if (file_table.metadatas.at(src_index).file_type == lunas::file_types::not_found)
		{
			return std::unexpected(lunas::error(lunas::error_type::source_not_found));
		}
		else if (not ipaths.at(src_index).is_src())
		{
			return std::unexpected(lunas::error(lunas::error_type::source_not_found));
		}
		else if (data.options.no_broken_symlink && file_table.metadatas.at(src_index).file_type == lunas::file_types::brokenlink)
		{
			return std::unexpected(lunas::error(lunas::error_type::source_broken_symlink));
		}

		return src_index;
	}

	std::expected<std::monostate, enum lunas::error_type> check_dest(
	    const struct file_metadata& src, const struct file_metadata& dest, struct lunas::parsed_data& data)
	{
		if (src.index == dest.index)
		{
			return std::unexpected(lunas::error_type::dest_check_skip_sync);
		}
		else if (not data.get_ipaths().at(dest.index).is_dest())
		{
			return std::unexpected(lunas::error_type::dest_check_skip_sync);
		}
		else if (dest.metadata.file_type == lunas::file_types::directory)
		{
			return std::unexpected(lunas::error_type::dest_check_skip_sync);
		}
		else if (dest.metadata.mtime == src.metadata.mtime)
		{
			return std::unexpected(lunas::error_type::dest_check_skip_sync);
		}
		else if (dest.metadata.file_type == lunas::file_types::brokenlink)
		{
			return std::unexpected(lunas::error_type::dest_check_brokenlink);
		}
		else if (src.metadata.file_type == lunas::file_types::resume_regular_file)
		{
			return std::unexpected(lunas::error_type::dest_check_skip_sync);
		}
		else if (dest.metadata.file_type != lunas::file_types::not_found &&
			 dest.metadata.file_type != lunas::file_types::resume_regular_file &&
			 dest.metadata.file_type != src.metadata.file_type)
		{
			return std::unexpected(lunas::error_type::dest_check_type_conflict);
		}
		else if (data.options.minimum_space)
		{
			assert(src.file_size != std::nullopt);
			assert(data.options.minimum_space != std::nullopt);
			if (data.options.hardlink_regular_files && not data.get_ipath(src.index).is_remote() &&
			    not data.get_ipath(dest.index).is_remote())
			{
				return std::monostate();
			}

			auto partition = lunas::get_partition(data.get_ipath(dest.index).sftp, data.get_ipath(dest.index).path);
			if (not partition)
			{
				lunas::printerr("{}", partition.error().message());
				return std::unexpected(partition.error().value());
			}

			if ((partition.value()->available() - *src.file_size) < *data.options.minimum_space)
			{
				lunas::printerr("partition '{}' is getting full: {}. file size: {}. minimum-space: {}",
				    data.get_ipath(dest.index).path, size_units(partition.value()->available()), size_units(*src.file_size),
				    size_units(*data.options.minimum_space));
				return std::unexpected(lunas::error_type::partition_info);
			}
		}

		return std::monostate();
	}
}
