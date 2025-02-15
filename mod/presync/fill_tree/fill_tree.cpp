module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <filesystem>
#	include <memory>
#	include <exception>
#	include <string>
#	include <set>
#	include <expected>
#	include <variant>
#endif

export module lunas.presync.fill_tree;
export import :types;
export import lunas.error;
export import lunas.file;
export import lunas.file_table;
import lunas.exclude;
import lunas.stdout;

export namespace lunas {
	namespace presync {
		std::expected<std::monostate, lunas::error> readdir(
		    std::set<lunas::file_table>& content, const std::string& path, const lunas::fill_tree_type& data);

		std::expected<std::monostate, lunas::error> input_directory_check(const lunas::fill_tree_type& data);
	}
}

namespace lunas {
	namespace content {
		void insert(std::set<lunas::file_table>& content, const struct lunas::metadata& metadata, const std::string& path,
		    const lunas::fill_tree_type& data) {
			auto itr = content.find(lunas::file_table(path, metadata, data.path_index, data.ipaths_count));
			if (itr != content.end())
				itr->metadatas.at(data.path_index) = metadata;
			else
				content.insert(lunas::file_table(path, metadata, data.path_index, data.ipaths_count));
		}

	}

	namespace presync {
		std::expected<std::monostate, lunas::error> readdir(
		    std::set<lunas::file_table>& content, const std::string& path, const lunas::fill_tree_type& data) {

			struct directory_options directory_options = {
			    .follow_symlink    = data.options->follow_symlink,
			    .no_broken_symlink = data.options->no_broken_symlink,
			};

			auto directory = lunas::opendir(data.ipath->sftp, path, directory_options);
			if (not directory)
				return std::unexpected(directory.error());

			std::expected<std::monostate, lunas::error> ok;

			while (auto src_file = directory.value()->read()) {
				if (auto ok = src_file.value().holds_attributes(); not ok)
					return std::unexpected(ok.error());

				std::string relative_path = src_file.value().path;
				relative_path		  = relative_path.substr(path.size(), relative_path.size());
				if (lunas::exclude(relative_path, data.options->exclude, data.options->exclude_pattern))
					continue;

				struct metadata metadata = {
				    .mtime     = std::get<time_t>(src_file->mtime),
				    .file_type = std::get<lunas::file_types>(src_file->file_type),
				};

				lunas::content::insert(content, metadata, relative_path, data);
			}

			if (not directory.value()->eof())
				return std::unexpected(lunas::error("", lunas::error_type::readdir_eof));

			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> input_directory_check(const lunas::fill_tree_type& data) {
			if (not data.ipath->path.empty() && data.ipath->path.back() != std::filesystem::path::preferred_separator)
				return std::unexpected(lunas::error(
				    "trailing slash missing from input directory '" + data.ipath->path + "'", lunas::error_type::ipath));

			auto type = lunas::get_attributes(data.ipath->sftp, data.ipath->path, data.options->follow_symlink);
			if (not type && type.error().value() == lunas::error_type::no_such_file) {
				if (not data.options->mkdir) {
					std::string err =
					    "input directory '" + data.ipath->path + "', doesn't exist. use -mkdir option to create it";
					return std::unexpected(lunas::error(err, lunas::error_type::input_directory_check));
				} else {
					auto ok = lunas::file_operations::mkdir(data.ipath->sftp, data.ipath->path, data.options->dry_run);
					if (not ok)
						return std::unexpected(ok.error());

					lunas::warn_ok("created input directory '{}', it was not found", data.ipath->path);
				}
			} else if (not type) {
				return std::unexpected(type.error());
			} else if (type.value()->file_type() != lunas::file_types::directory) {
				std::string err = "input path '" + data.ipath->path + "', isn't a directory";
				return std::unexpected(lunas::error(err, lunas::error_type::input_directory_check));
			}

			return std::monostate();
		}

	}
}
