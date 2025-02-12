module;

#include <memory>
#include <expected>
#include <functional>
#include <filesystem>

export module lunas.sync:misc;
export import :types;
export import lunas.sftp;
export import lunas.error;
export import lunas.ipath;
export import lunas.cppfs;
export import lunas.stdout;

export namespace lunas {
	size_t get_src_hash(const std::string& src, const unsigned long int& src_mtime);

	std::string get_dest_hash(const std::string& dest, const size_t& src_mtimepath_hash);

	auto regular_file_sync(const std::string& src, const std::string& dest, const time_t& src_mtime,
	    std::function<std::expected<syncstat, lunas::error>(const std::string&)> func) -> std::expected<syncstat, lunas::error>;

	std::string dest_lspart(const std::string& dest, size_t src_quick_hash);

	std::expected<std::uintmax_t, lunas::error> file_size(const std::unique_ptr<lunas::sftp>& sftp, const std::string& path);

	namespace remote {
		struct original_name {
				original_name(const std::unique_ptr<lunas::sftp>& session, const std::string& dest_lspart,
				    const std::string& original_name, lunas::sync_code& code);
				~original_name();

			private:
				const std::unique_ptr<lunas::sftp>& sftp;
				const std::string&		    lspart;
				const std::string&		    dest;
				lunas::sync_code&		    synccode;
		};
	}

	namespace local {
		struct original_name {
				original_name(
				    const std::string& dest_lspart, const std::string& original_name, lunas::sync_code& code, bool dry_run);
				~original_name();

			private:
				const std::string& lspart;
				const std::string& dest;
				lunas::sync_code&  synccode;
				bool		   dry_run = false;
		};
	}

	void register_synced_stats(const struct syncstat& syncstat, lunas::file_types file_type, lunas::ipath::input_path& ipath,
	    struct progress_stats& progress_stats);
}

namespace lunas {
	size_t get_src_hash(const std::string& src, const unsigned long int& src_mtime) {
		return std::hash<std::string>{}(src + std::to_string(src_mtime));
	}

	std::string get_dest_hash(const std::string& dest, const size_t& src_mtimepath_hash) {
		int		  dot_counter = 0;
		unsigned long int i	      = dest.size() - 1;
		for (;; i--) {
			if (dot_counter == 3)
				break;
			if (dest.at(i) == '.')
				dot_counter++;
			if (i == 0)
				break;
		}

		return dest.substr(i + 2, std::to_string(src_mtimepath_hash).size());
	}

	std::string dest_lspart(const std::string& src, const std::string& dest, const time_t src_mtime) {
		size_t src_quick_hash = get_src_hash(src, src_mtime);
		return dest + "." + std::to_string(src_quick_hash) + ".ls.part";
	}

	auto regular_file_sync(const std::string& src, const std::string& dest, const time_t& src_mtime,
	    std::function<std::expected<syncstat, lunas::error>(const std::string&)> func) -> std::expected<syncstat, lunas::error> {
		std::string dest_lspart_path = dest_lspart(src, dest, src_mtime);
		return func(dest_lspart_path);
	}

	std::expected<std::uintmax_t, lunas::error> file_size(const std::unique_ptr<lunas::sftp>& sftp, const std::string& path) {
		if (sftp != nullptr)
			return sftp->file_size(path);
		else
			return lunas::cppfs::file_size(path);
	}

	namespace remote {
		original_name::original_name(const std::unique_ptr<lunas::sftp>& session, const std::string& dest_lspart,
		    const std::string& original_name, lunas::sync_code& code)
		    : sftp(session), lspart(dest_lspart), dest(original_name), synccode(code) {
		}

		original_name::~original_name() {
			if (synccode != lunas::sync_code::success || dest == lspart)
				return;

			auto ok = sftp->rename(lspart, dest);
			if (not ok) {
				lunas::warn("couldn't rename '{}' to its original name", lspart);
				synccode = lunas::sync_code::post_sync_fail;
			}
		}
	}

	namespace local {
		original_name::original_name(
		    const std::string& dest_lspart, const std::string& original_name, lunas::sync_code& code, bool dry_run)
		    : lspart(dest_lspart), dest(original_name), synccode(code), dry_run(dry_run) {
		}

		original_name::~original_name() {
			if (dry_run)
				return;
			if (synccode != lunas::sync_code::success || dest == lspart)
				return;

			std::error_code ec;
			std::filesystem::rename(lspart, dest, ec);
			if (ec.value() != 0) {
				lunas::warn("couldn't rename '{}' to its original name", lspart);
				synccode = lunas::sync_code::post_sync_fail;
			}
		}
	}

	void register_synced_stats(const struct syncstat& syncstat, lunas::file_types file_type, lunas::ipath::input_path& ipath,
	    struct progress_stats& progress_stats) {
		if (syncstat.code != lunas::sync_code::success)
			return;

		progress_stats.total_synced += 1;

		if (file_type == lunas::file_types::directory) {
			ipath.increment_stats_synced_dirs();
		} else
			ipath.increment_stats_synced_files();

		ipath.increment_stats_synced_size(syncstat.copied_size);
	}
}
