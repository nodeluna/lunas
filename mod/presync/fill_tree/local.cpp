module;

#include <filesystem>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <set>
#include <expected>
#include <variant>

export module lunas.presync.fill_tree:local;
export import :types;
export import lunas.error;

import lunas.path;
import lunas.attributes;
import lunas.file_table;
import lunas.file_types;
import lunas.cppfs;
import lunas.stdout;
import lunas.attributes;
import lunas.exclude;
import lunas.config.options;

namespace fs = std::filesystem;

export namespace lunas {
	namespace presync {
		namespace local {
			std::expected<std::monostate, lunas::error> readdir(
			    std::set<lunas::file_table>& content, const std::string& path, const lunas::fill_tree_type& data);

			std::expected<std::variant<std::string, std::monostate>, lunas::error> input_directory_check(
			    const lunas::fill_tree_type& data);
		}
	}
}

namespace lunas {
	namespace presync {
		namespace local {
			namespace readdir_operations {
				std::expected<lunas::file_types, lunas::error> type(
				    const fs::directory_entry& entry, const std::string& path, const lunas::config::options* options);

				std::expected<time_t, lunas::error> mtime(
				    const struct metadata& metadata, const std::string& path, const lunas::fill_tree_type& data);

				void insert(std::set<lunas::file_table>& content, const struct lunas::metadata& metadata,
				    const std::string& str_entry, const lunas::fill_tree_type& data);
			}

			std::expected<std::monostate, lunas::error> readdir(
			    std::set<lunas::file_table>& content, const std::string& path, const lunas::fill_tree_type& data) {
				try {
					for (const auto& entry : fs::directory_iterator(path)) {
						{
							std::string relative_path = entry.path().string().substr(
							    data.ipath->path.size(), entry.path().string().size());
							if (lunas::exclude(
								relative_path, data.options->exclude, data.options->exclude_pattern))
								continue;

							const std::string& str_entry = entry.path().string();

							struct lunas::metadata metadata;

							{
								auto ok = readdir_operations::type(entry, str_entry, data.options);
								if (not ok)
									return std::unexpected(ok.error());
								metadata.file_type = ok.value();
							}

							{
								auto mtime = readdir_operations::mtime(metadata, str_entry, data);
								if (not mtime)
									return std::unexpected(mtime.error());
								metadata.mtime = mtime.value();
							}

							readdir_operations::insert(content, metadata, relative_path, data);
						}

						if (entry.is_directory()) {
							auto ok = readdir(content, entry.path().string(), data);
							if (not ok)
								return std::unexpected(ok.error());
						}
					}
				} catch (const std::exception& e) {
					std::string err =
					    "some files/directories were missed '" + data.ipath->path + "', " + std::strerror(errno);
					return std::unexpected(lunas::error(err, lunas::error_type::local_readdir));
				}

				return std::monostate();
			}

			std::expected<std::variant<std::string, std::monostate>, lunas::error> input_directory_check(
			    const lunas::fill_tree_type& data) {
				if (not data.ipath->path.empty() && data.ipath->path.back() != std::filesystem::path::preferred_separator)
					return std::unexpected(
					    lunas::error("trailing slash missing from input directory '" + data.ipath->path + "'",
						lunas::error_type::ipath));

				auto type = lunas::get_file_type(data.ipath->path, lunas::follow_symlink::yes);
				if (not type && type.error().value() == lunas::error_type::attributes_no_such_file) {
					if (not data.options->mkdir) {
						std::string err = "input directory '" + data.ipath->path +
								  "', doesn't exist. use -mkdir option to create it";
						return std::unexpected(lunas::error(err, lunas::error_type::input_directory_check));
					} else {
						auto ok = lunas::cppfs::mkdir(data.ipath->path, data.options->dry_run);
						if (not ok)
							return std::unexpected(ok.error());

						lunas::warn("created input directory '{}', it was not found", data.ipath->path);

						if (data.options->dry_run)
							return std::monostate();
					}
				} else if (not type) {
					return std::unexpected(type.error());
				} else if (type.value() != lunas::file_types::directory) {
					std::string err = "input path '" + data.ipath->path + "', isn't a directory";
					return std::unexpected(lunas::error(err, lunas::error_type::input_directory_check));
				}

				auto ok = lunas::permissions::is_file_readable(data.ipath->path, lunas::follow_symlink::yes);
				if (not ok) {
					return std::unexpected(ok.error());
				} else if (ok.value() == false) {
					std::string err = "couldn't read input directory '" + data.ipath->path + "', permission denied";
					return std::unexpected(lunas::error(err, lunas::error_type::input_directory_check));
				}

				return std::monostate();
			}

			namespace readdir_operations {
				std::expected<lunas::file_types, lunas::error> type(
				    const fs::directory_entry& entry, const std::string& path, const lunas::config::options* options) {
					std::filesystem::file_status status;
					if (options->follow_symlink == lunas::follow_symlink::yes)
						status = entry.status();
					else
						status = entry.symlink_status();

					lunas::file_types file_type = lunas::get_file_type(status);

					if (options->no_broken_symlink && file_type == lunas::file_types::symlink) {
						std::expected<bool, lunas::error> broken = lunas::is_broken_link(path);
						if (not broken)
							return std::unexpected(broken.error());
						if (broken.value())
							return lunas::file_types::brokenlink;
					}

					if (file_type == lunas::file_types::regular_file && lunas::presync::is_lspart(path))
						file_type = lunas::file_types::resume_regular_file;

					return file_type;
				}

				std::expected<time_t, lunas::error> mtime(
				    const struct metadata& metadata, const std::string& path, const lunas::fill_tree_type& data) {
					if (metadata.file_type != lunas::file_types::brokenlink) {
						auto ok = lunas::utime::get(path, lunas::time_type::mtime, data.options->follow_symlink);
						if (not ok)
							return std::unexpected(ok.error());
						return ok.value().mtime;
					} else {
						auto ok = lunas::utime::get(path, lunas::time_type::mtime, lunas::follow_symlink::no);
						if (not ok)
							return std::unexpected(ok.error());
						return ok.value().mtime;
					}
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
