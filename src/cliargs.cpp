#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "cliargs.h"
#include "config.h"
#include "log.h"
#include "about.h"
#include "path_parsing.h"
#include "config_handler.h"
#include "config_manager.h"

bool is_num(const std::string& x) {
	return std::all_of(x.begin(), x.end(), [](char c) { return std::isdigit(c); });
}

bool is_num_decimal(const std::string& x) {
	bool has_decimal = false;
	return std::all_of(x.begin(), x.end(), [&](char c) {
		if (std::isdigit(c))
			return true;
		else if (c == '.' && not has_decimal) {
			has_decimal = true;
			return true;
		} else
			return false;
	});
}

void next_arg_exists(const int& argc, const char* argv[], int i) {
	if ((argc - 1) == i) {
		llog::error(std::string("argument for option '") + argv[i] + std::string("' wasn't provided, exiting"));
		exit(1);
	}
	if (argv[i + 1][0] == '-') {
		llog::error(
		    std::string("invalid argument '") + argv[i + 1] + std::string("' for option '") + argv[i] + std::string("', exiting"));
		exit(1);
	}
}

int arg_exist(const char* argv[], const int& argc, int i) {
	if ((argc - 1) == i) {
		return -1;
	}
	if (argv[i + 1][0] == '-') {
		return -1;
	}
	return 0;
}

#ifdef REMOTE_ENABLED
struct input_path fill_remote_path(const int& argc, const char* argv[], int& index, const short& srcdest) {
	struct input_path remote_path;
	remote_path.ip	    = argv[index + 1];
	remote_path.srcdest = srcdest;
	remote_path.remote  = true;

	index++;
	while (index++ != (argc - 1)) {
		std::string option = argv[index];
		if (option.find('=') != option.npos)
			option.resize(option.find('='));
		if (option[0] == '-') {
			index--;
			break;
		}
		std::string argument = argv[index];
		argument	     = argument.substr(argument.find('=') + 1, argument.size());

		if ((option == "N" || option == "port") && is_num(argument) == false) {
			llog::error("argument '" + argument + "' for option '" + option + "' isn't a number");
			exit(1);
		} else if (option == "N" || option == "port") {
			int port = std::stoi(argument);
			if (port < 0) {
				llog::error("port number '" + std::to_string(port) + "' for '" + remote_path.ip + "' can't be negative ");
				exit(1);
			}
			remote_path.port = port;
		} else if (option == "pw" || option == "password") {
			remote_path.password = argument;
		} else {
			llog::error("option '" + option + "' isn't recognized");
			exit(1);
		}
	}

	return remote_path;
}
#endif // REMOTE_ENABLED

int fillopts(const int& argc, const char* argv[], int& index) {
	std::string option = argv[index];
	if (option == "-c" || option == "--config") {
		next_arg_exists(argc, argv, index);
		std::string argument = argv[index + 1];
		if (argument == "global")
			llog::error_exit("can't use -c global, 'global' preset runs by default if it exists", EXIT_FAILURE);
		std::optional<std::string> err = config_manager::preset(argument);
		if (err)
			llog::error_exit(*err, EXIT_FAILURE);
		index++;
	} else if (auto itr0 = lpaths_options.find(option); itr0 != lpaths_options.end()) {
		next_arg_exists(argc, argv, index);
		std::string argument = argv[index + 1];
		itr0->second(argument);
		index++;
#ifdef REMOTE_ENABLED
	} else if (auto itr1 = rpaths_options.find(option); itr1 != rpaths_options.end()) {
		next_arg_exists(argc, argv, index);
		const short srcdest = itr1->second();
		input_paths.push_back(fill_remote_path(argc, argv, index, srcdest));
#endif // REMOTE_ENABLED
	} else if (auto itr2 = onoff_options.find(option); itr2 != onoff_options.end()) {
		int	    ok = arg_exist(argv, argc, index);
		std::string argument;
		if (ok == -1)
			argument = "on";
		else {
			argument = argv[index + 1];
			index++;
		}
		ok = itr2->second(argument);
		if (ok != 0)
			llog::error_exit("wrong argument '" + argument + "' for on/off option '" + option + "'", EXIT_FAILURE);
	} else if (auto itr3 = misc_options.find(option); itr3 != misc_options.end()) {
		next_arg_exists(argc, argv, index);
		std::string argument = argv[index + 1];
		itr3->second(argument);
		index++;
	} else if (auto itr4 = info.find(option); itr4 != info.end())
		itr4->second();
	else {
		llog::error("option '" + option + "' wasn't recognized, read the man page");
		exit(1);
	}
	return 0;
}

int cliargs(const int& argc, const char* argv[]) {
	for (int i = 1; i < argc; i++) {
		fillopts(argc, argv, i);
	}
	return 0;
}
