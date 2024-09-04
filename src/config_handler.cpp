#include <string>
#include <iostream>
#include "config_handler.h"
#include "config.h"
#include "cliargs.h"
#include "log.h"
#include "about.h"


namespace config_filler {
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
}
