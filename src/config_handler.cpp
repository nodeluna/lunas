#include <string>
#include <iostream>
#include "config_handler.h"
#include "config.h"
#include "cliargs.h"
#include "log.h"
#include "about.h"
#include "base.h"
#include "path_parsing.h"
#include "os.h"


namespace config_filler {
	void fill_local_path(const std::string& argument, const short& srcdest){
		struct input_path local_path;
		local_path.path = parse_path::absolute(argument);
		local_path.srcdest = srcdest;
		input_paths.push_back(std::move(local_path));
	}

	int lpath_srcdest(const std::string& data){
		config_filler::fill_local_path(data, SRCDEST);
		return 0;
	}

	int lpath_src(const std::string& data){
		config_filler::fill_local_path(data, SRC);
		return 0;
	}

	int lpath_dest(const std::string& data){
		config_filler::fill_local_path(data, DEST);
		return 0;
	}

	short rpath_srcdest(void){
		return SRCDEST;
	}

	short rpath_src(void){
		return SRC;
	}

	short rpath_dest(void){
		return DEST;
	}

	int compression_level(const std::string& data){
		if(is_num(data) == false)
			llog::error_exit("argument '" + data + "' for option compression-level isn't a number", EXIT_FAILURE);

		int level = std::stoi(data);
		if(level > 9 || level <= 0)
			llog::error_exit("compression level must be between 1-9. provided level '" + data + "'", EXIT_FAILURE);
		options::compression = level;
		return 0;
	}

	int exclude(const std::string& data){
		std::string path = data;
		os::pop_seperator(path);
		options::exclude.insert(path);
		return 0;
	}

	int exclude_pattern(const std::string& data){
		std::string path = data;
		os::pop_seperator(path);
		options::exclude_pattern.insert(path);
		return 0;
	}

	int mkdir(const std::string& data){
		if(data == "on")
			options::mkdir = true;
		else if(data == "off")
			options::mkdir = false;
		else
			return -1;
		return 0;
	}

	int progress(const std::string& data){
		if(data == "on"){
			options::progress_bar = true;
		}else if(data == "off"){
			options::progress_bar = false;
		}else
			return -1;
		return 0;
	}

	int dry_run(const std::string& data){
		if(data == "on"){
			llog::print("--> dry run is on\n");
			options::dry_run = true;
		}else if(data == "off")
			options::dry_run = false;
		else
			return -1;
		return 0;
	}

	int quiet(const std::string& data){
		if(data == "on")
			options::quiet = true;
		else if(data == "off")
			options::quiet = false;
		else
			return -1;
		return 0;
	}

	int verbose(const std::string& data){
		if(data == "on")
			options::verbose = true;
		else if(data == "off")
			options::verbose = false;
		else
			return -1;
		return 0;
	}

	int follow_symlink(const std::string& data){
		if(data == "on")
			options::follow_symlink = true;
		else if(data == "off")
			options::follow_symlink = false;
		else
			return -1;
		return 0;
	}

	int resume(const std::string& data){
		if(data == "on")
			options::resume = true;
		else if(data == "off")
			options::resume = false;
		else
			return -1;
		return 0;
	}

	int compression(const std::string& data){
		if(data == "on")
			options::compression = true;
		else if(data == "off")
			options::compression = false;
		else
			return -1;
		return 0;
	}

	int fsync(const std::string& data){
		if(data == "on")
			options::fsync = true;
		else if(data == "off")
			options::fsync = false;
		else
			return -1;
		return 0;
	}

	int remove_extra(const std::string& data){
		if(data == "on")
			options::remove_extra = true;
		else if(data == "off")
			options::remove_extra = false;
		else
			return -1;
		return 0;
	}

	int update(const std::string& data){
		if(data == "on"){
			options::update = true;
			options::rollback = false;
		}else if(data == "off")
			options::update = false;
		else
			return -1;
		return 0;
	}

	int rollback(const std::string& data){
		if(data == "on"){
			options::rollback = true;
			options::update = false;
		}else if(data == "off")
			options::rollback = false;
		else
			return -1;
		return 0;
	}
}


namespace lunas_info {
	void author(void){
		llog::print(about::author);
		exit(0);
	}
	
	void version(void){
		llog::print(about::version);
		exit(0);
	}

	void license(void){
		llog::print(about::license);
		exit(0);
	}

	void help(void){
		llog::print(about::help);
		exit(0);
	}
	void meow(void){
		llog::print("  ~-~ MeooOOoooooOOOooowwwwwW ~-~");
		exit(0);
	}
}
