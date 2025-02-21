module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <filesystem>
#	include <string>
#	include <expected>
#	include <variant>
#	include <memory>
#endif

export module lunas.sync:remove;
import :stdout;
export import :misc;
export import lunas.sftp;
export import lunas.file_types;
export import lunas.error;
import lunas.file;
import lunas.exclude;
import lunas.cppfs;

export namespace lunas {
	std::expected<std::monostate, lunas::error> remove(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::string& path, lunas::file_types file_type, bool dry_run);

	std::expected<std::uintmax_t, lunas::error> get_size_and_remove(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::string& path, lunas::file_types file_type, bool dry_run);

	void register_remove(const std::uintmax_t& file_size, lunas::file_types file_type, lunas::ipath::input_path& ipath);

	std::expected<std::monostate, lunas::error> remove_extra(
	    struct lunas::parsed_data& data, const std::string& path, const size_t& src_index, const size_t& dest_index);
}

namespace lunas {
	std::expected<std::monostate, lunas::error> remove(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::string& path, lunas::file_types file_type, bool dry_run) {

		if (sftp != nullptr) {
			if (file_type == lunas::file_types::directory) {
				auto ok = sftp->rmdir(path);
				if (not ok)
					return std::unexpected(ok.error());
			} else {
				auto ok = sftp->unlink(path);
				if (not ok)
					return std::unexpected(ok.error());
			}
		} else {
			auto ok = lunas::cppfs::remove(path, dry_run);
			if (not ok)
				return std::unexpected(ok.error());
		}

		return std::monostate();
	}

	std::expected<std::uintmax_t, lunas::error> get_size_and_remove(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::string& path, lunas::file_types file_type, bool dry_run) {
		if (file_type == lunas::file_types::regular_file || file_type == lunas::file_types::resume_regular_file) {
			auto file_size = lunas::file_size(sftp, path);

			auto ok = lunas::remove(sftp, path, file_type, dry_run);
			if (not ok)
				return std::unexpected(ok.error());
			else if (not file_size)
				return std::unexpected(file_size.error());
			else
				return file_size.value();
		}

		auto ok = lunas::remove(sftp, path, file_type, dry_run);
		if (not ok)
			return std::unexpected(ok.error());
		return 0;
	}

	void register_remove(const std::uintmax_t& file_size, lunas::file_types file_type, lunas::ipath::input_path& ipath) {
		if (file_type == lunas::file_types::directory)
			ipath.increment_stats_removed_dirs();
		else
			ipath.increment_stats_removed_files();

		ipath.increment_stats_removed_size(file_size);
	}

	std::expected<std::monostate, lunas::error> remove_extra(
	    struct lunas::parsed_data& data, const std::string& path, const size_t& src_index, const size_t& dest_index) {
		const auto&		 ipaths		   = data.get_ipaths();
		struct directory_options directory_options = {
		    .follow_symlink    = lunas::follow_symlink::no,
		    .no_broken_symlink = false,
		    .recursive	       = false,
		};

		auto directory = lunas::opendir(ipaths.at(dest_index).sftp, path, directory_options);
		if (not directory)
			return std::unexpected(directory.error());

		while (true) {
			auto dest_file = directory.value()->read();
			if (not dest_file && dest_file.error().value() == lunas::error_type::no_such_file) {
				continue;
			} else if (not dest_file) {
				if (dest_file.error().value() != lunas::error_type::readdir_eof)
					lunas::printerr("{}", dest_file.error().message());
				break;
			}

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

			if (std::get<lunas::file_types>(dest_file->file_type) == lunas::file_types::directory) {
				auto ok = remove_extra(data, dest_file.value().path, src_index, dest_index);
				if (not ok)
					return std::unexpected(ok.error());
			}

			auto src_file = lunas::get_attributes(ipaths.at(src_index).sftp, src_path, lunas::follow_symlink::no);
			if (src_file) {
				continue;
			} else if (not src_file && src_file.error().value() != lunas::error_type::no_such_file) {
				lunas::warn("{}", src_file.error().message());
				continue;
			} else if (not src_file && src_file.error().value() == lunas::error_type::no_such_file) {
				lunas::print_remove_extra(dest_file.value().path);
				auto file_size = lunas::get_size_and_remove(ipaths.at(dest_index).sftp, dest_file.value().path,
				    std::get<lunas::file_types>(dest_file->file_type), data.options.dry_run);
				if (not file_size)
					lunas::printerr("{}", file_size.error().message());
				else
					lunas::register_remove(file_size.value(), std::get<lunas::file_types>(dest_file->file_type),
					    data.get_ipath(dest_index));
			}
		}

		if (not directory.value()->eof())
			lunas::println(false, "didn't reach eof of directory '{}'", ipaths.at(dest_index).path);

		return std::monostate();
	}
}
