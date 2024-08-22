#include <filesystem>
#include <limits>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#include "remote_session.h"
#include "fs_remote.h"
#endif // REMOTE_ENABLED

#include "init.h"
#include "cliargs.h"
#include "base.h"
#include "fs.h"
#include "log.h"
#include "os.h"
#include "utimes.h"
#include "cppfs.h"


#ifdef REMOTE_ENABLED
void init_rsessions(void){
	for(auto & remote_path : input_paths){
		if(remote_path.remote == false)
			continue;
		ssh_session ssh = rsession::init_ssh(remote_path.ip, remote_path.port, remote_path.password);
		remote_path.sftp = rsession::init_sftp(ssh, remote_path.ip);
		remote_path.path = rsession::absolute_path(remote_path.sftp, remote_path.ip);
	}
}
#endif // REMOTE_ENABLED

void fill_base(void){
	unsigned long int index_path = 0;
	for(const auto& i : input_paths){
		llog::print("--> reading directory " + input_paths.at(index_path).path);
		if(i.remote == false)
			fs_local::list_tree(i, index_path);
#ifdef REMOTE_ENABLED
		else
			fs_remote::list_tree(i, index_path);
#endif // REMOTE_ENABLED
		index_path++;
	}
	llog::print("");
}

void match_mtime(const std::string& src, const std::string& dest){
	struct time_val time_val = utime::get_local(src, 3);
	utime::set_local(dest, time_val);
}

void copy(const std::string& src, const std::string& dest, const short& type){
	std::string count = "(" + std::to_string(base::syncing_counter) + std::string("/") + std::to_string(base::to_be_synced) + ")";
	if(type == DIRECTORY)
		llog::print(count + " [Dir] "+ src + "' to '" + dest + "'");
	else
		llog::print(count + " [File] " + src + "' to '" + dest + "'");
	base::syncing_counter++;
	if(options::dry_run)
		return;
	if(type == DIRECTORY)
		fs::create_directory(dest);
	else{
		std::error_code ec;
		cppfs::copy(src, dest, ec);
		if(!llog::ec(dest, ec, "couln't sync", NO_EXIT)){
			base::syncing_counter--;
			return;
		}
		match_mtime(src, dest);
	}
}

unsigned long int get_src(const struct path& file){
	long int src_mtime = -2;
	unsigned long int src_mtime_i = 0;
	unsigned long int i = 0;

	if(options::rollback)
		src_mtime = std::numeric_limits<long int>::max();

	for(const auto& metadata : file.metadatas){
		if(condition::is_src(input_paths.at(i).srcdest) == false)
			goto end;
		if(!options::rollback && src_mtime >= metadata.mtime)
			goto end;
		else if(options::rollback && (src_mtime <= metadata.mtime || metadata.mtime == -1))
			goto end;

		src_mtime = metadata.mtime;
		src_mtime_i = i;
end:
		i++;
	}

	return src_mtime_i;
}



void updating(const struct path& file, const unsigned long int& src_mtime_i){
	const long int& src_mtime = file.metadatas.at(src_mtime_i).mtime;
	const short& type = file.metadatas.at(src_mtime_i).type;
	std::string src = input_paths.at(src_mtime_i).path + file.name;

	unsigned long int dest_index = 0;
	for(const auto& metadata : file.metadatas){
		if(src_mtime_i == dest_index)
			goto end;
		if(condition::is_dest(input_paths.at(dest_index).srcdest) == false)
			goto end;
		if(metadata.mtime != NON_EXISTENT && metadata.type == DIRECTORY)
			goto end;

		if(options::update && src_mtime > metadata.mtime){
			std::string dest = input_paths.at(dest_index).path + file.name;
			copy(src, dest, type);
		}else if(options::rollback && src_mtime < metadata.mtime){
			std::string dest = input_paths.at(dest_index).path + file.name;
			copy(src, dest, type);
		}else if(metadata.mtime == NON_EXISTENT){
			std::string dest = input_paths.at(dest_index).path + file.name;
			copy(src, dest, type);
		}
end:
		dest_index++;
	}
}

void syncing(){
	for(const auto& file : content){
		unsigned long int src_mtime_i = get_src(file);
		updating(file, src_mtime_i);
	}
}

void counter(){
	for(const auto& file : content){
		long int tmp = -1;
		unsigned long int files_dont_exist = 0;
		for(const auto& metadata : file.metadatas){
			if(tmp == -1 && metadata.mtime != -1){
				tmp = metadata.mtime;
				continue;
			}
			if(metadata.mtime != -1 && metadata.mtime != tmp && metadata.type != DIRECTORY){
				base::to_be_synced += file.metadatas.size() - 1 - files_dont_exist;
				break;
			}else if(metadata.mtime == -1){
				base::to_be_synced++;
				files_dont_exist++;
			}
		}
	}
}

int init_program(void){
	base::paths_count = input_paths.size();
#ifdef REMOTE_ENABLED
	init_rsessions();
#endif // REMOTE_ENABLED
	fill_base();
	counter();
	syncing();
	return 0;
}
