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
	for(const auto& i : local_paths){
		list_tree(i);
	}
}

void match_mtime(const std::string& src, const std::string& dest){
	struct time_val time_val = utime::get_local(src, 3);
	utime::set_local(dest, time_val);
}

void append_path(const struct path& file, struct input_path& metadata, const std::string& path){
	struct input_path input_path;
	input_path.path = path;
	input_path.mtime = -1;
	input_path.type = metadata.type;
	file.input_paths.push_back(input_path);
}


void copy(const struct path& file, struct input_path& metadata, const unsigned long int& src_mtime_i, const long int& src_mtime){
	std::string src = file.input_paths.at(src_mtime_i).path + "/" + file.name;
	std::string dest = metadata.path + "/" + file.name;
	llog::print("syncing '" + src + "' to '" + dest + "'");
	if(options::dry_run == false){
		fs::copy_options opts = fs::copy_options::update_existing;
		fs::copy(src, dest, opts);
	}
	match_mtime(src, dest);
	metadata.mtime = src_mtime;
}

unsigned long int get_src(const struct path& file){
	long int src_mtime = -2;
	unsigned long int src_mtime_i = 0;
	unsigned long int i = 0;

	for(const auto& metadata : file.input_paths){
		if(condition::is_src(metadata.srcdest) == false)
			continue;
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

void updating(const struct path& file, std::set<std::string>& paths_track, const long int& src_mtime, const unsigned long int& src_mtime_i){


	for(auto& metadata : file.input_paths){
		if((!options::update && !options::rollback))
			goto skip;
		if(metadata.path == file.input_paths.at(src_mtime_i).path)
			continue;
		if(condition::is_dest(metadata.srcdest) == false)
			continue;
		if(options::update && src_mtime > metadata.mtime)
			copy(file, metadata, src_mtime_i, src_mtime);
		/*else if(options::rollback && src_mtime < metadata.mtime)
			copy(file, metadata, src_mtime_i, src_mtime);*/
skip:
		if(paths_track.find(metadata.path) != paths_track.end())
			paths_track.erase(metadata.path);
	}
}

void making_files(const struct path& file, std::set<std::string>& paths_track, const long int& src_mtime, const unsigned long int& src_mtime_i){
	for(const auto& path_track : paths_track){
		if(path_track == file.input_paths.at(src_mtime_i).path)
			continue;
		if(condition::is_dest(file.input_paths.at(src_mtime_i).srcdest) == false)
			continue;
		append_path(file, file.input_paths.at(src_mtime_i), path_track);
		copy(file, file.input_paths.back(), src_mtime_i, src_mtime);
	}
}

void syncing(){
	for(const auto& file : content){
		std::set<std::string> paths_track;
		for(const auto& i : local_paths)
			paths_track.insert(i.path);

		unsigned long int src_mtime_i = get_src(file);
		long int src_mtime = file.input_paths.at(src_mtime_i).mtime;

		updating(file, paths_track, src_mtime, src_mtime_i);

		making_files(file, paths_track, src_mtime, src_mtime_i);
	}
}

int init_program(void){
	base::paths_count = local_paths.size();
	fill_base();
	syncing();
	return 0;
}
