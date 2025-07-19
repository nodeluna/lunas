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
#	include <filesystem>
#endif

export module lunas.sync;
import :types;
import :copy;
import :checks;
import :updating;
import :remove;

export import lunas.error;
export import lunas.ipath;
export import lunas.content;
import lunas.file_table;
import lunas.file_types;
import lunas.file;
import lunas.filter;
import lunas.hooks;

import lunas.stdout;

export namespace lunas
{
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data);
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data, lunas::content& content);
}

namespace lunas
{
	std::expected<std::vector<lunas::prehook>, lunas::error> setup_prehooks(const struct lunas::parsed_data& data)
	{
		std::vector<lunas::prehook> prehooks;
		if (not data.options.prehooks.empty())
		{
			for (const auto& prehook : data.options.prehooks)
			{
				prehooks.push_back({});
				auto ok = prehooks.back().parse(prehook);
				if (not ok)
				{
					lunas::printerr("[prehook-parsing-error]");
					return std::unexpected(ok.error());
				}
			}
		}

		return prehooks;
	}

	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data)
	{
		const auto& ipaths = data.get_ipaths();

		if (ipaths.empty())
		{
			return std::unexpected(lunas::error("didn't find any input directories"));
		}

		size_t src_index = 0;
		for (const auto& path : ipaths)
		{
			if (path.is_src())
			{
				break;
			}
			src_index++;
		}
		if (not ipaths.at(src_index).is_src())
		{
			return std::unexpected(lunas::error("didn't find any source in the input directories"));
		}

		struct directory_options directory_options = {
		    .follow_symlink    = data.options.follow_symlink,
		    .no_broken_symlink = data.options.no_broken_symlink,
		    .recursive	       = data.options.recursive,
		};

		auto directory = lunas::opendir(ipaths.at(src_index).sftp, ipaths.at(src_index).path, directory_options);
		if (not directory)
		{
			return std::unexpected(directory.error());
		}

		std::vector<lunas::prehook> prehooks;
		{
			auto ok = setup_prehooks(data);
			if (not ok)
			{
				return std::unexpected(ok.error());
			}
			prehooks = std::move(ok.value());
		}

		lunas::println(data.options.quiet, "--> opened source directory '{}'", ipaths.at(src_index).path);
		lunas::println(data.options.quiet, "");

		struct progress_stats			    progress_stats;
		std::expected<std::monostate, lunas::error> ok;

		while (auto src_file = directory.value()->read())
		{
			for (size_t dest_index = 0; dest_index < ipaths.size(); dest_index++)
			{
				if (src_index == dest_index || not ipaths.at(dest_index).is_dest())
				{
					continue;
				}

				if (auto ok = src_file.value().holds_attributes(); not ok)
				{
					lunas::printerr("{}", ok.error().message());
					continue;
				}

				std::filesystem::path dest_path;
				{
					std::string relative = src_file.value().path;
					relative	     = relative.substr(ipaths.at(src_index).path.size(), relative.size());
					if (directory.value()->filter_out(relative, data.options))
					{
						continue;
					}
					dest_path = ipaths.at(dest_index).path + relative;
				}

				struct metadata src_metadata = {
				    .mtime     = src_file->mtime.value(),
				    .file_type = src_file->file_type.value(),
				};
				struct metadata dest_metadata;

				auto dest_file = lunas::get_attributes(ipaths.at(dest_index).sftp, dest_path, data.options.follow_symlink);
				if (not dest_file && dest_file.error().value() != lunas::error_type::no_such_file)
				{
					lunas::warnln("{}", dest_file.error().message());
					continue;
				}
				else if (dest_file)
				{
					dest_metadata.mtime	= dest_file.value()->mtime();
					dest_metadata.file_type = dest_file.value()->file_type();
				}
				progress_stats.total_to_be_synced = progress_stats.total_synced;

				const struct file_metadata<std::filesystem::path> src_metadata_wrapper =
				    file_metadata(src_file.value().path, src_metadata, src_index, src_file->file_size);

				const struct file_metadata<std::filesystem::path> dest_metadata_wrapper =
				    file_metadata(dest_path, dest_metadata, dest_index);

				if (auto ok = check_dest(src_metadata_wrapper, dest_metadata_wrapper, data); not ok)
				{
					if (ok.error().value() != lunas::error_type::dest_check_skip_sync)
					{
						lunas::printerr("{}", ok.error().message());
					}
					continue;
				}

				if (not prehooks.empty())
				{
					auto prehook_action = prehook::pipe_hook(prehooks, src_file.value(), data.options);
					if (not prehook_action)
					{
						return std::unexpected(prehook_action.error());
					}
					else if (prehook_action.value() == hook_action::dont_sync)
					{
						continue;
					}
				}

				ok = check_mtime_and_sync(src_metadata_wrapper, dest_metadata_wrapper, data, progress_stats);
				if (not ok)
				{
					lunas::printerr("{}", ok.error().message());
				}
			}
		}

		if (not directory.value()->eof())
		{
			lunas::println(false, "didn't reach eof '{}'", ipaths.at(src_index).path);
		}

		if (data.options.remove_extra)
		{
			lunas::println(false, "");
			lunas::warn_ok("remove extra is running. checking for extra files to be removed...");

			for (size_t dest_index = 0; dest_index < ipaths.size(); dest_index++)
			{
				if (not ipaths.at(dest_index).is_dest_only())
				{
					continue;
				}

				auto ok = remove_extra(data, ipaths.at(dest_index).path, src_index, dest_index);
				if (not ok)
				{
					return std::unexpected(ok.error());
				}
			}
		}

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data, lunas::content& content)
	{

		struct progress_stats progress_stats;
		progress_stats.total_to_be_synced = content.to_be_synced;
		std::expected<std::monostate, lunas::error> synced;
		lunas::println(data.options.quiet, "");

		struct hooks hooks;
		{
			auto ok = setup_prehooks(data);
			if (not ok)
			{
				return std::unexpected(ok.error());
			}
			hooks.prehooks = std::move(ok.value());
		}


		for (auto file = content.files_table.begin(); file != content.files_table.end();)
		{
			auto src_index = get_src(*file, data);
			if (not src_index && data.options.remove_extra && src_index.error().value() == lunas::error_type::source_not_found)
			{
				goto retained_for_remove_extra;
			}
			else if (not src_index)
			{
				goto not_needed_in_the_files_table_any_longer;
			}

			synced = updating(*file, src_index.value(), data, progress_stats, hooks);
			if (not synced)
			{
				lunas::printerr("{}", synced.error().message());
			}

		not_needed_in_the_files_table_any_longer:
			file = content.files_table.erase(file);
			continue;

		retained_for_remove_extra:
			++file;
		}

		if (not data.options.remove_extra)
		{
			return std::monostate();
		}

		const auto& ipaths = data.get_ipaths();

		for (auto file = content.files_table.rbegin(); file != content.files_table.rend() && data.options.remove_extra; ++file)
		{
			auto src_index = get_src(*file, data);
			if (not src_index && src_index.error().value() == lunas::error_type::source_not_found)
			{
				size_t dest_index = 0;
				for (auto& metadata : file->metadatas)
				{
					if (metadata.file_type != lunas::file_types::not_found)
					{
						std::string to_be_removed = ipaths.at(dest_index).path + file->path;
						lunas::print_remove_extra(to_be_removed);
						auto file_size = lunas::get_size_and_remove(ipaths.at(dest_index).sftp, to_be_removed,
											    metadata.file_type, data.options.dry_run);
						if (not file_size)
						{
							lunas::printerr("{}", file_size.error().message());
						}
						else
						{
							lunas::register_remove(file_size.value(), metadata.file_type,
									       data.get_ipath(dest_index));
						}
					}
					dest_index++;
				}
			}
		}

		return std::monostate();
	}
}
