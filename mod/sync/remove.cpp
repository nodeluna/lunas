module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <filesystem>
#	include <string>
#	include <expected>
#	include <variant>
#	include <memory>
#endif

export module lunas.sync:remove;
export import :misc;
export import lunas.sftp;
export import lunas.file_types;
export import lunas.error;
import lunas.cppfs;

export namespace lunas {
	std::expected<std::monostate, lunas::error> remove(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::string& path, lunas::file_types file_type, bool dry_run);

	std::expected<std::uintmax_t, lunas::error> get_size_and_remove(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::string& path, lunas::file_types file_type, bool dry_run);

	void register_remove(const std::uintmax_t& file_size, lunas::file_types file_type, lunas::ipath::input_path& ipath);
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
}
