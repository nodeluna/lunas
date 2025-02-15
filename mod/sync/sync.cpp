module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <set>
#	include <expected>
#	include <variant>
#	include <ctime>
#	include <string>
#	include <cstddef>
#	include <exception>
#endif

export module lunas.sync;
import :types;
import :copy;
import :multi_source;
import :remove;

export import lunas.error;
export import lunas.ipath;
export import lunas.content;
import lunas.file_table;
import lunas.file_types;
import lunas.file;
import lunas.exclude;

import lunas.stdout;

export namespace lunas {
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data);
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data, lunas::content& content);
}

export namespace lunas {
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data) {
		const auto& ipaths = data.get_ipaths();

		if (ipaths.empty())
			return std::unexpected(lunas::error("didn't find any input directories"));

		size_t src_index = 0;
		for (const auto& path : ipaths) {
			if (path.is_src())
				break;
			src_index++;
		}
		if (not ipaths.at(src_index).is_src())
			return std::unexpected(lunas::error("didn't find any source in the input directories"));

		struct directory_options directory_options = {
		    .follow_symlink    = data.options.follow_symlink,
		    .no_broken_symlink = data.options.no_broken_symlink,
		};

		auto directory = lunas::opendir(ipaths.at(src_index).sftp, ipaths.at(src_index).path, directory_options);
		if (not directory)
			return std::unexpected(directory.error());

		lunas::println(data.options.quiet, "--> opened source directory '{}'", ipaths.at(src_index).path);
		lunas::println(data.options.quiet, "");

		struct progress_stats			    progress_stats;
		std::expected<std::monostate, lunas::error> ok;

		while (auto src_file = directory.value()->read()) {
			for (size_t dest_index = 0; dest_index < ipaths.size(); dest_index++) {
				if (src_index == dest_index || not ipaths.at(dest_index).is_dest())
					continue;

				if (auto ok = src_file.value().holds_attributes(); not ok) {
					lunas::printerr("{}", ok.error().message());
					continue;
				}

				std::string dest_path;
				{
					std::string relative = src_file.value().path;
					relative	     = relative.substr(ipaths.at(src_index).path.size(), relative.size());
					dest_path	     = ipaths.at(dest_index).path + relative;
					if (lunas::exclude(relative, data.options.exclude, data.options.exclude_pattern))
						continue;
				}

				struct metadata src_metadata = {
				    .mtime     = std::get<time_t>(src_file->mtime),
				    .file_type = std::get<lunas::file_types>(src_file->file_type),
				};
				struct metadata dest_metadata;

				auto dest_file = lunas::get_attributes(ipaths.at(dest_index).sftp, dest_path, data.options.follow_symlink);
				if (not dest_file && dest_file.error().value() != lunas::error_type::no_such_file) {
					lunas::warn("{}", dest_file.error().message());
					continue;
				} else if (dest_file) {
					dest_metadata.mtime	= dest_file.value()->mtime();
					dest_metadata.file_type = dest_file.value()->file_type();
				}
				progress_stats.total_to_be_synced = progress_stats.total_synced;

				ok = multi_source::check_dest_and_sync(
				    src_metadata, dest_metadata, src_index, dest_index, src_file->path, dest_path, data, progress_stats);
				if (not ok) {
					if (ok.error().value() == lunas::error_type::dest_check_type_conflict)
						lunas::printerr("conflict in types between '{}' and '{}'", src_file->path, dest_path);
					else if (ok.error().value() != lunas::error_type::dest_check_skip_sync)
						lunas::printerr("{}", ok.error().message());
				}
			}
		}

		if (not directory.value()->eof())
			lunas::println(false, "didn't reach eof '{}'", ipaths.at(src_index).path);

		if (data.options.remove_extra) {
			directory_options.follow_symlink = lunas::follow_symlink::no;
			lunas::println(false, "");
			lunas::warn_ok("remove extra is running. checking for extra files to be removed...");

			for (size_t dest_index = 0; dest_index < ipaths.size(); dest_index++) {
				if (not ipaths.at(dest_index).is_dest_only())
					continue;

				directory = lunas::opendir(ipaths.at(dest_index).sftp, ipaths.at(dest_index).path, directory_options);
				if (not directory)
					return std::unexpected(directory.error());

				while (auto dest_file = directory.value()->read()) {
					if (auto ok = dest_file.value().holds_attributes(); not ok) {
						lunas::printerr("{}", ok.error().message());
						continue;
					}

					std::string src_path;
					{
						std::string relative = dest_file.value().path;
						relative	     = relative.substr(ipaths.at(dest_index).path.size(), relative.size());
						src_path	     = ipaths.at(src_index).path + relative;
						if (lunas::exclude(relative, data.options.exclude, data.options.exclude_pattern))
							continue;
					}

					auto src_file =
					    lunas::get_attributes(ipaths.at(src_index).sftp, src_path, lunas::follow_symlink::no);
					if (src_file) {
						continue;
					} else if (not src_file && src_file.error().value() != lunas::error_type::no_such_file) {
						lunas::warn("{}", src_file.error().message());
						continue;
					} else if (not src_file && src_file.error().value() == lunas::error_type::no_such_file) {
						lunas::print_remove_extra(dest_file.value().path);
						auto file_size =
						    lunas::get_size_and_remove(ipaths.at(dest_index).sftp, dest_file.value().path,
							std::get<lunas::file_types>(dest_file->file_type), data.options.dry_run);
						if (not file_size)
							lunas::printerr("{}", file_size.error().message());
						else
							lunas::register_remove(file_size.value(),
							    std::get<lunas::file_types>(dest_file->file_type), data.get_ipath(dest_index));
					}
				}

				if (not directory.value()->eof())
					lunas::println(false, "didn't reach eof of directory '{}'", ipaths.at(dest_index).path);
			}
		}

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data, lunas::content& content) {

		struct progress_stats progress_stats;
		progress_stats.total_to_be_synced = content.to_be_synced;
		std::expected<std::monostate, lunas::error> synced;
		lunas::println(data.options.quiet, "");

		for (auto file = content.files_table.begin(); file != content.files_table.end();) {
			auto src_index = multi_source::get_src(*file, data);
			if (not src_index && data.options.remove_extra &&
			    src_index.error().value() == lunas::error_type::source_not_found) {
				goto retained_for_remove_extra;
			} else if (not src_index)
				goto not_needed_in_the_files_table_any_longer;

			synced = multi_source::updating(*file, src_index.value(), data, progress_stats);
			if (not synced)
				lunas::printerr("{}", synced.error().message());

		not_needed_in_the_files_table_any_longer:
			file = content.files_table.erase(file);
			continue;

		retained_for_remove_extra:
			++file;
		}

		if (not data.options.remove_extra)
			return std::monostate();

		const auto& ipaths = data.get_ipaths();

		for (auto file = content.files_table.rbegin(); file != content.files_table.rend() && data.options.remove_extra; ++file) {
			auto src_index = multi_source::get_src(*file, data);
			if (not src_index && src_index.error().value() == lunas::error_type::source_not_found) {
				size_t dest_index = 0;
				for (auto& metadata : file->metadatas) {
					if (metadata.file_type != lunas::file_types::not_found) {
						if (lunas::exclude(file->path, data.options.exclude, data.options.exclude_pattern))
							continue;
						std::string to_be_removed = ipaths.at(dest_index).path + file->path;
						lunas::print_remove_extra(to_be_removed);
						auto file_size = lunas::get_size_and_remove(
						    ipaths.at(dest_index).sftp, to_be_removed, metadata.file_type, data.options.dry_run);
						if (not file_size)
							lunas::printerr("{}", file_size.error().message());
						else
							lunas::register_remove(
							    file_size.value(), metadata.file_type, data.get_ipath(dest_index));
					}
					dest_index++;
				}
			}
		}

		return std::monostate();
	}
}
