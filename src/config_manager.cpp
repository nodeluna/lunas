#include <string>
#include <array>
#include <map>
#include <fstream>
#include <filesystem>
#include <optional>
#include <expected>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "luco.h"
#include "cppfs.h"
#include "log.h"
#include "config_manager.h"
#include "cliargs.h"

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

#ifdef REMOTE_ENABLED
	std::expected<struct input_path, std::string> fill_remote_path(const std::multimap<std::string, std::string>& nest,
			std::multimap<std::string, std::string>::iterator& it, const std::string& name){
		std::string nest_path = it->first;
		size_t nest_size = nest_path.size();
		struct input_path remote_path;
		remote_path.remote = true;
		while(it != nest.end() && it->first.find(nest_path) != it->first.npos){
			std::string option = it->first.substr(nest_size, it->first.size());
			if(auto itr1 = rpaths_options.find(option); itr1 != rpaths_options.end()){
				if(remote_path.ip.empty() == false)
					return std::unexpected("remote path for nest'" + name + "' provided twice");
				remote_path.srcdest = itr1->second();
				remote_path.ip = it->second;
			}else if(option == "N" || option == "port"){
				if(is_num(it->second))
					remote_path.port = std::stoi(it->second);
				else
					return std::unexpected("nest '" + name + "': port '" + it->second + "' isn't a valid number");
			}else if(option == "pw" || option == "password")
				remote_path.password = it->second;
			++it;
		}

		if(it != nest.begin())
			--it;
		if(remote_path.ip.empty())
			return std::unexpected("remote path for nest'" + name + "' isn't provided");

		return remote_path;
	}
#endif // REMOTE_ENABLED

	std::optional<std::string> config_fill(std::multimap<std::string, std::string>& nest, const std::string& name){
		for(auto it = nest.begin(); it != nest.end(); ++it){
			if(auto itr = onoff_options.find(it->first); itr != onoff_options.end()){
				itr->second(it->second);
			}else if(auto itr1 = lpaths_options.find(it->first); itr1 != lpaths_options.end()){
				itr1->second(it->second);
			}else if(auto itr2 = misc_options.find(it->first); itr2 != misc_options.end()){
				itr2->second(it->second);
			}
#ifdef REMOTE_ENABLED
			else if(auto itr3 = rpaths_options.find(it->first); itr3 != rpaths_options.end()){
				struct input_path remote_path;
				remote_path.path = it->second;
				remote_path.srcdest = itr3->second();
				remote_path.remote = true;
				input_paths.push_back(remote_path);
			}else if(it->first.find("remote::") != it->first.npos && it->first.front() != '#'){
				auto remote_path = config_manager::fill_remote_path(nest, it, name);
				if(remote_path)
					input_paths.push_back(remote_path.value());
				else
					return remote_path.error();
			}
#endif // REMOTE_ENABLED
			else if(it->first.front() != '#')
				return "nest '" + name + "': wrong option '" + it->first + "'";
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
