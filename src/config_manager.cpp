#include <string>
#include <array>
#include <map>
#include <fstream>
#include <filesystem>
#include <optional>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "luco.h"
#include "cppfs.h"
#include "log.h"
#include "config_manager.h"

namespace fs = std::filesystem;


namespace config_manager {
	std::optional<std::string> make_demo_config(void){
		std::string config_file = config_dir  + file_name;
		std::error_code ec;
		if(fs::exists(config_dir, ec) != true){
			cppfs::mkdir(config_dir, ec);
			if(ec.value() != 0)
				return "couldn't create config dir '" + config_dir + std::strerror(errno);
		}

		if(fs::exists(config_file, ec) == true)
			return "couldn't create a demo config, a config file already exists";

		if(ec.value() != 0)
			return "couldn't check config file '" + config_file + "', " + std::strerror(errno);

		std::fstream file;
		file.open(config_file, std::ios::out);
		if(file.is_open() == false)
			return "couldn't open config file '" + config_file + "', " + std::strerror(errno);

		file << DEMO_CONFIG;
		if(file.bad() == true)
			return "error writing config file '" + config_file + "', " + std::strerror(errno);

		file.close();
		if(file.is_open() == true){
			llog::warn("couldn't close config file '" + config_file + "', " + std::strerror(errno));
		}

		llog::print(":: wrote a demo config file to '" + config_file + "'");
		return std::nullopt;
	}

	void config_fill(const std::multimap<std::string, std::string>& nest, const std::string& name){
		for(auto& option : nest){
			auto itr = onoff_options.find(option.first);
			if(itr != onoff_options.end()){
				itr->second(option.second);
			}else if(auto itr1 = lpaths_options.find(option.first); itr1 != lpaths_options.end()){
				itr1->second(option.second);
			}else if(option.first.front() != '#')
				llog::error_exit("nest '" + name + "': wrong option '" + option.first + "'", EXIT_FAILURE);
		}
	}

	std::optional<std::string> preset(const std::string& name){
		if(name == "DEMO_CONFIG")
			return make_demo_config();

		luco::luco nests = luco::luco(config_dir + file_name);
		if(!nests.any_errors().empty())
			return nests.any_errors();

		nests.parse();

		auto itr = nests.find_preset(name);
		if(itr == nests.get_map().end())
			return "preset '" + name + "' doesn't exist";

		llog::print(":: running preset '" + name + "'");

		auto nest = nests.preset(name);
		config_manager::config_fill(nest, name);

		return std::nullopt;
	}
}
