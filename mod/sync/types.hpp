#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <cstdint>
#	include <ctime>
#	include <vector>
#	include <memory>
#	include <expected>
#	include <string>
#	include <iostream>
#	include <iomanip>
#	include <iostream>
#	include <filesystem>
#endif

#include "file_types/file_types.hpp"
#include "file_table/file_table.hpp"
#include "sftp/sftp.hpp"
#include "config/options.hpp"
#include "stats/stats.hpp"
#include "terminal/terminal.hpp"
#include "error/error.hpp"
#include "hooks/hooks.hpp"

namespace lunas
{
	enum class sync_code {
		none,
		success,
		post_sync_fail,
		failed,
		interrupted,
	};

	class progress_bar {
		private:
			bool lock     = false;
			bool quiet    = false;
			bool progress = false;

		public:
			progress_bar(bool progress, bool quiet);
			void bar(const double& full_size, const double& occupied);
			void ingoing(const double& full_size, const double& occupied);
			~progress_bar();
	};

	struct buffque {
			std::vector<char>					      buffer;
			int							      bytes_xfered = 0;
			std::expected<std::unique_ptr<lunas::sftp_aio>, lunas::error> aio;

			explicit buffque(std::uint64_t size) : buffer(size)
			{
			}
	};

	struct syncstat {
			enum sync_code code	   = lunas::sync_code::none;
			std::uintmax_t copied_size = 0;
	};

	struct progress_stats {
			std::uintmax_t total_synced	  = 1;
			std::uintmax_t total_to_be_synced = 0;
	};

	struct syncmisc {
			const time_t		      src_mtime		   = 0;
			const lunas::file_types	      file_type		   = lunas::file_types::not_found;
			const bool		      is_dest_regular_file = false;
			const lunas::config::options& options;
			const lunas::progress_stats   progress_stats;
	};

	enum class ownership_type {
		uid,
		gid,
	};

	template<typename path_type = std::filesystem::path>
	struct file_metadata {
			const path_type&		    path;
			const lunas::metadata&		    metadata;
			const size_t			    index;
			const std::optional<std::uintmax_t> file_size;

			file_metadata& operator=(file_metadata& other)	= delete ("avoiding dangling reference situation");
			file_metadata(file_metadata& other)		= delete ("avoiding dangling reference situation");
			file_metadata& operator=(file_metadata&& other) = delete ("avoiding dangling reference situation");
			file_metadata(file_metadata&& other)		= delete ("avoiding dangling reference situation");

			explicit file_metadata(const path_type& path, const lunas::metadata& metadata, const size_t& index,
					       std::optional<std::uintmax_t> size = std::nullopt)
			    : path(path), metadata(metadata), index(index), file_size(size)
			{
				if constexpr (not std::is_same_v<path_type, std::filesystem::path>)
				{
					static_assert(false, "dangling reference situation will happen if 'path' is an rvalue or not a "
							     "std::filesystem::path");
				}
			}
	};

	bool no_ownership_value(const lunas::config::options& options);

	int  ownership_value(const lunas::config::options& options, int uid, enum ownership_type type);
}
