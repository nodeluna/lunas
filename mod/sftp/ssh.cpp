module;

#include <libssh/sftp.h>
#include <libssh/libssh.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string>
#	include <filesystem>
#	include <chrono>
#	include <thread>
#	include <print>
#	include <iostream>
#endif

export module lunas.sftp:ssh;
import :raii;
import :log;

namespace fs = std::filesystem;
typedef int auth_response;

export namespace lunas {
	enum class ssh_log_level {
		no_log	  = SSH_LOG_NOLOG,
		warning	  = SSH_LOG_WARNING,
		protocol  = SSH_LOG_PROTOCOL,
		packet	  = SSH_LOG_PACKET,
		functions = SSH_LOG_FUNCTIONS,
	};

	struct session_data {
			~session_data() {
				if (not pw.empty())
					pw.clear();
			}

			std::string ip;
			int	    port = 22;
			std::string pw;
			std::string key_path;

			struct options {
					int	      compression_level = 0;
					int	      timeout		= 5;
					ssh_log_level log_level		= ssh_log_level::no_log;
					bool	      dry_run		= false;
			} options;
	};

	class ssh {
		protected:
			struct session_data session_data;
			ssh_session	    m_ssh = NULL;

		private:
			auth_response verify_publickey(const ssh_session& ssh, const std::string& ip);
			auth_response auth_password(const ssh_session& ssh, const std::string& pw);
			auth_response auth_publickey_manual(const ssh_session& ssh);
			auth_response auth_none(const ssh_session& ssh);
			std::string   auth_method(const int& method);
			auth_response auth_list(const ssh_session& ssh, const std::string& ip, const std::string& pw);
			int	      set_options(const ssh_session& ssh, ssh_options_e option, void* argument, const std::string& warning);

		public:
			ssh(const struct session_data& data);
			const ssh_session& get_ssh_session();
			std::string	   get_ip() const;
			std::string	   get_hostname() const;

			~ssh();
	};
}

namespace lunas {
	ssh::ssh(const struct session_data& data) {
		session_data = data;

		m_ssh = ssh_new();
		int rc;
		if (m_ssh == NULL) {
			std::string err = fmt::err_color("Failed to create ssh session to, '" + data.ip + "'");
			err += fmt::err_color(ssh_get_error(m_ssh));
			throw std::runtime_error("\n" + err);
		}

		std::string hostname = data.ip.substr(0, data.ip.find(':'));

		this->set_options(m_ssh, SSH_OPTIONS_HOST, ( void* ) hostname.c_str(), "hostname");
		this->set_options(m_ssh, SSH_OPTIONS_TIMEOUT, &session_data.options.timeout, "timeout");
		this->set_options(m_ssh, SSH_OPTIONS_PORT, &session_data.port, "port");
		this->set_options(m_ssh, SSH_OPTIONS_LOG_VERBOSITY, &session_data.options.log_level, "log level");
		if (session_data.options.compression_level > 0) {
			this->set_options(
			    m_ssh, SSH_OPTIONS_COMPRESSION_LEVEL, &session_data.options.compression_level, "compression level");
			this->set_options(m_ssh, SSH_OPTIONS_COMPRESSION, ( void* ) "yes", "compression");
		}

		rc = ssh_connect(m_ssh);
		if (rc != SSH_OK) {
			std::string err = fmt::err_color("failed to create ssh session to '" + data.ip + "'");
			err += fmt::err_color(ssh_get_error(m_ssh));
			err += fmt::err_color("exiting...");
			ssh_free(m_ssh);
			throw std::runtime_error("\n" + err);
		}

		rc = this->auth_list(m_ssh, data.ip, data.pw);
		if (rc == SSH_AUTH_SUCCESS)
			try {
				this->verify_publickey(m_ssh, data.ip);
			} catch (const std::exception& e) {
				ssh_disconnect(m_ssh);
				ssh_free(m_ssh);
				throw;
			}
		else {
			std::string err = fmt::err_color("couldn't authenticate to '" + data.ip + "'");
			err += fmt::err_color(ssh_get_error(m_ssh));
			err += fmt::err_color("exiting...");
			ssh_disconnect(m_ssh);
			ssh_free(m_ssh);
			throw std::runtime_error("\n" + err);
		}
	}

	int ssh::set_options(const ssh_session& ssh, ssh_options_e option, void* argument, const std::string& warning_type) {
		int rc = SSH_OK;
		if (rc = ssh_options_set(ssh, option, argument); rc != SSH_OK)
			llog::warn("couldn't set " + warning_type + " for '" + session_data.ip + "'");

		return rc;
	}

	const ssh_session& ssh::get_ssh_session() {
		return m_ssh;
	}

	std::string ssh::get_ip() const {
		return session_data.ip;
	}

	std::string ssh::get_hostname() const {
		return session_data.ip.substr(0, session_data.ip.find(":"));
	}

	int auth_fn(const char* prompt, char* buffer, size_t len, int echo, int verify, void* userdata) {
		const std::string* key_path = static_cast<std::string*>(userdata);
		( void ) prompt;
		std::string _prompt = "   --> enter passphrase for key '" + *key_path + "': ";
		return ssh_getpass(_prompt.c_str(), buffer, len, echo, verify);
	};

	auth_response ssh::auth_password(const ssh_session& ssh, const std::string& pw) {
		std::string password = pw;
		int	    rc	     = SSH_OK;
		int	    retry    = 3;
		while (retry >= 0) {
			retry--;
			if (password.empty() || rc == SSH_AUTH_DENIED)
				password = getpass("   --> Password: ");
			rc = ssh_userauth_password(ssh, NULL, password.c_str());
			if (rc == SSH_AUTH_DENIED) {
				llog::error("access denied. retries left: " + std::to_string(retry));
				continue;
			} else
				break;
		}
		return rc;
	}

	auth_response ssh::auth_publickey_manual(const ssh_session& ssh) {
		int rc = SSH_AUTH_DENIED;

		try {
			for (const auto& entry : fs::directory_iterator(std::string(getenv("HOME")) + "/.ssh/")) {
				if (entry.path().extension() != ".pub")
					continue;
				raii::ssh::key pubkey;

				struct ssh_key_data pubkey_data;
				{
					pubkey_data.path     = entry.path().string().c_str();
					pubkey_data.key_type = key_type_t::public_key;
				}

			try_import_publickey_again:
				rc = pubkey.import_key(pubkey_data);
				if (rc == SSH_AUTH_AGAIN) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					goto try_import_publickey_again;
				} else if (rc != SSH_OK)
					continue;

				int retries = 3;
			try_try_publickey_again:
				rc = ssh_userauth_try_publickey(ssh, NULL, pubkey.get());
				if (rc == SSH_AUTH_AGAIN && retries > 0) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					retries--;
					goto try_try_publickey_again;
				} else if (rc != SSH_AUTH_SUCCESS)
					continue;

				struct ssh_key_data privkey_data;
				{
					privkey_data.path     = entry.path().parent_path() / entry.path().stem();
					privkey_data.auth_fn  = auth_fn;
					privkey_data.key_type = key_type_t::private_key;
					privkey_data.userdata = static_cast<void*>(&privkey_data.path);
				}

				raii::ssh::key privkey;
			retry_passphrase:
				rc = privkey.import_key(privkey_data);
				if (rc != SSH_AUTH_SUCCESS && privkey.get_retry_countdown() > 0)
					goto retry_passphrase;
				else if (rc != SSH_AUTH_SUCCESS)
					continue;

				retries = 3;
			ssh_try_publickey_again:
				rc = ssh_userauth_publickey(ssh, NULL, privkey.get());
				if (rc == SSH_AUTH_SUCCESS) {
					break;
				} else if (rc == SSH_AUTH_AGAIN && retries > 0) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					retries--;
					goto ssh_try_publickey_again;
				} else
					return rc;
			}
		} catch (const std::exception& e) {
			llog::warn(e.what());
			rc = SSH_AUTH_DENIED;
		}

		return rc;
	}

	auth_response ssh::auth_none(const ssh_session& ssh) {
		return ssh_userauth_none(ssh, NULL);
	}

	std::string ssh::auth_method(const int& method) {
		std::string method_str;
		if (method & SSH_AUTH_METHOD_HOSTBASED)
			method_str += "hostbased, ";
		if (method & SSH_AUTH_METHOD_INTERACTIVE)
			method_str += "keyboard-interactive, ";
		if (method & SSH_AUTH_METHOD_GSSAPI_MIC)
			method_str += "gssapi-mic";

		return method_str;
	}

	auth_response ssh::auth_list(const ssh_session& ssh, const std::string& ip, const std::string& pw) {
		int rc = ssh_userauth_none(ssh, NULL);
		if (rc == SSH_AUTH_SUCCESS || rc == SSH_AUTH_ERROR)
			return rc;

		int method = ssh_userauth_list(ssh, NULL);
		if (rc != SSH_AUTH_SUCCESS && method & SSH_AUTH_METHOD_NONE)
			rc = auth_none(ssh);

		if (rc != SSH_AUTH_SUCCESS)
			std::println("--> authenticating: '{}'", ip);

		if (rc != SSH_AUTH_SUCCESS && method & SSH_AUTH_METHOD_PUBLICKEY)
			rc = this->auth_publickey_manual(ssh);

		if (rc != SSH_AUTH_SUCCESS && method & SSH_AUTH_METHOD_PASSWORD)
			rc = this->auth_password(ssh, pw);

		if (rc == SSH_AUTH_SUCCESS)
			return rc;
		else if (method & SSH_AUTH_METHOD_INTERACTIVE || method & SSH_AUTH_METHOD_GSSAPI_MIC ||
			 method & SSH_AUTH_METHOD_HOSTBASED) {
			llog::error("unsupported auth method for '" + ip + "', " + auth_method(method));
		}

		return SSH_AUTH_ERROR;
	}

	auth_response ssh::verify_publickey(const ssh_session& ssh, const std::string& ip) {
		ssh_known_hosts_e response = ssh_session_is_known_server(ssh);
		std::string	  err;
		switch (response) {
			case SSH_KNOWN_HOSTS_OK:
				break;
			case SSH_KNOWN_HOSTS_CHANGED:
				err = fmt::err_color("the server key for '" + ip +
						     "' has changed! either you are under attack or the admin changed the key");

				throw std::runtime_error("\n" + err);
			case SSH_KNOWN_HOSTS_OTHER:
				err = fmt::err_color(
				    "-[X] the server for '" + ip + "' gave a key of type while we had another type! possible attack!");
				throw std::runtime_error("\n" + err);
			case SSH_KNOWN_HOSTS_ERROR:
				err = fmt::err_color(
				    "error while checking the server '" + ip + "', is the publickey valid? " + ssh_get_error(ssh));
				throw std::runtime_error("\n" + err);
			case SSH_KNOWN_HOSTS_UNKNOWN:
			case SSH_KNOWN_HOSTS_NOT_FOUND:
				std::string ok;
				int	    rc;
				llog::warn2("the autheticity of server '" + ip + "' can't be established\n");
				llog::warn2("the publickey is not found, do you want to add it to '~/.ssh/known_hosts'? (y/n): ");
				std::cin >> ok;

				if (ok == "y")
					rc = ssh_session_update_known_hosts(ssh);
				else {
					err = fmt::err_color("server key verification failed");
					throw std::runtime_error("\n" + err);
				}
				if (ok == "y" && rc != SSH_OK) {
					err = fmt::err_color("couldn't add the key to the '~/.ssh/known_hosts'");
					err += fmt::err_color(ssh_get_error(ssh));
					throw std::runtime_error("\n" + err);
				}
				break;
		}
		return 0;
	}

	ssh::~ssh() {
		if (m_ssh != NULL)
			ssh_free(m_ssh);
	}
}
