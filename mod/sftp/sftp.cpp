module;

#include <string_view>
#include <string>
#include <memory>
#include <expected>
#include <iostream>
#include <print>
#include <filesystem>
#include <thread>
#include <chrono>
#include <optional>
#include <libssh/sftp.h>
#include <libssh/libssh.h>

export module lunas.sftp;
export import :attributes;
export import :raii;
export import :ssh;
export import :log;
export import :error;
export import :path;

#define REMOTE_BUFFER_SIZE 65536 * 2
namespace fs = std::filesystem;

export namespace lunas {
	class sftp : public ssh {
		protected:
			::sftp_session m_sftp = NULL;

		public:
			sftp(const struct session_data& data);
			~sftp();

			const sftp_session& get_sftp_session();

			std::optional<ssh_error>			      unlink(const std::string& path);
			std::optional<ssh_error>			      rmdir(const std::string& path);
			std::optional<ssh_error>			      mkdir(const std::string& path, const unsigned int& perms);
			std::optional<ssh_error>			      symlink(const std::string& target, const std::string& path);
			std::expected<std::string, ssh_error>		      cmd(const std::string& command);
			std::expected<std::string, ssh_error>		      readlink(const std::string& link);
			std::expected<bool, ssh_error>			      is_broken_link(const std::string& link);
			std::expected<std::string, ssh_error>		      homedir();
			std::expected<std::string, ssh_error>		      homedir(const std::string_view& user);
			std::expected<std::string, ssh_error>		      cwd();
			std::expected<std::string, ssh_error>		      absolute_path();
			std::expected<std::unique_ptr<attributes>, ssh_error> attributes(const std::string& path, follow_symlink type);
	};
}

namespace lunas {
	sftp::sftp(const struct session_data& data) : ssh(data) {
		m_sftp = sftp_new(m_ssh);

		if (m_sftp == NULL) {
			std::string err = fmt::err_color("failed to create an sftp session to '" + session_data.ip + "'");
			err += fmt::err_color(ssh_get_error(m_ssh));
			throw std::runtime_error("\n" + err);
		}

		int rc = sftp_init(m_sftp);
		if (rc != SSH_OK) {
			std::string err = fmt::err_color("failed to initialize an sftp session to '" + session_data.ip + "'");
			err += fmt::err_color(ssh_get_error(m_ssh));
			throw std::runtime_error("\n" + err);
		}
	}

	std::optional<ssh_error> sftp::unlink(const std::string& path) {
		if (not session_data.options.dry_run && sftp_unlink(m_sftp, path.c_str()) != SSH_OK)
			return ssh_error(this->get_sftp_session());

		return std::nullopt;
	}

	std::optional<ssh_error> sftp::rmdir(const std::string& path) {
		if (not session_data.options.dry_run && sftp_rmdir(m_sftp, path.c_str()) != SSH_OK)
			return ssh_error(this->get_sftp_session());

		return std::nullopt;
	}

	std::optional<ssh_error> sftp::mkdir(const std::string& path, const unsigned int& perms = 0755) {
		if (not session_data.options.dry_run && sftp_mkdir(m_sftp, path.c_str(), perms) != SSH_OK)
			return ssh_error(this->get_sftp_session());

		return std::nullopt;
	}

	std::optional<ssh_error> sftp::symlink(const std::string& target, const std::string& path) {
		if (not session_data.options.dry_run && sftp_symlink(m_sftp, target.c_str(), path.c_str()) != SSH_OK)
			return ssh_error(this->get_sftp_session());

		return std::nullopt;
	}

	std::expected<std::unique_ptr<attributes>, ssh_error> sftp::attributes(const std::string& path, follow_symlink follow) {
		auto attr = std::make_unique<lunas::attributes>(m_sftp, path, follow);
		if (attr->exists())
			return attr;
		else
			return std::unexpected(ssh_error(this->get_sftp_session()));
	}

	std::expected<std::string, ssh_error> sftp::cmd(const std::string& command) {
		std::string output;
		ssh_channel channel = ssh_channel_new(m_ssh);
		int	    rc	    = ssh_channel_open_session(channel);
		if (rc != SSH_OK) {
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't execute", command)));
		}
		raii::sftp::channel channel_obj = raii::sftp::channel(&channel);

		rc = ssh_channel_request_exec(channel, command.c_str());
		if (rc != SSH_OK) {
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't execute", command)));
		}

		char buffer[REMOTE_BUFFER_SIZE];
		int  is_stderr = 0;
		while (true) {
			int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), is_stderr);
			if (nbytes == 0 && output.empty() && is_stderr == 0) {
				is_stderr = 1;
				continue;
			}

			if (nbytes < 0) {
				return std::unexpected(
				    ssh_error(this->get_sftp_session(), fmt::err_path("error while getting output of ", command)));
			}
			if (nbytes == 0)
				break;
			output.append(buffer, nbytes);
		}

		if (output.empty() != true && output.back() == '\n')
			output.pop_back();

		if (is_stderr == 1)
			return std::unexpected(ssh_error(output));

		return output;
	}

	std::expected<std::string, ssh_error> sftp::readlink(const std::string& link) {
		return this->cmd("readlink -f \"" + link + "\"");
	}

	std::expected<bool, ssh_error> sftp::is_broken_link(const std::string& link) {
		auto target = this->readlink(link);
		if (not target)
			return std::unexpected(target.error());

		auto attrs = this->attributes(target.value(), follow_symlink::no);
		if (attrs)
			return false;

		return true;
	}

	std::expected<std::string, ssh_error> sftp::homedir() {
		auto path = this->cmd("echo $HOME");
		if (not path)
			return std::unexpected(
			    ssh_error(this->get_sftp_session(), fmt::err_path("couldn't get home directory for", session_data.ip)));
		else if (path->empty())
			return std::unexpected(
			    ssh_error("couldn't get home directory for '" + session_data.ip + "' environment variable is empty"));

		path::append_seperator(path.value());
		return path.value();
	}

	std::expected<std::string, ssh_error> sftp::homedir(const std::string_view& user) {
		char* dir = sftp_home_directory(m_sftp, user.data());
		if (dir == NULL) {
			return std::unexpected(ssh_error(this->get_sftp_session()));
		}

		std::string str_dir = dir;
		ssh_string_free_char(dir);

		path::append_seperator(str_dir);
		return str_dir;
	}

	std::expected<std::string, ssh_error> sftp::cwd() {
		auto path = this->cmd("pwd");
		if (not path) {
			return std::unexpected(
			    ssh_error(this->get_sftp_session(), fmt::err_path("couldn't get cwd directory for", session_data.ip)));
		}

		path::append_seperator(path.value());
		return path.value();
	}

	const sftp_session& sftp::get_sftp_session() {
		return m_sftp;
	}

	std::expected<std::string, ssh_error> sftp::absolute_path(/*TODO: add a path parameter instead of sftp_path*/) {
		if (session_data.ip.find(':') == session_data.ip.npos || session_data.ip.back() == ':') {
			return std::unexpected(
			    ssh_error(fmt::err_color("hostname: " + session_data.ip + " doesn't include an input path")));
		}

		std::string sftp_path = session_data.ip.substr(session_data.ip.find(":") + 1, session_data.ip.size());
		std::expected<std::string, ssh_error> path;

		if (sftp_path.size() > 1 && sftp_path.substr(0, 2) == "~/") {
			path = this->homedir();
			if (not path)
				return std::unexpected(
				    lunas::ssh_error(this->get_sftp_session(), fmt::err_path("couldn't get homedir", session_data.ip)));
			sftp_path = path.value() + sftp_path.substr(2, sftp_path.size());

		} else if (sftp_path.size() > 1 && sftp_path.front() != path_seperator && sftp_path.front() != '.') {
			path = this->homedir();
			if (not path)
				return std::unexpected(
				    lunas::ssh_error(this->get_sftp_session(), fmt::err_path("couldn't get homedir", session_data.ip)));
			sftp_path = path.value() + sftp_path;
		}

		bool follow_symlink = false;
		if (follow_symlink) {
			char* full_path = sftp_canonicalize_path(m_sftp, sftp_path.c_str());
			if (full_path != NULL) {
				sftp_path = full_path;
				ssh_string_free_char(full_path);
			}
		} else if (sftp_path.size() > 2 && sftp_path.substr(0, 3) == "../") {
			auto current_path = this->cwd();
			if (not current_path)
				return std::unexpected(
				    lunas::ssh_error(this->get_sftp_session(), fmt::err_path("couldn't get cwd", session_data.ip)));
			path::pop_seperator(current_path.value());
			std::string prefix("", 3);
			for (auto itr = sftp_path.begin(); itr != sftp_path.end(); ++itr) {
				if (prefix.size() >= 3)
					prefix.clear();

				prefix += *itr;

				if (prefix == "../") {
					if (current_path->rfind(std::filesystem::path::preferred_separator) == current_path->npos) {
						return std::unexpected(lunas::ssh_error(
						    fmt::err_path("couldn't resolve relative path", session_data.ip, "too many '../'")));
					}

					current_path->resize(current_path->rfind(std::filesystem::path::preferred_separator));

					sftp_path = sftp_path.substr(3, sftp_path.size());
					itr	  = sftp_path.begin();
					prefix.clear();
					prefix += *itr;
				}
			}
			sftp_path = current_path.value() + std::filesystem::path::preferred_separator + sftp_path;
		}

		auto attributes = this->attributes(sftp_path, follow_symlink::yes);
		if (attributes && attributes.value()->file_type() == file_type::directory)
			path::append_seperator(sftp_path);

		return sftp_path;
	}

	sftp::~sftp() {
		if (m_sftp != NULL) {
			sftp_free(m_sftp);
			m_sftp = NULL;
		}
	}
}
