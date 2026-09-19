#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <filesystem>
#	include <memory>
#	include <expected>
#	include <variant>
#	include <stack>
#	include <chrono>
#	include <system_error>
#	include <cerrno>
#	include <cstring>
#	include <stdexcept>
#	include <exception>
#endif

#include "attributes.hpp"
#include "sftp/sftp.hpp"
#include "file_types/file_types.hpp"
#include "attributes/attributes.hpp"
#include "error/error.hpp"
#include "stdout/stdout.hpp"
#include "filter/filter.hpp"
#include "config/options.hpp"

namespace lunas
{
	struct directory_options {
			lunas::follow_symlink follow_symlink	= lunas::follow_symlink::no;
			bool		      no_broken_symlink = false;
			bool		      recursive		= true;
	};

	class local_directory {
		private:
			std::filesystem::directory_iterator itr;

		public:
			local_directory();
			local_directory(const std::filesystem::path path, const std::filesystem::directory_options& options);

			static std::expected<local_directory, lunas::error> init(const std::filesystem::path&	 path,
										 const struct directory_options& options)
			{
				try
				{
					std::filesystem::directory_options local_directory_options;

					if (options.follow_symlink == lunas::follow_symlink::yes)
					{
						local_directory_options = std::filesystem::directory_options::follow_directory_symlink;
					}
					return lunas::local_directory(path, local_directory_options);
				}
				catch (const lunas::error& e)
				{
					return std::unexpected(e);
				}
			}

			bool							      eof();
			std::expected<std::filesystem::directory_entry, lunas::error> read();
	};

	struct directory_entry {
			std::string				       filename;
			std::filesystem::path			       path;
			std::expected<lunas::file_types, lunas::error> file_type =
			    std::unexpected(error("empty directory_entry.file_type value"));
			std::expected<time_t, lunas::error>	    mtime     = std::unexpected(error("empty directory_entry.mtime value"));
			std::uintmax_t				    file_size = 0;
			std::expected<std::monostate, lunas::error> holds_attributes();
			std::expected<std::monostate, lunas::error> holds_file_type();
	};

	class directory {
			using remote_dirs_stack = std::stack<std::expected<std::unique_ptr<lunas::sftp_dir>, lunas::error>>;
			using local_dirs_stack	= std::stack<lunas::local_directory>;

		private:
			[[maybe_unused]] const std::unique_ptr<lunas::sftp>& sftp;
			std::variant<local_dirs_stack, remote_dirs_stack>    dir;
			directory_entry					     abstract_entry;
			struct directory_options			     directory_options;

			directory_entry convert_to_directory_entry(std::unique_ptr<lunas::sftp_attributes>& attr);
			directory_entry convert_to_directory_entry(std::filesystem::directory_entry& attr);

		public:
			directory(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path,
				  const struct directory_options& options);
			bool							   eof();
			[[nodiscard]] std::expected<directory_entry, lunas::error> read();
			[[nodiscard]] bool					   filter_out(const std::filesystem::path&  relative_path,
											      const lunas::config::options& options) const;
	};

	std::expected<std::unique_ptr<lunas::directory>, lunas::error>
	opendir(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path, const lunas::directory_options& options);
}
