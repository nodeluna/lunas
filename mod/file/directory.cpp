#include <filesystem>
#include <memory>
#include <expected>
#include <variant>
#include <stack>
#include <system_error>

#include "directory.hpp"
#include "sftp/sftp.hpp"
#include "file_types/file_types.hpp"
#include "attributes/attributes.hpp"
#include "error/error.hpp"
#include "filter/filter.hpp"
#include "config/options.hpp"

namespace lunas
{
	local_directory::local_directory()
	{
	}

	local_directory::local_directory(const std::filesystem::path path, const std::filesystem::directory_options& options)
	{
		std::error_code ec;
		itr = std::filesystem::directory_iterator(path, options, ec);
		if (ec.value() != 0)
		{
			auto error_type =
			    ec == std::errc::no_such_file_or_directory ? lunas::error_type::no_such_file : lunas::error_type::opendir;
			throw lunas::error("couldn't open directory '" + path.string() + "', " + ec.message(), error_type);
		}
	}

	bool local_directory::eof()
	{
		return itr == std::default_sentinel_t();
	}

	std::expected<std::filesystem::directory_entry, lunas::error> local_directory::read()
	{
		if (this->eof())
		{
			return std::unexpected(lunas::error("", lunas::error_type::readdir_eof));
		}

		std::filesystem::directory_entry entry = *itr;
		++itr;
		return entry;
	}

	directory_entry directory::convert_to_directory_entry(std::unique_ptr<lunas::sftp_attributes>& attr)
	{
		abstract_entry.filename		= attr->name();
		abstract_entry.path		= attr->path();
		abstract_entry.file_type	= attr->file_type();

		auto&	   file_type		= abstract_entry.file_type.value();
		const auto unfollowed_file_type = file_type;

		if (file_type == lunas::file_types::symlink)
		{
			if (directory_options.no_broken_symlink && sftp->is_broken_link(abstract_entry.path))
			{
				abstract_entry.file_type = lunas::file_types::brokenlink;
			}
			else if (directory_options.follow_symlink == lunas::follow_symlink::yes)
			{
				auto attr = sftp->attributes(abstract_entry.path, lunas::follow_symlink::yes);
				if (not attr)
				{
					abstract_entry.file_type = std::unexpected(attr.error());
				}
				else
				{
					abstract_entry.file_type = attr.value()->file_type();
				}
			}
		}

		if (not abstract_entry.file_type)
		{
			return abstract_entry;
		}

		if (file_type != lunas::file_types::brokenlink && unfollowed_file_type == lunas::file_types::symlink &&
		    directory_options.follow_symlink == lunas::follow_symlink::yes)
		{
			auto ok = sftp->get_utimes(abstract_entry.path, lunas::sftp::time_type::mtime, directory_options.follow_symlink);
			if (not ok)
			{
				abstract_entry.mtime = std::unexpected(ok.error());
			}
			else
			{
				abstract_entry.mtime = ok.value().mtime;
			}
		}
		else
		{
			abstract_entry.mtime = attr->mtime();
		}

		file_type = lunas::if_lspart_return_resume_type(abstract_entry.path, file_type);
		if (file_type == lunas::file_types::regular_file)
		{
			abstract_entry.file_size = attr->file_size();
		}
		return abstract_entry;
	}

	directory_entry directory::convert_to_directory_entry(std::filesystem::directory_entry& attr)
	{
		abstract_entry.filename = attr.path().filename();
		abstract_entry.path	= attr.path().string();

		if (directory_options.follow_symlink == lunas::follow_symlink::yes)
		{
			abstract_entry.file_type = get_file_type(attr.status());
		}
		else
		{
			abstract_entry.file_type = get_file_type(attr.symlink_status());
		}

		auto& file_type = abstract_entry.file_type.value();

		if (directory_options.no_broken_symlink && file_type == lunas::file_types::symlink)
		{
			std::expected<bool, lunas::error> broken = lunas::is_broken_link(abstract_entry.path);
			if (not broken)
			{
				abstract_entry.file_type = std::unexpected(broken.error());
			}
			if (broken.value())
			{
				abstract_entry.file_type = lunas::file_types::brokenlink;
			}
		}

		if (not abstract_entry.file_type)
		{
			return abstract_entry;
		}

		std::expected<lunas::time_val, lunas::error> ok;
		if (file_type != lunas::file_types::brokenlink)
		{
			ok = lunas::utime::get(abstract_entry.path, lunas::time_type::mtime, directory_options.follow_symlink);
		}
		else
		{
			ok = lunas::utime::get(abstract_entry.path, lunas::time_type::mtime, lunas::follow_symlink::no);
		}

		if (not ok)
		{
			abstract_entry.mtime = std::unexpected(ok.error());
		}
		else
		{
			abstract_entry.mtime = ok.value().mtime;
		}

		file_type = lunas::if_lspart_return_resume_type(abstract_entry.path, file_type);
		if (file_type == lunas::file_types::regular_file)
		{
			abstract_entry.file_size = attr.file_size();
		}
		return abstract_entry;
	}

	directory::directory(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path,
			     const struct directory_options& options)
	    : sftp(sftp), directory_options(options)
	{
		if (sftp != nullptr)
		{
			remote_dirs_stack temp;
			dir		= std::move(temp);
			auto& sftp_dirs = std::get<remote_dirs_stack>(dir);
			sftp_dirs.push(sftp->opendir(path.string()));
			if (not sftp_dirs.top())
			{
				throw lunas::error(sftp_dirs.top().error());
			}
		}
		else
		{
			local_dirs_stack temp;
			dir		 = std::move(temp);
			auto& local_dirs = std::get<local_dirs_stack>(dir);
			auto  local_dir	 = lunas::local_directory::init(path, options);
			if (not local_dir)
			{
				throw local_dir.error();
			}
			local_dirs.push(local_dir.value());
		}
	}

	std::expected<std::monostate, lunas::error> directory_entry::holds_attributes()
	{
		if (not this->file_type)
		{
			return std::unexpected(this->file_type.error());
		}
		else if (not this->mtime)
		{
			return std::unexpected(this->mtime.error());
		}

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> directory_entry::holds_file_type()
	{
		if (not this->file_type)
		{
			return std::unexpected(this->file_type.error());
		}

		return std::monostate();
	}

	bool directory::eof()
	{
		if (std::holds_alternative<remote_dirs_stack>(dir))
		{
			return std::get<remote_dirs_stack>(dir).empty();
		}
		else
		{
			return std::get<local_dirs_stack>(dir).empty();
		}
	}

	[[nodiscard]] std::expected<directory_entry, lunas::error> directory::read()
	{
		if (this->eof())
		{
			return std::unexpected(lunas::error("", lunas::error_type::readdir_eof));
		}

		if (std::holds_alternative<remote_dirs_stack>(dir))
		{
			auto& sftp_dirs = std::get<remote_dirs_stack>(dir);

			if (directory_options.recursive && abstract_entry.holds_file_type() &&
			    abstract_entry.file_type.value() == lunas::file_types::directory)
			{
				sftp_dirs.push(sftp->opendir(abstract_entry.path));
				if (not sftp_dirs.top())
				{
					abstract_entry = directory_entry{};
					return std::unexpected(sftp_dirs.top().error());
				}
			}

			std::expected<std::unique_ptr<lunas::sftp_attributes>, lunas::error> remote_entry;
			while (not sftp_dirs.empty())
			{
				remote_entry = sftp_dirs.top().value()->read(sftp->get_sftp_session());
				if (not remote_entry)
				{
					if (remote_entry.error().value() == lunas::error_type::sftp_eof)
					{
						sftp_dirs.pop();
					}
					else
					{
						return std::unexpected(remote_entry.error());
					}
					continue;
				}
				std::string filename = remote_entry.value()->name();
				if (filename != "." && filename != "..")
				{
					break;
				}
			}
			if (sftp_dirs.empty())
			{
				return std::unexpected(lunas::error("", lunas::error_type::readdir_eof));
			}

			return convert_to_directory_entry(remote_entry.value());
		}
		else
		{
			auto& local_dirs = std::get<local_dirs_stack>(dir);

			if (directory_options.recursive && abstract_entry.holds_file_type() &&
			    abstract_entry.file_type.value() == lunas::file_types::directory)
			{
				auto local_dir = lunas::local_directory::init(abstract_entry.path, directory_options);
				if (not local_dir)
				{
					abstract_entry = directory_entry{};
					return std::unexpected(local_dir.error());
				}
				local_dirs.push(local_dir.value());
			}

			std::expected<std::filesystem::directory_entry, lunas::error> entry;
			while (not local_dirs.empty())
			{
				entry = local_dirs.top().read();
				if (not entry)
				{
					if (entry.error().value() == lunas::error_type::readdir_eof)
					{
						local_dirs.pop();
					}
					else
					{
						return std::unexpected(entry.error());
					}
				}
				else
				{
					break;
				}
			}
			if (local_dirs.empty())
			{
				return std::unexpected(lunas::error("", lunas::error_type::readdir_eof));
			}

			return convert_to_directory_entry(entry.value());
		}
	}

	[[nodiscard]] bool directory::filter_out(const std::filesystem::path& relative_path, const lunas::config::options& options) const
	{
		if (not lunas::allow(relative_path, options.allow, options.allow_pattern))
		{
			return true;
		}
		else if (lunas::exclude(relative_path, options.exclude, options.exclude_pattern))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	std::expected<std::unique_ptr<lunas::directory>, lunas::error>
	opendir(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path, const lunas::directory_options& options)
	{
		try
		{
			return std::make_unique<lunas::directory>(sftp, path, options);
		}
		catch (const lunas::error& error)
		{
			return std::unexpected(error);
		}
	}
}
