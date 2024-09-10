#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <iostream>
#include <libssh/sftp.h>
#include "log.h"
#include "raii_sftp.h"
#include "path_parsing.h"
#include "file_types.h"
#include "remote_session.h"


#define SSH_AUTH_METHOD_PUBLICKEY_COMBO_1 (SSH_AUTH_METHOD_PUBLICKEY)
#define SSH_AUTH_METHOD_PUBLICKEY_COMBO_2 (SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_HOSTBASED)
#define SSH_AUTH_METHOD_PUBLICKEY_COMBO_3 (SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE)
#define SSH_AUTH_METHOD_PUBLICKEY_COMBO_4 (SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_GSSAPI_MIC) 
#define SSH_AUTH_METHOD_PUBLICKEY_COMBO_5 (SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE | SSH_AUTH_METHOD_GSSAPI_MIC)
#define SSH_AUTH_METHOD_PUBLICKEY_COMBO_6 (SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE | SSH_AUTH_METHOD_HOSTBASED)
#define SSH_AUTH_METHOD_PUBLICKEY_COMBO_7 (SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_GSSAPI_MIC | SSH_AUTH_METHOD_HOSTBASED)
#define SSH_AUTH_METHOD_PUBLICKEY_COMBO_8 (SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE | SSH_AUTH_METHOD_HOSTBASED | SSH_AUTH_METHOD_GSSAPI_MIC)

#define SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_1 (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY)
#define SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_2 (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_HOSTBASED)
#define SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_3 (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE)
#define SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_4 (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_GSSAPI_MIC)
#define SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_5 (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE | SSH_AUTH_METHOD_GSSAPI_MIC)
#define SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_6 (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_GSSAPI_MIC | SSH_AUTH_METHOD_HOSTBASED)
#define SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_7 (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE |SSH_AUTH_METHOD_HOSTBASED)
#define SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_8 (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE |SSH_AUTH_METHOD_HOSTBASED | SSH_AUTH_METHOD_GSSAPI_MIC)

namespace rsession {
	int verify_publickey(const ssh_session& ssh, const std::string& ip){
		ssh_known_hosts_e response = ssh_session_is_known_server(ssh);
		switch(response){
			case SSH_KNOWN_HOSTS_OK:
				break;
			case SSH_KNOWN_HOSTS_CHANGED:
				llog::error("the server key for '" + ip + "' has changed! either you are under attack or the admin changed the key");
				exit(1);
			case SSH_KNOWN_HOSTS_OTHER:
				llog::error("-[X] the server for '" + ip + "' gave a key of type while we had another type! possible attack!");
				exit(1);
			case SSH_KNOWN_HOSTS_ERROR:
				llog::error("error while checking the server '" + ip + "', is the publickey valid? " + ssh_get_error(ssh));
				exit(1);
			case SSH_KNOWN_HOSTS_UNKNOWN:
			case SSH_KNOWN_HOSTS_NOT_FOUND:
				std::string ok;
				int rc;
				llog::warn("the autheticity of server '" + ip + "' can't be established\n");
				llog::warn("the publickey is not found, do you want to add it to '~/.ssh/known_hosts'? (y/n): ");
				std::cin >> ok;

				if(ok == "y")
					rc = ssh_session_update_known_hosts(ssh);
				else{
					llog::error("server key verification failed");
					exit(1);
				}
				if(ok == "y" && rc != SSH_OK){
					llog::error("couldn't add the key to the '~/.ssh/known_hosts'");
					llog::error(ssh_get_error(ssh));
					exit(1);
				}
			break;
		}
		return 0;
	}

	int auth_password(const ssh_session& ssh, const std::string& ip, const std::string& pw){
		std::string password = pw;
		if(password.empty()){
			llog::print("--> login for: '" + ip + "'");
			password = getpass("   --> Password: ");
		}
		return ssh_userauth_password(ssh, NULL, password.c_str());
	}

	int auth_publickey(const ssh_session& ssh, const char* password){
		return ssh_userauth_publickey_auto(ssh, NULL, password);
	}

	int auth_publickey_passphrase(const ssh_session& ssh, const std::string& ip, const std::string& pw){
		std::string password = pw;
		if(password.empty()){
			llog::print("--> login for: '" + ip + "'");
			password = getpass("   --> private-key passphrase: ");
		}
		return rsession::auth_publickey(ssh, password.c_str());
	}

	int auth_none(const ssh_session& ssh){
		return ssh_userauth_none(ssh, NULL);
	}

	std::string auth_method(const int& method){
		switch(method){
			case SSH_AUTH_METHOD_NONE:
				return "none";
			case SSH_AUTH_METHOD_PASSWORD:
				return "password";
			case SSH_AUTH_METHOD_PUBLICKEY:
				return "publickey";
			case SSH_AUTH_METHOD_HOSTBASED:
				return "hostbased";
			case SSH_AUTH_METHOD_INTERACTIVE:
				return "keyboard-interactive";
			case SSH_AUTH_METHOD_GSSAPI_MIC:
				return "gssapi-mic";
			case SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY:
				return "publickey, password";
			case SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_INTERACTIVE:
				return "password, keyboard-interactive";
			case SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE:
				return "publickey, keyboard-interactive";
			case SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE:
				return "publickey, password, keyboard-interactive";
			case SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY | SSH_AUTH_METHOD_INTERACTIVE | SSH_AUTH_METHOD_GSSAPI_MIC:
				return "publickey, password, keyboard-interactive, gssapi-mic";
			default:
				return "unknown method";
		}
		return "unknown method: " + method;
	}

	int auth_list(const ssh_session& ssh, const std::string& ip, const std::string& pw){
		int rc = ssh_userauth_none(ssh, NULL);
		if(rc == SSH_AUTH_SUCCESS || rc == SSH_AUTH_ERROR)
			return rc;

		int method = ssh_userauth_list(ssh, NULL);
		switch(method){
			case SSH_AUTH_METHOD_NONE:
				rc = rsession::auth_none(ssh);
				break;
			case SSH_AUTH_METHOD_PUBLICKEY_COMBO_1:
			case SSH_AUTH_METHOD_PUBLICKEY_COMBO_2:
			case SSH_AUTH_METHOD_PUBLICKEY_COMBO_3:
			case SSH_AUTH_METHOD_PUBLICKEY_COMBO_4:
			case SSH_AUTH_METHOD_PUBLICKEY_COMBO_5:
			case SSH_AUTH_METHOD_PUBLICKEY_COMBO_6:
			case SSH_AUTH_METHOD_PUBLICKEY_COMBO_7:
			case SSH_AUTH_METHOD_PUBLICKEY_COMBO_8:
				rc = rsession::auth_publickey(ssh, NULL);
				if(rc == SSH_AUTH_DENIED)
					rc = rsession::auth_publickey_passphrase(ssh, ip, pw);
				break;
			case SSH_AUTH_METHOD_PASSWORD:
				rc = rsession::auth_password(ssh, ip, pw);
				break;
			case SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_1: 
			case SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_2:
			case SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_3:
			case SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_4:
			case SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_5:
			case SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_6:
			case SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_7:
			case SSH_AUTH_METHOD_PW_PUBLICKEY_COMBO_8:				
				rc = rsession::auth_publickey(ssh, NULL);
				if(rc == SSH_AUTH_SUCCESS)
					return rc;
				else if(rc == SSH_AUTH_DENIED){
					rc = rsession::auth_publickey_passphrase(ssh, ip, pw);
					if(rc == SSH_AUTH_SUCCESS)
						break;
					llog::error(ssh_get_error(ssh));
				}
				rc = rsession::auth_password(ssh, ip, pw);
				break;
			default:
				llog::error("unsupported auth method for '" + ip + "', " + rsession::auth_method(method));
				llog::error(ssh_get_error(ssh));
				llog::error("exiting...");
				exit(1);
		}

		if(rc == SSH_AUTH_SUCCESS)
			return rc;

		return SSH_AUTH_ERROR;
	}

	ssh_session init_ssh(const std::string& ip, const int& port, const std::string& pw){
		ssh_session ssh = ssh_new();
		int rc;
		if(ssh == NULL){
			llog::error("Failed to create ssh session to, '" + ip + "'");
			llog::error("exiting...");
			exit(1);
		}
		std::string hostname = ip.substr(0, ip.find(':'));
		ssh_options_set(ssh, SSH_OPTIONS_HOST, hostname.c_str());
		ssh_options_set(ssh, SSH_OPTIONS_PORT, &port);
		if(options::compression){
			if(ssh_options_set(ssh, SSH_OPTIONS_COMPRESSION, "yes") != SSH_OK)
				llog::warn("couldn't set compression for '" + ip + "'");
			if(ssh_options_set(ssh, SSH_OPTIONS_COMPRESSION_LEVEL, &options::compression_level) != SSH_OK)
				llog::warn("couldn't set compression level for '" + ip + "'");
		}
 
		rc = ssh_connect(ssh);
		if (rc != SSH_OK){
			llog::error("failed to create ssh session to '" + ip + "'");
			llog::error(ssh_get_error(ssh));
			llog::error("exiting...");
			ssh_free(ssh);
			exit(1);
		}

		rc = rsession::auth_list(ssh, ip, pw);
		if(rc == SSH_AUTH_SUCCESS)
			rsession::verify_publickey(ssh, ip);
		else{
			llog::error("couldn't authenticate to '" + ip + "'");
			llog::error(ssh_get_error(ssh));
			llog::error("exiting...");
			ssh_disconnect(ssh);
			ssh_free(ssh);
			exit(1);
		}

		return ssh;
	}

	sftp_session init_sftp(const ssh_session& ssh, const std::string& ip){
		sftp_session sftp = sftp_new(ssh);
		int rc;

		if (sftp == NULL){
			llog::error("failed to create an sftp session to '" + ip + "'");
			llog::error(ssh_get_error(ssh));
			llog::error("If you are using fish shell try bash");
			exit(1);
		}

		rc = sftp_init(sftp);
		if (rc != SSH_OK){
			llog::error("failed to initialize an sftp session to '" + ip + "'");
			llog::error(ssh_get_error(ssh));
			llog::error("If you are using fish shell try bash");
			exit(1);
		}

		return sftp;
	}
	std::string absolute_path(const sftp_session& sftp, const std::string& ip){
		if(ip.find(':') == ip.npos || ip.back() == ':'){
			llog::error("hostname: " + ip + " doesn't include an input path");
			exit(1);
		}
		std::string sftp_path = ip.substr(ip.find(":")+1, ip.size());
		std::expected<std::string, SSH_STATUS> path;

		if(sftp_path.size() > 1 && sftp_path.substr(0, 2) == "~/"){
			path = sftp::homedir(sftp->session, ip);
			if(not path)
				llog::rc(sftp, ip, path.error(), "couldn't get homedir", EXIT_FAILURE);
			sftp_path = path.value() + sftp_path.substr(2, sftp_path.size());

		}else if(sftp_path.size() > 1 && sftp_path.front() != path_seperator && sftp_path.front() != '.'){
			path = sftp::homedir(sftp->session, ip);
			if(not path)
				llog::rc(sftp, ip, path.error(), "couldn't get homedir", EXIT_FAILURE);
			sftp_path = path.value() + sftp_path;
		}

		if(options::follow_symlink){
			char* full_path = sftp_canonicalize_path(sftp, sftp_path.c_str());
			if(full_path != NULL){
				sftp_path = full_path;
				ssh_string_free_char(full_path);
			}
		}else if(sftp_path.size() > 2 && sftp_path.substr(0, 3) == "../"){
			int depth = 0;
			auto current_path = sftp::cwd(sftp->session, ip);
			if(not current_path)
				llog::rc(sftp, ip, path.error(), "couldn't get cwd", EXIT_FAILURE);
			os::pop_seperator(current_path.value());
			parse_path::adjust_relative_path(sftp_path, depth);
			parse_path::append_to_relative_path(sftp_path, current_path.value(), depth);
		}
		sftp_attributes attributes = sftp::attributes(sftp, sftp_path);
		if(status::remote_type(attributes) == DIRECTORY)
			os::append_seperator(sftp_path);
		if(attributes != NULL)
			sftp_attributes_free(attributes);
		return sftp_path;
	}
	int free_sftp(sftp_session& sftp){
		if(sftp != NULL)
			sftp_free(sftp);
		return 0;
	}
	int free_ssh(ssh_session& ssh){
		if(ssh != NULL){
			ssh_disconnect(ssh);
			ssh_free(ssh);
		}
		return 0;
	}
}

#endif // REMOTE_ENABLED
