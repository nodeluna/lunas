#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <libssh/sftp.h>
#include "fs_remote.h"
#include "base.h"
#include "raii_sftp.h"
#include "file_types.h"
#include <cstring>


namespace fs_remote {
	void readdir(const struct input_path& remote_path, const std::string& dir_path, const unsigned long int& index_path){
		sftp_dir dir = sftp_opendir(remote_path.sftp, dir_path.c_str());
		if(dir == NULL)
			llog::remote_error_ssh(remote_path.sftp->session, dir_path, "directory wasn't opened", EXIT_FAILURE);
		raii::sftp::dir dir_obj = raii::sftp::dir(&dir, dir_path);

		sftp_attributes attributes = NULL;
		while((attributes = sftp_readdir(remote_path.sftp, dir)) != NULL){
			std::unique_ptr<raii::sftp::attributes> attr_obj = std::make_unique<raii::sftp::attributes>(&attributes);

			struct metadata metadata;
			metadata.type = (short int)(attributes->type);
			metadata.mtime = attributes->mtime;

			std::string full_path;
			{
			std::string file_name = attributes->name;
			if(file_name == ".." || file_name == ".")
				continue;

			full_path = dir_path + file_name;
			}
			std::string relative_path = full_path.substr(remote_path.path.size(), full_path.size());

			auto itr = content.find(path(relative_path, metadata, index_path));
			if(itr != content.end())
				itr->metadatas[index_path] = metadata;
			else
				content.insert(path(relative_path, metadata, index_path));

			if(status::remote_type(attributes) == DIRECTORY){
				attr_obj.reset();
				os::append_seperator(full_path);
				readdir(remote_path, full_path, index_path);
			}
		}
		if(!sftp_dir_eof(dir))
			llog::remote_error_ssh(remote_path.sftp->session, "couldn't reach the end of directory", remote_path.path, EXIT_FAILURE);
	}

	void list_tree(struct input_path& remote_path, const unsigned long int& index_path){
		sftp_attributes attributes = sftp_lstat(remote_path.sftp, remote_path.path.c_str());
		std::unique_ptr<raii::sftp::attributes> attr_obj = std::make_unique<raii::sftp::attributes>(&attributes);
		if(attributes == NULL && sftp_get_error(remote_path.sftp) == SSH_FX_NO_SUCH_FILE){
			if(options::mkdir){
				int rc = sftp::mkdir(remote_path.sftp, remote_path.path);
				llog::rc(remote_path.sftp, remote_path.path, rc, "couldn't make input directory", EXIT_FAILURE);
				llog::print("-[!] created input directory '" + remote_path.path + "', it was not found");
				os::append_seperator(remote_path.path);
			}else {
				llog::error("input directory of '" + remote_path.ip + "' doesn't exist. use -mkdir to create it");
				exit(1);
			}
		}else if(status::remote_type(attributes) != DIRECTORY){
			llog::error("input path of '" + remote_path.ip + "' isn't a directory");
			exit(1);
		}
		attr_obj.reset();

		fs_remote::readdir(remote_path, remote_path.path, index_path);
	}
}

#endif // REMOTE_ENABLED
