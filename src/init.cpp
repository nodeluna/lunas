#include <filesystem>
#include <limits>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include "config.h"
#include "init.h"
#include "cliargs.h"
#include "base.h"
#include "fs.h"
#include "log.h"
#include "os.h"
#include "utimes.h"


void fill_base(void){
	unsigned long int index_path = 0;
	for(const auto& i : input_paths){
		list_tree(i, index_path);
		index_path++;
	}
}

void match_mtime(const std::string& src, const std::string& dest){
	struct time_val time_val = utime::get_local(src, 3);
	utime::set_local(dest, time_val);
}

void copy(const std::string& src, const std::string& dest, const short& type){
	llog::print("syncing '" + src + "' to '" + dest + "'");
	if(options::dry_run == false){
		fs::copy_options opts;
		if(options::update)
			opts = fs::copy_options::update_existing;
		else
			opts = fs::copy_options::overwrite_existing;
		fs::copy(src, dest, opts);
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
		else if(options::rollback && src_mtime <= metadata.mtime)
			goto end;

		src_mtime = metadata.mtime;
		src_mtime_i = i;
end:
		i++;
	}

	return src_mtime_i;
}



void updating(const struct path& file, const unsigned long int& src_mtime_i){
	unsigned long int src_mtime = file.metadatas[src_mtime_i].mtime;
	std::string src = input_paths.at(src_mtime_i).path + path_seperator + file.name;

	unsigned long int dest_index = 0;
	for(auto& metadata : file.metadatas){
		if(src_mtime_i == dest_index)
			goto end;
		if(condition::is_dest(input_paths.at(dest_index).srcdest) == false)
			goto end;

		if(options::update && src_mtime > metadata.mtime){
			std::string dest = input_paths.at(dest_index).path + path_seperator + file.name;
			copy(src, dest, metadata.type);
		}else if(options::rollback && src_mtime < metadata.mtime){
			std::string dest = input_paths.at(dest_index).path + path_seperator + file.name;
			copy(src, dest, metadata.type);
		}else if(metadata.mtime == -1){
			std::string dest = input_paths.at(dest_index).path + path_seperator + file.name;
			copy(src, dest, metadata.type);
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

int init_program(void){
	base::paths_count = input_paths.size();
	fill_base();
	syncing();
	return 0;
}
