module;

#include <filesystem>
#include <memory>
#include <cerrno>
#include <cstdlib>
#include <exception>
#include <string>
#include <set>
#include <expected>
#include <variant>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <libssh/sftp.h>

export module lunas.presync.fill_tree:remote;
export import :types;
export import lunas.error;

import lunas.sftp;
import lunas.file_types;
import lunas.file_table;
import lunas.path;
import lunas.error;
import lunas.stdout;
import lunas.exclude;

export namespace lunas {
	namespace presync {
		namespace remote {
			std::expected<std::monostate, lunas::error> readdir(
			    std::set<lunas::file_table>& content, const std::string& path, const lunas::fill_tree_type& data);

			std::expected<std::monostate, lunas::error> input_directory_check(const lunas::fill_tree_type& data);

		}
	}
}

namespace lunas {
	namespace presync {
		namespace remote {
			namespace readdir_operations {
				std::expected<lunas::file_types, lunas::error> type(
				    std::unique_ptr<lunas::attributes>& file, const std::string& path, const lunas::fill_tree_type& data);

				std::expected<time_t, lunas::error> mtime(
				    const struct metadata& metadata, const std::string& path, const lunas::fill_tree_type& data);

				void insert(std::set<lunas::file_table>& content, const struct lunas::metadata& metadata,
				    const std::string& path, const lunas::fill_tree_type& data);
			}

			std::expected<std::monostate, lunas::error> readdir(
			    std::set<lunas::file_table>& content, const std::string& path, const lunas::fill_tree_type& data) {

				auto dir = data.ipath->sftp->opendir(path);
				if (not dir)
					return std::unexpected(dir.error());

				while (auto file = dir.value()->read(data.ipath->sftp->get_sftp_session())) {
					std::string full_path;
					{
						{
							std::string file_name = file.value()->name();
							full_path	      = path + file_name;
							if (file_name == ".." || file_name == ".")
								continue;
						}
						std::string relative_path = full_path.substr(data.ipath->path.size(), full_path.size());
						if (lunas::exclude(relative_path, data.options->exclude, data.options->exclude_pattern))
							continue;

						struct metadata metadata;
						{
							auto type = readdir_operations::type(file.value(), full_path, data);
							if (not type)
								return std::unexpected(type.error());
							metadata.file_type = type.value();
						}

						{
							metadata.mtime = file.value()->mtime();
							auto mtime     = readdir_operations::mtime(metadata, full_path, data);
							if (not mtime)
								return std::unexpected(mtime.error());
							metadata.mtime = mtime.value();
						}

						readdir_operations::insert(content, metadata, relative_path, data);
					}

					if (file.value()->file_type() == lunas::file_types::directory) {
						file.value().reset();
						lunas::path::append_seperator(full_path);
						readdir(content, full_path, data);
					}
				}

				if (auto eof = dir.value()->eof(data.ipath->sftp->get_sftp_session(), path); not eof)
					return std::unexpected(eof.error());

				return std::monostate();
			}

			std::expected<std::monostate, lunas::error> input_directory_check(const lunas::fill_tree_type& data) {
				if (not data.ipath->path.empty() && data.ipath->path.back() != std::filesystem::path::preferred_separator)
					return std::unexpected(
					    lunas::error("trailing slash missing from input directory '" + data.ipath->path + "'",
						lunas::error_type::ipath));

				auto type = data.ipath->sftp->attributes(data.ipath->path, data.options->follow_symlink);
				if (not type && type.error().value() == lunas::error_type::sftp_no_such_file) {
					if (not data.options->mkdir) {
						std::string err =
						    "input directory '" + data.ipath->path + "', doesn't exist. use -mkdir to create it";
						return std::unexpected(lunas::error(err, lunas::error_type::input_directory_check));
					} else if (data.options->mkdir) {
						auto ok = data.ipath->sftp->mkdir(data.ipath->path);
						if (not ok)
							return std::unexpected(ok.error());

						lunas::warn("created input directory '{}', it was not found", data.ipath->path);
						return std::monostate();
					}
				} else if (not type) {
					return std::unexpected(type.error());
				} else if (type.value()->file_type() != lunas::file_types::directory) {
					std::string err = "input path '" + data.ipath->path + "' isn't a directory";
					return std::unexpected(lunas::error(err, lunas::error_type::input_directory_check));
				}

				return std::monostate();
			}

			namespace readdir_operations {
				std::expected<lunas::file_types, lunas::error> type(
				    std::unique_ptr<lunas::attributes>& file, const std::string& path, const lunas::fill_tree_type& data) {
					if (file->file_type() == lunas::file_types::symlink) {
						if (data.options->no_broken_symlink && data.ipath->sftp->is_broken_link(path)) {
							return lunas::file_types::brokenlink;
						} else if (data.options->follow_symlink == lunas::follow_symlink::yes) {
							auto attr = data.ipath->sftp->attributes(path, lunas::follow_symlink::yes);
							if (not attr)
								return std::unexpected(attr.error());
							return attr.value()->file_type();
						}
					}

					if (file->file_type() == lunas::file_types::regular_file && lunas::presync::is_lspart(path))
						return lunas::file_types::resume_regular_file;

					return file->file_type();
				}

				std::expected<time_t, lunas::error> mtime(
				    const struct metadata& metadata, const std::string& path, const lunas::fill_tree_type& data) {
					if (metadata.file_type != lunas::file_types::brokenlink &&
					    data.options->follow_symlink == lunas::follow_symlink::yes) {
						auto ok = data.ipath->sftp->get_utimes(
						    path, lunas::sftp::time_type::mtime, data.options->follow_symlink);
						if (not ok)
							return std::unexpected(ok.error());
						return ok.value().mtime;
					} else
						return metadata.mtime;
				}

				void insert(std::set<lunas::file_table>& content, const struct lunas::metadata& metadata,
				    const std::string& path, const lunas::fill_tree_type& data) {
					auto itr = content.find(lunas::file_table(path, metadata, data.path_index, data.ipaths_count));
					if (itr != content.end())
						itr->metadatas.at(data.path_index) = metadata;
					else
						content.insert(lunas::file_table(path, metadata, data.path_index, data.ipaths_count));
				}
			}
		}
	}
}
