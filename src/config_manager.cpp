#include <string>
#include <array>
#include <map>
#include <fstream>
#include <filesystem>
#include <optional>
#include <variant>
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

	std::variant<struct input_path, std::string> fill_remote_path(const std::multimap<std::string, std::string>& nest,
			std::multimap<std::string, std::string>::iterator& it, const std::string& name){
		size_t nest_size = it->first.find("remote::") + std::string("remote::").size();
		struct input_path remote_path;
		remote_path.remote = true;
		++it;
		while(it != nest.end() && it->first.find("remote::") != it->first.npos){
			std::string option = it->first.substr(nest_size, it->first.size());
			if(auto itr1 = rpaths_options.find(option); itr1 != rpaths_options.end()){
				if(remote_path.ip.empty() == false)
					return "remote path for nest'" + name + "' provided twice";
				remote_path.srcdest = itr1->second();
				remote_path.ip = it->second;
			}else if(option == "N" || option == "port"){
				if(is_num(it->second))
					remote_path.port = std::stoi(it->second);
				else
					return "nest '" + name + "': port '" + it->second + "' isn't a valid number";
			}else if(option == "pw" || option == "password")
				remote_path.password = it->second;
			++it;
		}

		if(remote_path.ip.empty())
			return "remote path for nest'" + name + "' isn't provided";

		return remote_path;
	}

	std::optional<std::string> config_fill(std::multimap<std::string, std::string>& nest, const std::string& name){
		for(auto it = nest.begin(); it != nest.end(); ++it){
			auto itr = onoff_options.find(it->first);
			if(itr != onoff_options.end()){
				itr->second(it->second);
			}else if(auto itr1 = lpaths_options.find(it->first); itr1 != lpaths_options.end()){
				itr1->second(it->second);
			}else if(it->first.find("remote::") != it->first.npos){
				auto remote_path = config_manager::fill_remote_path(nest, it, name);
				if(std::holds_alternative<struct input_path>(remote_path))
					input_paths.push_back(std::get<struct input_path>(remote_path));
				else if(std::holds_alternative<struct input_path>(remote_path))
					return std::get<std::string>(remote_path);
			}else if(it->first.front() != '#')
				return "nest '" + name + "': wrong option '" + it->first + "'";

			if(it == nest.end())
				break;
		}

		return std::nullopt;
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
		return config_manager::config_fill(nest, name);
	}
}
