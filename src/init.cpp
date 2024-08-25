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
#include "fs_local.h"
#include "log.h"
#include "os.h"
#include "cppfs.h"
#include "copy.h"
#include "size_units.h"


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
	for(auto& i : input_paths){
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

unsigned long int get_src(const struct path& file){
	long int src_mtime = -2;
	unsigned long int src_mtime_i = std::numeric_limits<unsigned long int>::max();
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

void register_sync(const struct syncstat& syncstat, const unsigned long int& dest_index, const short& type){
	if(syncstat.code == 0)
		return;
	input_paths.at(dest_index).synced_size += syncstat.copied_size;
	if(type == DIRECTORY)
		input_paths.at(dest_index).synced_dirs++;
	else
		input_paths.at(dest_index).synced_files++;
	base::syncing_counter++;
}

void updating(const struct path& file, const unsigned long int& src_mtime_i){
	const long int& src_mtime = file.metadatas.at(src_mtime_i).mtime;
	const short& type = file.metadatas.at(src_mtime_i).type;
	std::string src = input_paths.at(src_mtime_i).path + file.name;

	unsigned long int dest_index = 0;
	bool sync = false;
	for(const auto& metadata : file.metadatas){
		if(src_mtime_i == dest_index)
			goto end;
		if(condition::is_dest(input_paths.at(dest_index).srcdest) == false)
			goto end;
		if(metadata.mtime != NON_EXISTENT && metadata.type == DIRECTORY)
			goto end;

		if(options::update && src_mtime > metadata.mtime)
			sync = true;
		else if(options::rollback && src_mtime < metadata.mtime)
			sync = true;
		else if(metadata.mtime == NON_EXISTENT)
			sync = true;
		if(sync){
			std::string dest = input_paths.at(dest_index).path + file.name;
#ifdef REMOTE_ENABLED
			syncstat syncstat = lunas::copy(src, dest, input_paths.at(src_mtime_i).sftp, input_paths.at(dest_index).sftp, type);
#else
			struct syncstat syncstat =lunas::copy(src, dest, type);
#endif // REMOTE_ENABLED
			register_sync(syncstat, dest_index, type);
		}
end:
		sync = false;
		dest_index++;
	}
}

void syncing(){
	for(const auto& file : content){
		unsigned long int src_mtime_i = get_src(file);
		if(src_mtime_i == std::numeric_limits<unsigned long int>::max())
			continue;
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

#ifdef REMOTE_ENABLED
void free_rsessions(){
	for(auto& input_path : input_paths){
		if(input_path.remote == false)
			continue;
		ssh_session ssh = input_path.sftp->session;
		rsession::free_sftp(input_path.sftp);
		rsession::free_ssh(ssh);
	}
}
#endif // REMOTE_ENABLED

void print_stats(){
	llog::print("");
	for(const auto& input_path : input_paths){
		std::string stats = size_units(input_path.synced_size);
		stats += ", Files: " + std::to_string(input_path.synced_files);
		stats += ", Dirs: " + std::to_string(input_path.synced_dirs);
		llog::print("total synced to '" + input_path.path + "': " + stats);
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
#ifdef REMOTE_ENABLED
	free_rsessions();
#endif // REMOTE_ENABLED
	print_stats();
	return 0;
}
