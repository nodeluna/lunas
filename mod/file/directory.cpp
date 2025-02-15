module;

#include <filesystem>
#include <memory>
#include <expected>
#include <variant>
#include <stack>
#include <chrono>
#include <system_error>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <exception>

export module lunas.file:directory;
export import :attributes;

export import lunas.sftp;
export import lunas.file_types;
export import lunas.attributes;
export import lunas.error;
export import lunas.stdout;
export import lunas.file_types;

export namespace lunas {
	struct directory_options {
			lunas::follow_symlink follow_symlink	= lunas::follow_symlink::no;
			bool		      no_broken_symlink = false;
	};

	class local_directory {
		private:
			std::filesystem::recursive_directory_iterator itr;

		public:
			local_directory();
			local_directory(const std::filesystem::path path, const std::filesystem::directory_options& options);
			bool							      eof();
			std::expected<std::filesystem::directory_entry, lunas::error> read();
	};

	struct directory_entry {
			std::string				      filename;
			std::string				      path;
			std::variant<lunas::file_types, lunas::error> file_type = lunas::error("empty directory_entry.file_type value");
			std::variant<time_t, lunas::error>	      mtime	= lunas::error("empty directory_entry.mtime value");
			std::expected<std::monostate, lunas::error>   holds_attributes();
	};

	class directory {
			using remote_dirs_stack = std::stack<std::expected<std::unique_ptr<lunas::sftp_dir>, lunas::error>>;

		private:
			[[maybe_unused]] const std::unique_ptr<lunas::sftp>&	sftp;
			std::variant<lunas::local_directory, remote_dirs_stack> dir;
			directory_entry						abstract_entry;
			struct directory_options				directory_options;

			directory_entry convert_to_directory_entry(std::unique_ptr<lunas::sftp_attributes>& attr);
			directory_entry convert_to_directory_entry(std::filesystem::directory_entry& attr);

		public:
			directory(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path,
			    const struct directory_options& options);
			bool							   eof();
			[[nodiscard]] std::expected<directory_entry, lunas::error> read();
	};

	std::expected<std::unique_ptr<lunas::directory>, lunas::error> opendir(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path, const lunas::directory_options& options);
}

namespace lunas {
	local_directory::local_directory() {
	}

	local_directory::local_directory(const std::filesystem::path path, const std::filesystem::directory_options& options)
	    : itr(path, options) {
	}

	bool local_directory::eof() {
		return itr == std::default_sentinel_t();
	}

	std::expected<std::filesystem::directory_entry, lunas::error> local_directory::read() {
		if (this->eof())
			return std::unexpected(lunas::error("", lunas::error_type::readdir_eof));

		std::filesystem::directory_entry entry = *itr;
		++itr;
		return entry;
	}

	directory_entry directory::convert_to_directory_entry(std::unique_ptr<lunas::sftp_attributes>& attr) {
		abstract_entry.filename	 = attr->name();
		abstract_entry.path	 = attr->path();
		abstract_entry.file_type = attr->file_type();

		auto&	   file_type		= std::get<lunas::file_types>(abstract_entry.file_type);
		const auto unfollowed_file_type = file_type;
		if (file_type == lunas::file_types::symlink) {
			if (directory_options.no_broken_symlink && sftp->is_broken_link(abstract_entry.path)) {
				abstract_entry.file_type = lunas::file_types::brokenlink;
			} else if (directory_options.follow_symlink == lunas::follow_symlink::yes) {
				auto attr = sftp->attributes(abstract_entry.path, lunas::follow_symlink::yes);
				if (not attr)
					abstract_entry.file_type = attr.error();
				else
					abstract_entry.file_type = attr.value()->file_type();
			}
		}

		if (not std::holds_alternative<lunas::file_types>(abstract_entry.file_type))
			return abstract_entry;

		if (file_type != lunas::file_types::brokenlink && unfollowed_file_type == lunas::file_types::symlink &&
		    directory_options.follow_symlink == lunas::follow_symlink::yes) {
			auto ok = sftp->get_utimes(abstract_entry.path, lunas::sftp::time_type::mtime, directory_options.follow_symlink);
			if (not ok)
				abstract_entry.mtime = ok.error();
			else
				abstract_entry.mtime = ok.value().mtime;
		} else {
			abstract_entry.mtime = attr->mtime();
		}

		file_type = lunas::if_lspart_return_resume_type(abstract_entry.path, file_type);
		return abstract_entry;
	}

	directory_entry directory::convert_to_directory_entry(std::filesystem::directory_entry& attr) {
		abstract_entry.filename = attr.path().filename();
		abstract_entry.path	= attr.path().string();

		if (directory_options.follow_symlink == lunas::follow_symlink::yes)
			abstract_entry.file_type = get_file_type(attr.status());
		else
			abstract_entry.file_type = get_file_type(attr.symlink_status());

		auto& file_type = std::get<lunas::file_types>(abstract_entry.file_type);

		if (directory_options.no_broken_symlink && file_type == lunas::file_types::symlink) {
			std::expected<bool, lunas::error> broken = lunas::is_broken_link(abstract_entry.path);
			if (not broken)
				abstract_entry.file_type = broken.error();
			if (broken.value())
				abstract_entry.file_type = lunas::file_types::brokenlink;
		}

		if (not std::holds_alternative<lunas::file_types>(abstract_entry.file_type))
			return abstract_entry;

		std::expected<lunas::time_val, lunas::error> ok;
		if (file_type != lunas::file_types::brokenlink)
			ok = lunas::utime::get(abstract_entry.path, lunas::time_type::mtime, directory_options.follow_symlink);
		else
			ok = lunas::utime::get(abstract_entry.path, lunas::time_type::mtime, lunas::follow_symlink::no);

		if (not ok)
			abstract_entry.mtime = ok.error();
		else
			abstract_entry.mtime = ok.value().mtime;

		file_type = lunas::if_lspart_return_resume_type(abstract_entry.path, file_type);
		return abstract_entry;
	}

	directory::directory(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path, const struct directory_options& options)
	    : sftp(sftp), directory_options(options) {
		if (sftp != nullptr) {
			remote_dirs_stack temp;
			dir		= std::move(temp);
			auto& sftp_dirs = std::get<remote_dirs_stack>(dir);
			sftp_dirs.push(sftp->opendir(path.string()));
			if (not sftp_dirs.top())
				throw lunas::error(sftp_dirs.top().error());
		} else {
			lunas::local_directory temp;
			dir		= std::move(temp);
			auto& local_dir = std::get<lunas::local_directory>(dir);
			try {
				std::filesystem::directory_options local_directory_options;
				if (options.follow_symlink == lunas::follow_symlink::yes)
					local_directory_options = std::filesystem::directory_options::follow_directory_symlink;
				local_dir = lunas::local_directory(path, local_directory_options);
			} catch (const std::exception& e) {
				throw lunas::error(
				    "couldn't open directory '" + path.string() + ", " + std::strerror(errno), lunas::error_type::opendir);
			}
		}
	}

	std::expected<std::monostate, lunas::error> directory_entry::holds_attributes() {
		if (not std::holds_alternative<lunas::file_types>(this->file_type))
			return std::unexpected(std::get<lunas::error>(this->file_type));
		else if (not std::holds_alternative<time_t>(this->mtime))
			return std::unexpected(std::get<lunas::error>(this->mtime));

		return std::monostate();
	}

	bool directory::eof() {
		if (std::holds_alternative<remote_dirs_stack>(dir))
			return std::get<remote_dirs_stack>(dir).empty();
		else
			return std::get<lunas::local_directory>(dir).eof();
	}

	[[nodiscard]] std::expected<directory_entry, lunas::error> directory::read() {
		if (this->eof())
			return std::unexpected(lunas::error("", lunas::error_type::readdir_eof));

		if (std::holds_alternative<remote_dirs_stack>(dir)) {
			auto& sftp_dirs = std::get<remote_dirs_stack>(dir);

			if (std::holds_alternative<lunas::file_types>(abstract_entry.file_type) &&
			    std::get<lunas::file_types>(abstract_entry.file_type) == lunas::file_types::directory) {
				sftp_dirs.push(sftp->opendir(abstract_entry.path));
				if (not sftp_dirs.top())
					return std::unexpected(sftp_dirs.top().error());
			}

			std::expected<std::unique_ptr<lunas::sftp_attributes>, lunas::error> remote_entry;
			while (not sftp_dirs.empty()) {
				remote_entry = sftp_dirs.top().value()->read(sftp->get_sftp_session());
				if (not remote_entry) {
					if (remote_entry.error().value() == lunas::error_type::sftp_eof)
						sftp_dirs.pop();
					else
						return std::unexpected(remote_entry.error());
					continue;
				}
				std::string filename = remote_entry.value()->name();
				if (filename != "." && filename != "..")
					break;
			}
			if (sftp_dirs.empty())
				return std::unexpected(lunas::error("", lunas::error_type::readdir_eof));

			return convert_to_directory_entry(remote_entry.value());
		} else {
			auto& local_dir = std::get<lunas::local_directory>(dir);
			auto  entry	= local_dir.read();
			if (not entry)
				return std::unexpected(entry.error());
			return convert_to_directory_entry(entry.value());
		}
	}

	std::expected<std::unique_ptr<lunas::directory>, lunas::error> opendir(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path, const lunas::directory_options& options) {
		try {
			return std::make_unique<lunas::directory>(sftp, path, options);
		} catch (const lunas::error& error) {
			return std::unexpected(error);
		}
	}
}
