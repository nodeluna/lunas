#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <cctype>
#	include <expected>
#	include <string>
#	include <algorithm>
#	include <print>
#	include <variant>
#	include <filesystem>
#	include <unordered_map>
#	include <functional>
#endif

#include "about/about.hpp"
#include "input_path/input_path.hpp"
#include "options.hpp"
#include "sftp/sftp.hpp"
#include "path/path.hpp"
#include "stdout/stdout.hpp"
#include "error/error.hpp"
#include "file_types/file_types.hpp"

namespace lunas
{
	namespace config
	{
		namespace filler
		{
			bool				is_num(const std::string& x);

			bool				is_num_decimal(const std::string& x);

			struct lunas::ipath::local_path fill_local_path(const std::string& argument, const lunas::ipath::srcdest& srcdest);

			struct lunas::ipath::local_path lpath_srcdest(const std::string& data);

			struct lunas::ipath::local_path lpath_src(const std::string& data);

			struct lunas::ipath::local_path lpath_dest(const std::string& data);

			lunas::ipath::srcdest		rpath_srcdest(void);

			lunas::ipath::srcdest		rpath_src(void);

			lunas::ipath::srcdest		rpath_dest(void);

			std::expected<std::monostate, lunas::error> compression_level(const std::string&      data,
										      lunas::config::options& options);

			std::expected<std::uintmax_t, lunas::error> parse_size(const std::string& data);

			std::expected<std::monostate, lunas::error> handle_size(const std::string&	       data,
										std::optional<std::uintmax_t>& size_variable);

			std::expected<std::monostate, lunas::error> minimum_space(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> max_file_size(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> min_file_size(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> exclude(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> exclude_pattern(const std::string&	    data,
										    lunas::config::options& options);

			std::expected<std::monostate, lunas::error> allow(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> allow_pattern(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> mkdir(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> progress(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> dry_run(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> quiet(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> verbose(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> follow_symlink(const std::string&	   data,
										   lunas::config::options& options);

			std::expected<std::monostate, lunas::error> no_broken_link(const std::string&	   data,
										   lunas::config::options& options);

			std::expected<std::monostate, lunas::error> link(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> no_recursive(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> resume(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> compression(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> attributes_uid(const std::string&	   data,
										   lunas::config::options& options);

			std::expected<std::monostate, lunas::error> attributes_gid(const std::string&	   data,
										   lunas::config::options& options);

			std::expected<std::monostate, lunas::error> attributes_own(const std::string&	   data,
										   lunas::config::options& options);

			std::expected<std::monostate, lunas::error> attributes_atime(const std::string&	     data,
										     lunas::config::options& options);

			std::expected<std::monostate, lunas::error> attributes_mtime(const std::string&	     data,
										     lunas::config::options& options);

			std::expected<std::monostate, lunas::error> attributes_utimes(const std::string&      data,
										      lunas::config::options& options);

			std::expected<std::monostate, lunas::error> attributes_all(const std::string&	   data,
										   lunas::config::options& options);

			std::expected<std::monostate, lunas::error> attributes(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> fsync(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> remove_extra(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> remove_before(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> remove_partials(const std::string&	    data,
										    lunas::config::options& options);

			std::expected<std::monostate, lunas::error> update(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> rollback(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> ssh_log(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> sftp_timeout(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> sftp_retries(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> connect_timeout(const std::string&	    data,
										    lunas::config::options& options);

			std::expected<std::monostate, lunas::error> mtime_grace(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> prehook(const std::string& data, lunas::config::options& options);

			std::expected<std::monostate, lunas::error> posthook(const std::string& data, lunas::config::options& options);
		}

		namespace info
		{
			void author(void);

			void version(void);

			void license(void);

			void help(void);

			void meow(void);
		}
	}
}
