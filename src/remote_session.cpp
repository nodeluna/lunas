#include "config.h"

#ifdef REMOTE_ENABLED

#	include <string>
#	include <iostream>
#	include <filesystem>
#	include <thread>
#	include <chrono>
#	include <libssh/sftp.h>
#	include <libssh/libssh.h>
#	include "log.h"
#	include "raii_sftp.h"
#	include "path_parsing.h"
#	include "file_types.h"
#	include "remote_session.h"

namespace fs = std::filesystem;

namespace rsession {
	int verify_publickey(const ssh_session& ssh, const std::string& ip) {
		ssh_known_hosts_e response = ssh_session_is_known_server(ssh);
		switch (response) {
			case SSH_KNOWN_HOSTS_OK:
				break;
			case SSH_KNOWN_HOSTS_CHANGED:
				llog::error("the server key for '" + ip +
					    "' has changed! either you are under attack or the admin changed the key");
				exit(1);
			case SSH_KNOWN_HOSTS_OTHER:
				llog::error(
				    "-[X] the server for '" + ip + "' gave a key of type while we had another type! possible attack!");
				exit(1);
			case SSH_KNOWN_HOSTS_ERROR:
				llog::error("error while checking the server '" + ip + "', is the publickey valid? " + ssh_get_error(ssh));
				exit(1);
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
					llog::error("server key verification failed");
					exit(1);
				}
				if (ok == "y" && rc != SSH_OK) {
					llog::error("couldn't add the key to the '~/.ssh/known_hosts'");
					llog::error(ssh_get_error(ssh));
					exit(1);
				}
				break;
		}
		return 0;
	}

	int auth_password(const ssh_session& ssh, const std::string& pw) {
		std::string password = pw;
		int	    rc	     = SSH_OK;
		int	    retry    = 3;
		while (retry > 0) {
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

	int auth_none(const ssh_session& ssh) {
		return ssh_userauth_none(ssh, NULL);
	}

	std::string auth_method(const int& method) {
		std::string method_str;
		if (method & SSH_AUTH_METHOD_HOSTBASED)
			method_str += "hostbased, ";
		if (method & SSH_AUTH_METHOD_INTERACTIVE)
			method_str += "keyboard-interactive, ";
		if (method & SSH_AUTH_METHOD_GSSAPI_MIC)
			method_str += "gssapi-mic";

		return method_str;
	}

	int auth_fn(const char* prompt, char* buffer, size_t len, int echo, int verify, void* userdata) {
		const std::string* key_path = static_cast<std::string*>(userdata);
		( void ) prompt;
		std::string _prompt = "   --> enter passphrase for key '" + *key_path + "': ";
		return ssh_getpass(_prompt.c_str(), buffer, len, echo, verify);
	};

	int auth_publickey_manual(const ssh_session& ssh) {
		int rc = 0;

		try {
			for (const auto& entry : fs::directory_iterator(std::string(getenv("HOME")) + "/.ssh/")) {
				if (entry.path().extension() == ".pub") {
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
			}
		} catch (const std::exception& e) {
			llog::warn(e.what());
			rc = SSH_AUTH_DENIED;
		}

		return rc;
	}

	int auth_list(const ssh_session& ssh, const std::string& ip, const std::string& pw) {
		int rc = ssh_userauth_none(ssh, NULL);
		if (rc == SSH_AUTH_SUCCESS || rc == SSH_AUTH_ERROR)
			return rc;

		int method = ssh_userauth_list(ssh, NULL);
		if (rc != SSH_AUTH_SUCCESS && method & SSH_AUTH_METHOD_NONE)
			rc = rsession::auth_none(ssh);

		if (rc != SSH_AUTH_SUCCESS)
			llog::print(std::string("--> authenticating: '") + ip + std::string("'"));

		if (rc != SSH_AUTH_SUCCESS && method & SSH_AUTH_METHOD_PUBLICKEY)
			rc = rsession::auth_publickey_manual(ssh);

		if (rc != SSH_AUTH_SUCCESS && method & SSH_AUTH_METHOD_PASSWORD)
			rc = rsession::auth_password(ssh, pw);

		if (rc == SSH_AUTH_SUCCESS)
			return rc;
		else if (method & SSH_AUTH_METHOD_INTERACTIVE || method & SSH_AUTH_METHOD_GSSAPI_MIC ||
			 method & SSH_AUTH_METHOD_HOSTBASED) {
			llog::error("unsupported auth method for '" + ip + "', " + rsession::auth_method(method));
			llog::error(ssh_get_error(ssh));
			llog::error("exiting...");
			exit(1);
		}

		return SSH_AUTH_ERROR;
	}

	ssh_session init_ssh(const std::string& ip, const int& port, const std::string& pw) {
		ssh_session ssh = ssh_new();
		int	    rc;
		if (ssh == NULL) {
			llog::error("Failed to create ssh session to, '" + ip + "'");
			llog::error("exiting...");
			exit(1);
		}
		std::string hostname = ip.substr(0, ip.find(':'));

		int log = SSH_LOG_NOLOG;
		if (options::ssh_log_level == ssh_log_level::no_log)
			log = SSH_LOG_NOLOG;
		else if (options::ssh_log_level == ssh_log_level::warning)
			log = SSH_LOG_WARNING;
		else if (options::ssh_log_level == ssh_log_level::protocol)
			log = SSH_LOG_PROTOCOL;
		else if (options::ssh_log_level == ssh_log_level::packet)
			log = SSH_LOG_PACKET;
		else if (options::ssh_log_level == ssh_log_level::functions)
			log = SSH_LOG_FUNCTIONS;

		ssh_options_set(ssh, SSH_OPTIONS_HOST, hostname.c_str());
		ssh_options_set(ssh, SSH_OPTIONS_PORT, &port);
		ssh_options_set(ssh, SSH_OPTIONS_LOG_VERBOSITY, &log);
		if (options::compression) {
			if (ssh_options_set(ssh, SSH_OPTIONS_COMPRESSION, "yes") != SSH_OK)
				llog::warn("couldn't set compression for '" + ip + "'");
			if (ssh_options_set(ssh, SSH_OPTIONS_COMPRESSION_LEVEL, &options::compression_level) != SSH_OK)
				llog::warn("couldn't set compression level for '" + ip + "'");
		}

		rc = ssh_connect(ssh);
		if (rc != SSH_OK) {
			llog::error("failed to create ssh session to '" + ip + "'");
			llog::error(ssh_get_error(ssh));
			llog::error("exiting...");
			ssh_free(ssh);
			exit(1);
		}

		rc = rsession::auth_list(ssh, ip, pw);
		if (rc == SSH_AUTH_SUCCESS)
			rsession::verify_publickey(ssh, ip);
		else {
			llog::error("couldn't authenticate to '" + ip + "'");
			llog::error(ssh_get_error(ssh));
			llog::error("exiting...");
			ssh_disconnect(ssh);
			ssh_free(ssh);
			exit(1);
		}

		return ssh;
	}

	sftp_session init_sftp(const ssh_session& ssh, const std::string& ip) {
		sftp_session sftp = sftp_new(ssh);
		int	     rc;

		if (sftp == NULL) {
			llog::error("failed to create an sftp session to '" + ip + "'");
			llog::error(ssh_get_error(ssh));
			llog::error("If you are using fish shell try bash");
			exit(1);
		}

		rc = sftp_init(sftp);
		if (rc != SSH_OK) {
			llog::error("failed to initialize an sftp session to '" + ip + "'");
			llog::error(ssh_get_error(ssh));
			llog::error("If you are using fish shell try bash");
			exit(1);
		}

		return sftp;
	}

	std::string absolute_path(const sftp_session& sftp, const std::string& ip) {
		if (ip.find(':') == ip.npos || ip.back() == ':') {
			llog::error("hostname: " + ip + " doesn't include an input path");
			exit(1);
		}
		std::string			       sftp_path = ip.substr(ip.find(":") + 1, ip.size());
		std::expected<std::string, SSH_STATUS> path;

		if (sftp_path.size() > 1 && sftp_path.substr(0, 2) == "~/") {
			path = sftp::homedir(sftp->session, ip);
			if (not path)
				llog::rc(sftp, ip, path.error(), "couldn't get homedir", EXIT_FAILURE);
			sftp_path = path.value() + sftp_path.substr(2, sftp_path.size());

		} else if (sftp_path.size() > 1 && sftp_path.front() != path_seperator && sftp_path.front() != '.') {
			path = sftp::homedir(sftp->session, ip);
			if (not path)
				llog::rc(sftp, ip, path.error(), "couldn't get homedir", EXIT_FAILURE);
			sftp_path = path.value() + sftp_path;
		}

		if (options::follow_symlink) {
			char* full_path = sftp_canonicalize_path(sftp, sftp_path.c_str());
			if (full_path != NULL) {
				sftp_path = full_path;
				ssh_string_free_char(full_path);
			}
		} else if (sftp_path.size() > 2 && sftp_path.substr(0, 3) == "../") {
			int  depth	  = 0;
			auto current_path = sftp::cwd(sftp->session, ip);
			if (not current_path)
				llog::rc(sftp, ip, path.error(), "couldn't get cwd", EXIT_FAILURE);
			os::pop_seperator(current_path.value());
			parse_path::adjust_relative_path(sftp_path, depth);
			parse_path::append_to_relative_path(sftp_path, current_path.value(), depth);
		}
		sftp_attributes attributes = sftp::attributes(sftp, sftp_path);
		if (status::remote_type(attributes) == DIRECTORY)
			os::append_seperator(sftp_path);
		if (attributes != NULL)
			sftp_attributes_free(attributes);
		return sftp_path;
	}

	int free_sftp(sftp_session& sftp) {
		if (sftp != NULL)
			sftp_free(sftp);
		return 0;
	}

	int free_ssh(ssh_session& ssh) {
		if (ssh != NULL) {
			ssh_disconnect(ssh);
			ssh_free(ssh);
		}
		return 0;
	}
}

#endif // REMOTE_ENABLED
