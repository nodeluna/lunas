#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <memory>
#	include <expected>
#	include <functional>
#	include <filesystem>
#	include <string>
#endif

#include "../types.hpp"
#include "sftp/sftp.hpp"
#include "error/error.hpp"
#include "input_path/input_path.hpp"
#include "cppfs/cppfs.hpp"
#include "stdout/stdout.hpp"

namespace lunas
{
	size_t	    get_src_hash(const std::string& src, const unsigned long int& src_mtime);

	std::string get_dest_hash(const std::string& dest, const size_t& src_mtimepath_hash);

	auto	    regular_file_sync(const std::string& src, const std::string& dest, const time_t& src_mtime,
				      std::function<std::expected<syncstat, lunas::error>(const std::pair<std::string, size_t>&)> func)
	    -> std::expected<syncstat, lunas::error>;

	std::string				    dest_lspart(const std::string& dest, size_t src_quick_hash);

	std::string				    make_dest_lspart(const std::string& dest, size_t src_quick_hash);

	std::expected<std::uintmax_t, lunas::error> file_size(const std::unique_ptr<lunas::sftp>& sftp, const std::string& path);

	namespace remote
	{
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

	namespace local
	{
		struct original_name {
				original_name(const std::string& dest_lspart, const std::string& original_name, lunas::sync_code& code,
					      bool dry_run);
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
