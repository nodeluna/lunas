#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <iostream>
#include <libssh/sftp.h>
#include "log.h"
#include "raii_sftp.h"
#include "path_parsing.h"


namespace rsession {
	int verify_publickey(ssh_session& ssh, const std::string& ip){
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

	ssh_session init_ssh(const std::string& ip, const int& port, const std::string& pw){
		ssh_session ssh = ssh_new();
		int rc;
		if(ssh == NULL){
			llog::error("Failed to create ssh session to, '" + ip + "'");
			llog::error("exiting...");
			exit(1);
		}
		ssh_options_set(ssh, SSH_OPTIONS_HOST, ip.c_str());
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

		rc = ssh_userauth_publickey_auto(ssh, NULL, NULL);
		if (rc == SSH_AUTH_SUCCESS){
			rsession::verify_publickey(ssh, ip);
		}else{
			std::string password = pw;
			if(password.size() == 0){
				llog::print("--> login for: '" + ip + "'");
				password = getpass("   --> Password: ");
			}
			rc = ssh_userauth_password(ssh, NULL, password.c_str());
			if (rc != SSH_AUTH_SUCCESS){
				llog::error("Couldn't Authenticate");
				llog::error(ssh_get_error(ssh));
				ssh_disconnect(ssh);
				ssh_free(ssh);
				exit(1);
			}
			rsession::verify_publickey(ssh, ip);
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

		if(sftp_path.size() > 1 && sftp_path.substr(0, 2) == "~/")
			sftp_path = sftp::homedir(sftp->session, ip) + sftp_path.substr(2, sftp_path.size());
		else if(sftp_path.size() > 1 && sftp_path.front() != path_seperator)
			sftp_path = sftp::homedir(sftp->session, ip) + sftp_path;

		if(options::follow_symlink){
			char* full_path = sftp_canonicalize_path(sftp, sftp_path.c_str());
			if(full_path != NULL){
				sftp_path = full_path;
				ssh_string_free_char(full_path);
			}
		}else if(sftp_path.size() > 2 && sftp_path.substr(0, 3) == "../"){
			int depth = 0;
			std::string current_path = sftp::cwd(sftp->session, ip);
			os::pop_seperator(current_path);
			parse_path::adjust_relative_path(sftp_path, depth);
			parse_path::append_to_relative_path(sftp_path, current_path, depth);
		}
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
