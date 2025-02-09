module;

#include <set>
#include <expected>
#include <variant>
#include <vector>
#include <ctime>
#include <string>

export module lunas.sync:multi_source;
export import :types;
export import :copy;

export import lunas.error;
export import lunas.ipath;
export import lunas.file_table;
export import lunas.file_types;
export import lunas.stdout;

export namespace lunas {
	namespace multi_source {
		std::expected<size_t, lunas::error> get_src(const lunas::file_table& file_table, const struct lunas::parsed_data& data) {
			time_t src_mtime	   = 0;
			size_t src_index	   = 0;
			size_t potential_src_index = 0;

			const auto& ipaths = data.get_ipaths();

			bool first = true;
			for (const auto& potential_src_metadata : file_table.metadatas) {
				if (not ipaths.at(potential_src_index).is_src() ||
				    potential_src_metadata.file_type == lunas::file_types::not_found)
					goto end;
				if (first) {
					first = false;
					goto assign;
				} else if (not data.options.rollback && src_mtime > potential_src_metadata.mtime)
					goto end;
				else if (data.options.rollback && src_mtime < potential_src_metadata.mtime)
					goto end;
				else if (src_mtime == potential_src_metadata.mtime) {
					if (ipaths.at(src_index).is_remote() && not ipaths.at(potential_src_index).is_remote())
						goto assign;
					else
						goto end;
				}

			assign:
				src_mtime = potential_src_metadata.mtime;
				src_index = potential_src_index;
			end:
				potential_src_index++;
			}

			if (file_table.metadatas.at(src_index).file_type == lunas::file_types::not_found)
				return std::unexpected(lunas::error(lunas::error_type::source_not_found));
			else if (not ipaths.at(src_index).is_src())
				return std::unexpected(lunas::error(lunas::error_type::source_not_found));
			else if (data.options.no_broken_symlink &&
				 file_table.metadatas.at(src_index).file_type == lunas::file_types::brokenlink)
				return std::unexpected(lunas::error(lunas::error_type::source_broken_symlink));

			return src_index;
		}

		std::expected<std::monostate, enum lunas::error_type> sync_dest(const lunas::metadata& src_metadata,
		    const lunas::metadata& dest_metadata, const size_t src_index, const size_t dest_index,
		    const struct lunas::parsed_data& data) {
			if (src_index == dest_index)
				return std::unexpected(lunas::error_type::dest_check_same_input_path);
			else if (not data.get_ipaths().at(dest_index).is_dest())
				return std::unexpected(lunas::error_type::dest_check_not_dest);
			else if (dest_metadata.file_type == lunas::file_types::directory)
				return std::unexpected(lunas::error_type::dest_check_existing_directory);
			else if (dest_metadata.mtime == src_metadata.mtime)
				return std::unexpected(lunas::error_type::dest_check_same_mtime);
			else if (dest_metadata.file_type == lunas::file_types::brokenlink)
				return std::unexpected(lunas::error_type::dest_check_brokenlink);
			else if (dest_metadata.file_type != lunas::file_types::not_found &&
				 dest_metadata.file_type != src_metadata.file_type)
				return std::unexpected(lunas::error_type::dest_check_type_conflict);

			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> updating(const lunas::file_table& file_table, const size_t src_index,
		    struct lunas::parsed_data& data, struct progress_stats& progress_stats) {

			time_t	    src_mtime  = file_table.metadatas.at(src_index).mtime;
			const auto& ipaths     = data.get_ipaths();
			size_t	    dest_index = 0;
			for (const auto& dest_metadata : file_table.metadatas) {
				bool sync = false;

				if (auto ok = sync_dest(file_table.metadatas.at(src_index), dest_metadata, src_index, dest_index, data);
				    not ok) {
					if (ok.error() == lunas::error_type::dest_check_type_conflict)
						lunas::printerr("conflict in types between '{}' and '{}'",
						    ipaths.at(src_index).path + file_table.path,
						    ipaths.at(dest_index).path + file_table.path);
					goto end;
				}

				if (data.options.update && src_mtime > dest_metadata.mtime)
					sync = true;
				else if (data.options.rollback && src_mtime < dest_metadata.mtime)
					sync = true;
				else if (dest_metadata.file_type == lunas::file_types::not_found)
					sync = true;

				if (sync) {
					struct syncmisc misc = {
					    .src_mtime	    = src_mtime,
					    .file_type	    = file_table.metadatas.at(src_index).file_type,
					    .options	    = data.options,
					    .progress_stats = progress_stats,
					};
					std::string src	 = ipaths.at(src_index).path + file_table.path;
					std::string dest = ipaths.at(dest_index).path + file_table.path;
#ifdef REMOTE_ENABLED
					auto syncstat = lunas::copy(src, dest, ipaths.at(src_index).sftp, ipaths.at(dest_index).sftp, misc);
#else
					auto syncstat = lunas::copy(src, dest, misc);
#endif // REMOTE_ENABLED

					if (not syncstat)
						lunas::printerr("{}", syncstat.error().message());
					else
						lunas::register_synced_stats(
						    syncstat.value(), misc.file_type, data.get_ipath(dest_index), progress_stats);
				}

			end:
				dest_index++;
			}

			return std::monostate();
		}
	}
}
