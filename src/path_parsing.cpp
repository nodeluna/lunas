#include <filesystem>
#include <string>
#include "os.h"
#include "log.h"

namespace fs = std::filesystem;

namespace parse_path{
	void adjust_relative_path(std::string& path, int& depth){
		if(path.size() < 3)
			return;
		std::string three_char = path.substr(0, 3);
		if(three_char == "../"){
			path = path.substr(3, path.size());
			depth += 1;
			adjust_relative_path(path, depth);
		}
	}

	void append_to_relative_path(std::string& path, std::string& current_path, int& depth){
		if(depth != 0){
			depth -= 1;
			if(current_path.rfind(path_seperator) != current_path.npos)
				current_path.resize(current_path.rfind(path_seperator));
			else{
				llog::error("couldn't resolve relative path '" + path + "', too many '../'");
				exit(1);
			}
			append_to_relative_path(path, current_path, depth);
		}else if(current_path.back() != path_seperator)
			path = current_path + path_seperator + path;
		else
			path = current_path + path;
	}

	void clean_path(std::string& path){
		std::string temp;
		for(unsigned long int index = 0; index < path.size(); index++){
			if(path.substr(index, 2) == "./"){
				index++;
				continue;
			}
			temp += path[index];
		}

		path = temp;
	}

	std::string absolute(const std::string& path){
		std::string temp_path = path;
		if (path.size() == 0)
			return "";
		if (path.front() == path_seperator){
			os::append_seperator(temp_path);
			return temp_path;
		}if (path.size() > 1 && path.substr(0, 2) == "~/"){
			temp_path = std::getenv("HOME") + std::string(1, path_seperator) + path.substr(2, path.size());
			os::append_seperator(temp_path);
			return temp_path;
		}

		int depth = 0;
		std::string current_path = fs::current_path().string();
		parse_path::adjust_relative_path(temp_path, depth);
		parse_path::append_to_relative_path(temp_path, current_path, depth);
		parse_path::clean_path(temp_path);
		os::append_seperator(temp_path);
		return temp_path;
	}
}
