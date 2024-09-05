#include <string>
#include <array>
#include <map>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "luco.h"
#include "cppfs.h"
#include "log.h"
#include "config_manager.h"

namespace fs = std::filesystem;


namespace config_manager {
	std::string config_dir = std::getenv("HOME") + std::string("/.config/lunas/");
	std::string file_name = std::string("lunas.luco");

	int make_demo_config(void){
		std::string config_file = config_dir  + file_name;
		std::error_code ec;
		if(fs::exists(config_dir, ec) != true){
			cppfs::mkdir(config_dir, ec);
			llog::ec(config_file, ec, "couldn't create config dir", EXIT_FAILURE);
		}

		if(fs::exists(config_file, ec) == true){
			llog::error("couldn't create a demo config, a config file already exists");
			exit(1);
		}
		llog::ec(config_file, ec, "couldn't check config file", EXIT_FAILURE);

		std::fstream file;
		file.open(config_file, std::ios::out);
		if(file.is_open() == false){
			llog::error("couldn't open config file '" + config_file + "', " + std::strerror(errno));
			exit(1);
		}

		file << DEMO_CONFIG;
		if(file.bad() == true){
			llog::error("error writing config file '" + config_file + "', " + std::strerror(errno));
			exit(1);
		}

		file.close();
		if(file.is_open() == true){
			llog::warn("couldn't close config file '" + config_file + "', " + std::strerror(errno));
		}

		llog::print(":: wrote a demo config file to '" + config_file + "'");
		return 0;
	}

	void config_fill(const std::multimap<std::string, std::string>& nest, const std::string& name){
		for(auto& option : nest){
			auto itr = onoff_options.find(option.first);
			if(itr != onoff_options.end()){
				itr->second(option.second);
			}else if(option.first.front() != '#')
				llog::error_exit("nest '" + name + "': wrong option '" + option.first + "'", EXIT_FAILURE);
		}
	}

	void preset(const std::string& name){
		if(name == "DEMO_CONFIG"){
			make_demo_config();
			return;
		}

		luco::luco nests = luco::luco(config_dir + file_name);
		nests.parse();

		auto itr = nests.find_preset("global");
		if(itr == nests.get_map().end())
			llog::error_exit("preset 'global' doesn't exist", EXIT_FAILURE);
		itr = nests.find_preset(name);
		if(itr == nests.get_map().end())
			llog::error_exit("preset '" + name + "' doesn't exist", EXIT_FAILURE);

		auto global_nest = nests.preset("global");
		config_manager::config_fill(global_nest, "global");

		auto nest = nests.preset(name);
		config_manager::config_fill(nest, name);
	}
}
