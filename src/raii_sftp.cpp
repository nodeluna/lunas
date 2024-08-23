#include "config.h"

#ifdef REMOTE_ENABLED

#include <iostream>
#include <libssh/sftp.h>
#include <fcntl.h>
#include "raii_sftp.h"
#include "log.h"

namespace raii{
	namespace sftp{
		session::session(sftp_session* sftp_) : _sftp(sftp_){
		}

		session::~session(){
			if(*_sftp == NULL)
				return;
			sftp_free(*_sftp);
		}

		channel::channel(ssh_channel* channel_) : _channel(channel_){
		}

		channel::~channel(){
			if(*_channel == NULL)
				return;
			ssh_channel_close(*_channel);
			ssh_channel_free(*_channel);
		}


		attributes::attributes(sftp_attributes* attr) : _attributes(attr){
		}

		attributes::~attributes(){
			if(*_attributes == NULL)
				return;
			sftp_attributes_free(*_attributes);
		}
		
		dir::dir(sftp_dir* dir, const std::string& path) : _dir(dir), _path(path){
		}

		dir::~dir(){
			if(*_dir == NULL)
				return;

			int rc = sftp_closedir(*_dir);
			if (rc == SSH_OK)
				return;

			llog::error("Couldn't close dir '" +  _path + "', " + ssh_get_error((*_dir)->sftp->session));
		}

		file::file(sftp_file* file, const std::string& path) : _file(file), _path(path){
		}

		file::~file(){
			if(*_file == NULL)
				return;

			int rc = sftp_close(*_file);
			if (rc == SSH_OK)
				return;

			llog::error("-[X] Couldn't close file '" +  _path + "', " + ssh_get_error((*_file)->sftp->session));
		}

		link_target::link_target(char** target_) : target(target_){
		}

		link_target::~link_target(){
			if(target == NULL)
				return;
			ssh_string_free_char(*target);
		}
		statvfs_t::statvfs_t(sftp_statvfs_t* partition_stats) : _partition_stats(partition_stats) {

		};
		statvfs_t::~statvfs_t(){
			if(*_partition_stats == NULL)
				return;
			sftp_statvfs_free(*_partition_stats);
		}
	}
}

namespace sftp{
	/*int unlink(const sftp_session& sftp, const std::string& path){
		if(options::dry_run == false)
			return sftp_unlink(sftp, path.c_str());
		return SSH_OK;
	}
	int rmdir(const sftp_session& sftp, const std::string& path){
		if(options::dry_run == false)
			return sftp_rmdir(sftp, path.c_str());
		return SSH_OK;
	}*/
	int mkdir(const sftp_session& sftp, const std::string& path){
		if(options::dry_run == false)
			return sftp_mkdir(sftp, path.c_str(),  S_IRWXU);
		return SSH_OK;
	}

	int symlink(const sftp_session& sftp, const std::string& target, const std::string& path){
		if(options::dry_run == false)
			return sftp_symlink(sftp, target.c_str(), path.c_str());
		return SSH_OK;
	}
	sftp_attributes attributes(const sftp_session& sftp, const std::string& path){
		sftp_attributes attributes;
		if(options::follow_symlink)
			attributes = sftp_stat(sftp, path.c_str());
		else
			attributes = sftp_lstat(sftp, path.c_str());
		return attributes;
	}

	std::string cmd(const ssh_session& ssh, const std::string& command, const std::string& ip){
		std::string output;
		ssh_channel channel = ssh_channel_new(ssh);
		int rc = ssh_channel_open_session(channel);
		if(rc != SSH_OK){
			llog::remote_error_ssh(ssh, ip, "couldn't excute '" + command + "'", NO_EXIT);
			return "";
		}
		raii::sftp::channel channel_obj = raii::sftp::channel(&channel);

		rc = ssh_channel_request_exec(channel, command.c_str());
		if(rc != SSH_OK){
			llog::remote_error_ssh(ssh, ip, "couldn't excute '" + command + "'", NO_EXIT);
			return "";
		}

		char buffer[REMOTE_BUFFER_SIZE];
		while(true){
			int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
			if(nbytes < 0){
				llog::remote_error_ssh(ssh, ip, "error while getting '" + command + "' output", NO_EXIT);
				return "";
			}
			if(nbytes == 0)
				break;
			output.append(buffer, nbytes);
		}

		if(output.empty() != true && output.back() == '\n')
			output.pop_back();

		return output;
	}
	
	std::string homedir(const ssh_session& ssh, const std::string& ip){
		std::string path = sftp::cmd(ssh, "echo $HOME", ip);
		if(path.empty())
			llog::error("couldn't get home directory for '" + ip + "' environment variable is empty");
		os::append_seperator(path);
		return path;
	}

	std::string cwd(const ssh_session& ssh, const std::string& ip){
		std::string path = sftp::cmd(ssh, "pwd", ip);
		if(path.empty())
			llog::error("couldn't get cwd directory for '" + ip + "'");
		os::append_seperator(path);
		return path;
	}
}
#endif // REMOTE_ENABLED
