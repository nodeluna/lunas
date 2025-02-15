module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <expected>
#	include <vector>
#	include <string>
#	include <print>
#	include <variant>
#	include <set>
#	include <cstdint>
#endif

export module lunas.presync:misc;

import lunas.error;
import lunas.stdout;
import lunas.ipath;
import lunas.sftp;
import lunas.file_table;
import lunas.file_types;

export namespace lunas {
	namespace presync {
		std::expected<std::monostate, lunas::error> input_paths_are_different(
		    const std::vector<struct lunas::ipath::input_path>& ipaths);
		size_t to_be_synced_counter(const std::set<file_table>& conent);
	}
}

namespace lunas {
	namespace presync {
		std::string fmt_err(const std::string& path1, const std::string& path2) {
			std::string err = "";
			lunas::fmterr_multiline(err, "input paths are the same");
			lunas::fmterr_multiline(err, "{}", path1);
			lunas::fmterr_multiline(err, "{}", path2);
			return err;
		}

		std::expected<std::monostate, lunas::error> input_paths_are_different(
		    const std::vector<struct lunas::ipath::input_path>& ipaths) {

			size_t out = 0, in = 0;
			for (const auto& path1 : ipaths) {
				in = 0;
				for (const auto& path2 : ipaths) {
					if (out == in)
						goto end;
					else if ((path1.is_remote() && not path2.is_remote()) ||
						 (not path1.is_remote() && path2.is_remote()))
						goto end;

					if (not path1.is_remote() && not path2.is_remote() && path1.path == path2.path) {
						std::string err = fmt_err(path1.path, path2.path);
						return std::unexpected(lunas::error(err, lunas::error_type::presync_same_input_path));
					}
#ifdef REMOTE_ENABLED
					else if (path1.is_remote() && path2.is_remote() && path1.path == path2.path) {
						const std::string hostname1 = path1.sftp->get_hostname();
						const std::string hostname2 = path2.sftp->get_hostname();
						if (hostname1 == hostname2) {
							std::string err = fmt_err(path1.sftp->get_ip(), path2.sftp->get_ip());
							return std::unexpected(
							    lunas::error(err, lunas::error_type::presync_same_input_path));
						}
					}
#endif // REMOTE_ENABLED
				end:
					in++;
				}
				out++;
			}

			return std::monostate();
		}

		size_t to_be_synced_counter(const std::set<file_table>& content) {
			size_t to_be_synced = 0;
			for (const auto& file : content) {
				time_t tmp		= 0;
				size_t files_dont_exist = 0;
				for (const auto& metadata : file.metadatas) {
					if (tmp == 0 && metadata.file_type != lunas::file_types::not_found) {
						tmp = metadata.mtime;
						continue;
					}

					if (metadata.file_type != lunas::file_types::not_found && metadata.mtime != tmp &&
					    metadata.file_type != lunas::file_types::directory) {
						to_be_synced += file.metadatas.size() - 1 - files_dont_exist;
						break;
					} else if (metadata.file_type == lunas::file_types::not_found) {
						to_be_synced++;
						files_dont_exist++;
					}
				}
			}

			return to_be_synced;
		}
	}
}
