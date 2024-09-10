#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <cstring>
#include <fcntl.h>
#include <libssh/sftp.h>
#include "fs_remote.h"
#include "base.h"
#include "raii_sftp.h"
#include "file_types.h"
#include "resume.h"
#include "exclude.h"
#include "utimes.h"

namespace remote_readdir_operations {
	void type(const struct input_path& remote_path, const sftp_attributes& attributes, struct metadata& metadata, const std::string& full_path);

	void mtime(const struct input_path& remote_path, const sftp_attributes& attributes, struct metadata& metadata, const std::string& full_path);

	void type_brokenlink(const struct input_path& remote_path, struct metadata& metadata, const std::string& full_path);

	bool lspart(const struct input_path& remote_path, const struct metadata& metadata, const std::string& full_path, const std::string& relative_path,
			const std::string& file_name, const unsigned long int& index_path);

	void insert(const struct metadata& metadata, const std::string& relative_path, const unsigned long int& index_path);
}


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

			std::string full_path;
			{
			std::string relative_path;
			std::string file_name = attributes->name;
			if(file_name == ".." || file_name == ".")
				continue;

			full_path = dir_path + file_name;
			if(utils::exclude(full_path, remote_path.path))
					continue;

			remote_readdir_operations::type(remote_path, attributes, metadata, full_path);
			remote_readdir_operations::mtime(remote_path, attributes, metadata, full_path);

			relative_path = full_path.substr(remote_path.path.size(), full_path.size());

			if(remote_readdir_operations::lspart(remote_path, metadata, full_path, relative_path, file_name, index_path))
				continue;
			remote_readdir_operations::insert(metadata, relative_path, index_path);
			}

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
		sftp_attributes attributes = sftp_stat(remote_path.sftp, remote_path.path.c_str());
		std::unique_ptr<raii::sftp::attributes> attr_obj = std::make_unique<raii::sftp::attributes>(&attributes);
		if(attributes == NULL && sftp_get_error(remote_path.sftp) == SSH_FX_NO_SUCH_FILE){
			if(options::mkdir){
				int rc = sftp::mkdir(remote_path.sftp, remote_path.path, S_IRWXU);
				llog::rc(remote_path.sftp, remote_path.path, rc, "couldn't make input directory", EXIT_FAILURE);
				llog::print("-[!] created input directory '" + remote_path.path + "', it was not found");
				os::append_seperator(remote_path.path);
				if(options::dry_run)
					return;
			}else {
				llog::error("input directory of '" + remote_path.ip + "' doesn't exist. use -mkdir to create it");
				exit(1);
			}
		}else if(status::remote_type(attributes) != DIRECTORY){
			llog::error("input path of '" + remote_path.ip + "' isn't a directory");
			exit(1);
		}else if(status::remote_type(attributes) == DIRECTORY)
			os::append_seperator(remote_path.path);
		attr_obj.reset();

		fs_remote::readdir(remote_path, remote_path.path, index_path);
	}
}

namespace remote_readdir_operations {
	void type(const struct input_path& remote_path, const sftp_attributes& attributes, struct metadata& metadata, const std::string& full_path){
		if(options::no_broken_symlink && attributes->type == SYMLINK && sftp::is_broken_link(remote_path.sftp, full_path, remote_path.ip)){
			metadata.type = BROKEN_SYMLINK;
		}else if(options::follow_symlink){
			metadata.type = status::remote_type2(remote_path.sftp, full_path, true);
			if(metadata.type == -1)
				exit(1);
		}else
			metadata.type = (short int)(attributes->type);
	}

	void mtime(const struct input_path& remote_path, const sftp_attributes& attributes, struct metadata& metadata, const std::string& full_path){
		if(options::follow_symlink && metadata.type == SYMLINK){
			auto utime = utime::get_remote(remote_path.sftp, full_path, 2);
			if(std::holds_alternative<int>(utime))
				llog::rc(remote_path.sftp, full_path, std::get<int>(utime), "couldn't get mtime", EXIT_FAILURE);
			else
				metadata.mtime = std::get<struct time_val>(utime).mtime;
		}else
			metadata.mtime = attributes->mtime;
	}

	bool lspart(const struct input_path& remote_path, const struct metadata& metadata, const std::string& full_path, const std::string& relative_path,
			const std::string& file_name, const unsigned long int& index_path){

		if(resume::is_lspart(file_name)){
			if(options::resume)
				part_files.insert(path(relative_path, metadata, index_path));
			else{
				int rc = sftp::unlink(remote_path.sftp, full_path);
				llog::rc(remote_path.sftp, full_path, rc, "couldn't remove incomplete file.ls.part", NO_EXIT);
			}
			return true;
		}

		return false;
	}

	void insert(const struct metadata& metadata, const std::string& relative_path, const unsigned long int& index_path){
		auto itr = content.find(path(relative_path, metadata, index_path));
		if(itr != content.end())
			itr->metadatas[index_path] = metadata;
		else
			content.insert(path(relative_path, metadata, index_path));
	}
}

#endif // REMOTE_ENABLED
