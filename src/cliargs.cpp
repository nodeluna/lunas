#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include "cliargs.h"
#include "config.h"
#include "log.h"
#include "about.h"
#include "path_parsing.h"
#include "config_handler.h"


std::unordered_map<std::string, std::function<int(std::string)>> onoff_options = {
	{"-R",			config_filler::resume		},
	{"--resume",		config_filler::resume		},
	{"-mkdir",		config_filler::mkdir		},
	{"--make-directory",	config_filler::mkdir		},
	{"-q",			config_filler::quiet		},
	{"--quiet",		config_filler::quiet		},
	{"-v",			config_filler::verbose		},
	{"--verbose",		config_filler::verbose		},
	{"-P",			config_filler::progress		},
	{"--progress",		config_filler::progress		},
	{"-dr",			config_filler::dry_run		},
	{"--dry-run",		config_filler::dry_run		},
	{"-u",			config_filler::update		},
	{"--update",		config_filler::update		},
	{"-rb",			config_filler::rollback		},
	{"--rollback",		config_filler::rollback		},
	{"-L",			config_filler::follow_symlink	},
	{"--dereference",	config_filler::follow_symlink	},
	{"-F",			config_filler::fsync		},
	{"--fsync",		config_filler::fsync		},
#ifdef REMOTE_ENABLED
	{"-C",			config_filler::compression	},
	{"--compression",	config_filler::compression	},
#endif // REMOTE_ENABLED
};

std::unordered_map<std::string, std::function<void(void)>> info = {
	{"-h",			lunas_info::help		},
	{"--help",		lunas_info::help		},
	{"--author",		lunas_info::author		},
	{"-V",			lunas_info::version		},
	{"--version",		lunas_info::version		},
	{"--license",		lunas_info::license		},
};



bool is_num(const std::string& x){
    return std::all_of(x.begin(), x.end(), [](char c){
	    return std::isdigit(c);
	    });
}

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

int arg_exist(const char* argv[], const int& argc, int i){
        if((argc-1) == i){
                return -1;
        }
        if(argv[i+1][0] == '-'){
                return -1;
        }
        return 0;
}

void fill_local_path(const std::string& argument, const short& srcdest){
	struct input_path local_path;
	local_path.path = parse_path::absolute(argument);
	local_path.srcdest = srcdest;
	input_paths.push_back(std::move(local_path));
}

#ifdef REMOTE_ENABLED
void fill_remote_path(const int& argc, const char* argv[], int& index, const short& srcdest){
	struct input_path remote_path;
	remote_path.ip = argv[index+1];
	remote_path.srcdest = srcdest;
	remote_path.remote = true;

	index++;
	while(index++ != (argc-1)){
		std::string option = argv[index];
		if(option.find('=') != option.npos)
			option.resize(option.find('='));
		if(option[0] == '-'){
			index--;
			break;
		}
		std::string argument = argv[index];
		argument = argument.substr(argument.find('=')+1, argument.size());

		if((option == "N" || option == "port") && is_num(argument) == false){
			llog::error("argument '" + argument + "' for option '" + option + "' isn't a number");
			exit(1);
		}else if(option == "N" || option == "port"){
			int port = std::stoi(argument);
			if(port < 0){
				llog::error("port number '" + std::to_string(port) + "' for '" + remote_path.ip + "' can't be negative ");
				exit(1);
			}
			remote_path.port = port;
		}else if(option == "pw" || option == "password"){
			remote_path.password = argument;
		}else{
			llog::error("option '" + option + "' isn't recognized");
			exit(1);
		}
	}
	input_paths.push_back(std::move(remote_path));
}
#endif // REMOTE_ENABLED

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
#ifdef REMOTE_ENABLED
	}else if(option == "-r" || option == "--remote-path"){
		next_arg_exists(argc, argv, index);
		fill_remote_path(argc, argv, index, SRCDEST);
	}else if(option == "-rs" || option == "-rsrc" || option == "--remote-source"){
		next_arg_exists(argc, argv, index);
		fill_remote_path(argc, argv, index, SRC);
	}else if(option == "-rd" || option == "-rdest" || option == "--remote-destination"){
		next_arg_exists(argc, argv, index);
		fill_remote_path(argc, argv, index, DEST);
	}else if(option == "--compression-level" || option == "-CL"){
		next_arg_exists(argc, argv, index);
		std::string argument = argv[index+1];
		if(is_num(argument) == false)
			llog::error_exit("argument '" + argument + "' for option '" + option + "' isn't a number", EXIT_FAILURE);

		int level = std::stoi(argument);
		if(level > 9 || level <= 0)
			llog::error_exit("compression level must be between 1-9. provided level '" + argument + "'", EXIT_FAILURE);
		options::compression = level;
		index++;
#endif // REMOTE_ENABLED
	}else if(auto itr = onoff_options.find(option); itr != onoff_options.end()){
		int ok = arg_exist(argv, argc, index);
		std::string argument;
		if(ok == -1)
			argument = "on";
		else{
			argument = argv[index+1];
			index++;
		}
		ok = itr->second(argument);
		if(ok != 0)
			llog::error_exit("wrong argument '" + argument + "' for on/off option '" + option + "'", EXIT_FAILURE);
	}else if(option == "--exclude" || option == "-x"){
		next_arg_exists(argc, argv, index);
		std::string argument = argv[index+1];
		os::pop_seperator(argument);
		options::exclude.insert(argument);
		index++;
	}else if(auto itr = info.find(option); itr != info.end())
		itr->second();
	else{
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
