module;

#include <filesystem>
#include <memory>
#include <expected>
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

export namespace lunas {
	class local_directory {
		private:
			std::filesystem::recursive_directory_iterator itr;
			std::filesystem::recursive_directory_iterator end;

		public:
			local_directory();
			local_directory(const std::filesystem::path path);
			bool							      eof();
			std::expected<std::filesystem::directory_entry, lunas::error> read();
	};

	struct directory_entry {
			std::string	  filename;
			std::string	  path;
			lunas::file_types file_type = lunas::file_types::not_found;
			time_t		  mtime	    = 0;
	};

	class directory {
			using remote_dirs_stack = std::stack<std::expected<std::unique_ptr<lunas::sftp_dir>, lunas::error>>;

		private:
			[[maybe_unused]] const std::unique_ptr<lunas::sftp>&	sftp;
			std::variant<lunas::local_directory, remote_dirs_stack> dir;
			directory_entry						abstract_entry;

			directory_entry convert_to_directory_entry(std::unique_ptr<lunas::sftp_attributes>& attr);
			directory_entry convert_to_directory_entry(std::filesystem::directory_entry& attr);

		public:
			directory(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path);
			bool							   eof();
			[[nodiscard]] std::expected<directory_entry, lunas::error> read();
	};

	std::expected<std::unique_ptr<lunas::directory>, lunas::error> opendir(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path);
}

namespace lunas {
	local_directory::local_directory() {
	}

	local_directory::local_directory(const std::filesystem::path path)
	    : itr(path), end(std::filesystem::recursive_directory_iterator()) {
	}

	bool local_directory::eof() {
		return itr == end;
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
		abstract_entry.mtime	 = attr->mtime();
		abstract_entry.file_type = lunas::if_lspart_return_resume_type(abstract_entry.path, abstract_entry.file_type);
		return abstract_entry;
	}

	directory_entry directory::convert_to_directory_entry(std::filesystem::directory_entry& attr) {
		abstract_entry.filename	 = attr.path().filename();
		abstract_entry.path	 = attr.path().string();
		abstract_entry.file_type = get_file_type(attr.symlink_status());
		abstract_entry.mtime	 = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(attr.last_write_time()));
		abstract_entry.file_type = lunas::if_lspart_return_resume_type(abstract_entry.path, abstract_entry.file_type);
		return abstract_entry;
	}

	directory::directory(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path) : sftp(sftp) {
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
				local_dir = lunas::local_directory(path);
			} catch (const std::exception& e) {
				throw lunas::error(
				    "couldn't open directory '" + path.string() + ", " + std::strerror(errno), lunas::error_type::opendir);
			}
		}
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

			if (abstract_entry.file_type == lunas::file_types::directory) {
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
	    const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path) {
		try {
			return std::make_unique<lunas::directory>(sftp, path);
		} catch (const lunas::error& error) {
			return std::unexpected(error);
		}
	}
}
