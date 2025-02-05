module;

#include <string>
#include <memory>
#include <exception>
#include <expected>
#include <libssh/sftp.h>

export module lunas.sftp:dir;
export import :attributes;
export import :error;
export import :log;

import lunas.stdout;

export namespace lunas {
	class sftp_dir {
		private:
			::sftp_dir m_dir = NULL;
			bool	   m_eof = false;

		public:
			sftp_dir(const sftp_session& sftp, const std::string& path);
			std::expected<std::unique_ptr<lunas::attributes>, lunas::error> read(const sftp_session& sftp);
			std::expected<bool, lunas::error> eof(const sftp_session& sftp, const std::string& path);
			~sftp_dir();
	};
}

namespace lunas {
	sftp_dir::sftp_dir(const sftp_session& sftp, const std::string& path) : m_dir(sftp_opendir(sftp, path.c_str())) {
		if (m_dir == NULL)
			throw std::runtime_error(fmt::err_path("couldn't open sftp directory", path));
	}

	std::expected<std::unique_ptr<lunas::attributes>, lunas::error> sftp_dir::read(const sftp_session& sftp) {
		::sftp_attributes attr = sftp_readdir(sftp, m_dir);
		if (attr == NULL && not sftp_dir_eof(m_dir))
			return std::unexpected(ssh_error(sftp, "sftp readdir error"));
		else if (sftp_dir_eof(m_dir)) {
			m_eof = true;
			return std::unexpected(lunas::error("", lunas::error_type::sftp_eof));
		}

		return std::make_unique<lunas::attributes>(attr);
	}

	std::expected<bool, lunas::error> sftp_dir::eof(const sftp_session& sftp, const std::string& path) {
		if (m_eof)
			return true;
		return std::unexpected(ssh_error(sftp, fmt::err_path("couldn't reach eof of sftp directory", path)));
	}

	sftp_dir::~sftp_dir() {
		if (m_dir != NULL)
			sftp_closedir(m_dir);
	}
}
