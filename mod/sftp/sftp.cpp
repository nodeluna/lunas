module;

#include <chrono>
#include <libssh/sftp.h>
#include <libssh/libssh.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string_view>
#	include <string>
#	include <memory>
#	include <expected>
#	include <iostream>
#	include <print>
#	include <filesystem>
#	include <thread>
#	include <variant>
#endif

export module lunas.sftp;
export import :attributes;
export import :raii;
export import :ssh;
export import :log;
export import :error;
export import :path;
export import :dir;
export import :file;
export import :limits;

#define REMOTE_BUFFER_SIZE 65536 * 2
namespace fs = std::filesystem;

export namespace lunas {
	class sftp : public ssh {
		protected:
			::sftp_session m_sftp = NULL;

		public:
			enum class time_type : uint8_t {
				atime  = 1,
				mtime  = 2,
				utimes = 3,
			};

			struct time_val {
					time_t atime	  = 0;
					time_t atime_nsec = 0;
					time_t mtime	  = 0;
					time_t mtime_nsec = 0;
			};

			struct owner {
					int uid = -1;
					int gid = -1;
			};

			sftp(const struct session_data& data);
			~sftp();

			const sftp_session& get_sftp_session();

			std::expected<std::monostate, lunas::error> unlink(const std::string& path);
			std::expected<std::monostate, lunas::error> rmdir(const std::string& path);
			std::expected<std::monostate, lunas::error> mkdir(const std::string& path, const unsigned int& perms);
			std::expected<std::monostate, lunas::error> mkdir(const std::string& path, const std::filesystem::perms& perms);
			std::expected<std::monostate, lunas::error> symlink(const std::string& target, const std::string& path);
			std::expected<std::monostate, lunas::error> rename(const std::string& original, const std::string& newname);
			std::expected<std::unique_ptr<lunas::sftp_dir>, lunas::error>  opendir(const std::string& path);
			std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error> openfile(
			    const std::string& path, int access_type, mode_t mode);
			std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error> openfile(
			    const std::string& path, int access_type, std::filesystem::perms mode);
			std::expected<std::unique_ptr<lunas::sftp_limits>, lunas::error> limits();
			std::expected<std::string, lunas::error>			 cmd(const std::string& command);
			std::expected<std::string, lunas::error>			 readlink(const std::string& link);
			std::expected<bool, lunas::error>				 is_broken_link(const std::string& link);
			std::expected<std::string, lunas::error>			 homedir();
			std::expected<std::string, lunas::error>			 homedir(const std::string_view& user);
			std::expected<std::string, lunas::error>			 cwd();
			std::expected<std::string, lunas::error>			 absolute_path();
			std::expected<std::uintmax_t, lunas::error>			 file_size(const std::string& path);

			template<typename struct_time_val = time_val, typename struct_time_type = time_type>
			std::expected<struct_time_val, lunas::error> get_utimes(
			    const std::string& path, const struct_time_type time_type, const lunas::follow_symlink follow);

			template<typename struct_time_val = time_val>
			std::expected<std::monostate, lunas::error> set_utimes(
			    const std::string& path, const struct_time_val& time_val, const lunas::follow_symlink follow);

			std::expected<uint32_t, lunas::error> get_permissions(const std::string& path, const lunas::follow_symlink follow);

			std::expected<sftp::owner, lunas::error> get_ownership(const std::string& path, const lunas::follow_symlink follow);
			std::expected<std::monostate, lunas::error> set_ownership(
			    const std::string& path, const sftp::owner own, const lunas::follow_symlink follow);
			std::expected<std::unique_ptr<lunas::sftp_attributes>, lunas::error> attributes(
			    const std::string& path, follow_symlink type);

			std::string get_str_error();
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

	std::expected<std::monostate, lunas::error> sftp::unlink(const std::string& path) {
		if (not session_data.options.dry_run && sftp_unlink(m_sftp, path.c_str()) != SSH_OK)
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't remove file", path)));

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> sftp::rmdir(const std::string& path) {
		if (not session_data.options.dry_run && sftp_rmdir(m_sftp, path.c_str()) != SSH_OK)
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't remove directory", path)));

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> sftp::mkdir(const std::string& path, const unsigned int& perms = 0755) {
		if (not session_data.options.dry_run && sftp_mkdir(m_sftp, path.c_str(), perms) != SSH_OK)
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't make directory", path)));

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> sftp::mkdir(const std::string& path, const std::filesystem::perms& perms) {
		if (not session_data.options.dry_run && sftp_mkdir(m_sftp, path.c_str(), ( unsigned int ) perms) != SSH_OK)
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't make directory", path)));

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> sftp::symlink(const std::string& target, const std::string& path) {
		if (not session_data.options.dry_run && sftp_symlink(m_sftp, target.c_str(), path.c_str()) != SSH_OK)
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't make symlink", path)));

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> sftp::rename(const std::string& original, const std::string& newname) {
		if (not session_data.options.dry_run && sftp_rename(m_sftp, original.c_str(), newname.c_str()) != SSH_OK)
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't rename file", original)));

		return std::monostate();
	}

	std::expected<std::unique_ptr<lunas::sftp_dir>, lunas::error> sftp::opendir(const std::string& path) {
		try {
			return std::make_unique<lunas::sftp_dir>(this->get_sftp_session(), path);
		} catch (const std::exception& e) {
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't open directory", path)));
		}
	}

	std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error> sftp::openfile(
	    const std::string& path, int access_type, mode_t mode) {
		try {
			return std::make_unique<lunas::sftp_file>(this->get_sftp_session(), path, access_type, mode);
		} catch (const std::exception& e) {
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't open file", path)));
		}
	}

	std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error> sftp::openfile(
	    const std::string& path, int access_type, std::filesystem::perms mode) {
		try {
			return std::make_unique<lunas::sftp_file>(this->get_sftp_session(), path, access_type, mode);
		} catch (const std::exception& e) {
			return std::unexpected(ssh_error(this->get_sftp_session(), fmt::err_path("couldn't open file", path)));
		}
	}

	std::expected<std::unique_ptr<lunas::sftp_limits>, lunas::error> sftp::limits() {
		try {
			return std::make_unique<lunas::sftp_limits>(this->get_sftp_session());
		} catch (const std::exception& e) {
			return std::unexpected(ssh_error(this->get_sftp_session(), "couldn't get sftp_limits"));
		}
	}

	std::expected<std::unique_ptr<sftp_attributes>, lunas::error> sftp::attributes(const std::string& path, follow_symlink follow) {
		auto attr = std::make_unique<lunas::sftp_attributes>(m_sftp, path, follow);
		if (attr->exists())
			return attr;
		else
			return std::unexpected(ssh_error(this->get_sftp_session()));
	}

	std::expected<std::string, lunas::error> sftp::cmd(const std::string& command) {
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

	std::expected<std::string, lunas::error> sftp::readlink(const std::string& link) {
		return this->cmd("readlink -f \"" + link + "\"");
	}

	std::expected<bool, lunas::error> sftp::is_broken_link(const std::string& link) {
		auto target = this->readlink(link);
		if (not target)
			return std::unexpected(target.error());

		auto attrs = this->attributes(target.value(), follow_symlink::no);
		if (attrs)
			return false;

		return true;
	}

	std::expected<std::string, lunas::error> sftp::homedir() {
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

	std::expected<std::string, lunas::error> sftp::homedir(const std::string_view& user) {
		char* dir = sftp_home_directory(m_sftp, user.data());
		if (dir == NULL) {
			return std::unexpected(ssh_error(this->get_sftp_session()));
		}

		std::string str_dir = dir;
		ssh_string_free_char(dir);

		path::append_seperator(str_dir);
		return str_dir;
	}

	std::expected<std::string, lunas::error> sftp::cwd() {
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

	template<typename struct_time_val, typename struct_time_type>
	std::expected<struct_time_val, lunas::error> sftp::get_utimes(
	    const std::string& path, const struct_time_type time_type, const lunas::follow_symlink follow) {

		auto attr = this->attributes(path, follow);
		if (not attr)
			return std::unexpected(attr.error());
		auto& attributes = attr.value();

		struct_time_val time_val;
		struct timespec timespec;
		int		rv;

		switch (time_type) {
			case struct_time_type::atime:
				time_val.atime	    = attributes->atime();
				time_val.atime_nsec = attributes->atime_nseconds();

				rv = clock_gettime(CLOCK_REALTIME, &timespec);
				if (rv == 0) {
					time_val.mtime	    = timespec.tv_sec;
					time_val.mtime_nsec = timespec.tv_nsec;
				} else
					return std::unexpected(
					    lunas::ssh_error(this->get_sftp_session(), fmt::err_path("couldn't get utimes", path)));
				break;
			case struct_time_type::mtime:
				time_val.mtime	    = attributes->mtime();
				time_val.mtime_nsec = attributes->mtime_nseconds();

				rv = clock_gettime(CLOCK_REALTIME, &timespec);
				if (rv == 0) {
					time_val.atime	    = timespec.tv_sec;
					time_val.atime_nsec = timespec.tv_nsec;
				} else
					return std::unexpected(
					    lunas::ssh_error(this->get_sftp_session(), fmt::err_path("couldn't get utimes", path)));
				break;
			case struct_time_type::utimes:
				time_val.atime	    = attributes->atime();
				time_val.atime_nsec = attributes->atime_nseconds();
				time_val.mtime	    = attributes->mtime();
				time_val.mtime_nsec = attributes->mtime_nseconds();
				break;
			default:
				break;
		}

		return time_val;
	}

	template<typename struct_time_val>
	std::expected<std::monostate, lunas::error> sftp::set_utimes(
	    const std::string& path, const struct_time_val& time_val, const lunas::follow_symlink follow) {

		struct sftp_attributes_struct attributes;
		attributes.flags	  = SSH_FILEXFER_ATTR_ACMODTIME;
		attributes.atime	  = time_val.atime;
		attributes.atime_nseconds = time_val.atime_nsec;
		attributes.mtime	  = time_val.mtime;
		attributes.mtime_nseconds = time_val.mtime_nsec;

		int rc = SSH_OK;

		if (follow == lunas::follow_symlink::yes)
			rc = sftp_setstat(m_sftp, path.c_str(), &attributes);
		else
			rc = sftp_lsetstat(m_sftp, path.c_str(), &attributes);

		if (rc != SSH_OK)
			return std::unexpected(lunas::ssh_error(this->get_sftp_session(), fmt::err_path("couldn't set utimes", path)));

		return std::monostate();
	}

	std::expected<sftp::owner, lunas::error> sftp::get_ownership(const std::string& path, const lunas::follow_symlink follow) {
		struct owner own;

		auto attr = this->attributes(path, follow);
		if (not attr)
			return std::unexpected(attr.error());
		auto& attributes = attr.value();

		own.uid = attributes->uid();
		own.gid = attributes->gid();
		return own;
	}

	std::expected<uint32_t, lunas::error> sftp::get_permissions(const std::string& path, const lunas::follow_symlink follow) {
		auto attr = this->attributes(path, follow);
		if (not attr)
			return std::unexpected(attr.error());

		unsigned int perms = attr.value()->permissions();
		return perms;
	}

	std::expected<std::uintmax_t, lunas::error> sftp::file_size(const std::string& path) {
		auto attr = this->attributes(path, lunas::follow_symlink::yes);
		if (not attr)
			return std::unexpected(attr.error());

		return attr.value()->file_size();
	}

	std::expected<std::monostate, lunas::error> sftp::set_ownership(
	    const std::string& path, const sftp::owner own, const lunas::follow_symlink follow) {
		struct sftp_attributes_struct attributes;
		attributes.flags = SSH_FILEXFER_ATTR_UIDGID;
		attributes.uid	 = own.uid;
		attributes.gid	 = own.gid;

		if (own.uid == -1 || own.gid == -1) {
			auto attr = this->attributes(path, follow);
			if (not attr)
				return std::unexpected(attr.error());
			if (own.uid == -1)
				attributes.uid = attr.value()->uid();
			if (own.gid == -1)
				attributes.gid = attr.value()->gid();
		}

		int rc;
		if (follow == lunas::follow_symlink::yes)
			rc = sftp_setstat(m_sftp, path.c_str(), &attributes);
		else
			rc = sftp_lsetstat(m_sftp, path.c_str(), &attributes);

		if (rc != SSH_OK)
			return std::unexpected(lunas::ssh_error(this->get_sftp_session(), fmt::err_path("couldn't set ownership", path)));

		return std::monostate();
	}

	std::expected<std::string, lunas::error> sftp::absolute_path(/*TODO: add a path parameter instead of sftp_path*/) {
		if (session_data.ip.find(':') == session_data.ip.npos || session_data.ip.back() == ':') {
			return std::unexpected(
			    ssh_error(fmt::err_color("hostname: " + session_data.ip + " doesn't include an input path")));
		}

		std::string sftp_path = session_data.ip.substr(session_data.ip.find(":") + 1, session_data.ip.size());
		std::expected<std::string, lunas::error> path;

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
		if (attributes && attributes.value()->file_type() == lunas::file_types::directory)
			path::append_seperator(sftp_path);

		return sftp_path;
	}

	std::string sftp::get_str_error() {
		return ssh_get_error(m_ssh);
	}

	sftp::~sftp() {
		if (m_sftp != NULL) {
			sftp_free(m_sftp);
			m_sftp = NULL;
		}
	}
}
