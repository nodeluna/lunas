#include <iostream>
#include <string>
#include <vector>
#include "cliargs.h"
#include "config.h"
#include "log.h"
#include "about.h"

void next_arg_exists(const int& argc, const char* argv[], int i){
        if((argc-1) == i){
		llog::error(std::string("argument for option '") + argv[i] + std::string("' wasn't provided, exiting"));
                exit(1);
        }
        if(argv[i+1][0] == '-'){
		llog::error(std::string("invalid argument '") + argv[i+1] + std::string("' for option '") + argv[i] + std::string("', exiting"));
                exit(1);
        }
}

void fill_local_path(const std::string& argument, const short& srcdest){
	struct local_path local_path;
	local_path.path = argument;
	local_path.srcdest = srcdest;
	local_paths.push_back(std::move(local_path));
}

int fillopts(const int& argc, const char* argv[], int& index){
	std::string option = argv[index];
	if(option == "-p" || option == "--path"){
		next_arg_exists(argc, argv, index);
		std::string argument = argv[index+1];
		fill_local_path(argument, SRCDEST);
		index++;
	}else if(option == "-s" || option == "-src" || option == "--source"){
		next_arg_exists(argc, argv, index);
		std::string argument = argv[index+1];
		fill_local_path(argument, SRC);
		index++;
	}else if(option == "-d" || option == "-dest" || option == "--destination"){
		next_arg_exists(argc, argv, index);
		std::string argument = argv[index+1];
		fill_local_path(argument, DEST);
		index++;
	}else if(option == "-u" || option == "--update"){
		options::update = true;
		options::rollback = false;
	}else if(option == "-rb" || option == "--rollback"){
		options::update = false;
		options::rollback = true;
	}else if(option == "--author"){
		llog::print(about::author);
		exit(0);
	}else if(option == "--version" ||  option == "-V"){
		llog::print(about::version);
		exit(0);
	}else{
		llog::error("option '" + option + "' wasn't recognized, read the man page");
		exit(1);
	}
	return 0;
}

int cliargs(const int& argc, const char* argv[]){
	for(int i = 1; i < argc; i++){
		fillopts(argc, argv, i);
	}
	return 0;
}
