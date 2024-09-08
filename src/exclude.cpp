#include <string>
#include <algorithm>
#include "config.h"


namespace utils{
	bool exclude(const std::string& path, const std::string& input_path){
		if(options::exclude.empty() && options::exclude_pattern.empty())
			return false;

		std::string relative_path = path.substr(input_path.size(), path.size());

		if(not options::exclude.empty()){
			auto itr = options::exclude.find(path.substr(input_path.size() , path.size()));
			if(itr != options::exclude.end())
				return true;
			bool excluded = std::any_of(options::exclude.begin(), options::exclude.end(), 
					[&](const std::string& x_path){
						return relative_path.size() > x_path.size() && relative_path.substr(0, x_path.size()) == x_path;
					});
			if(excluded)
				return true;
		}

		if(not options::exclude_pattern.empty()){
			bool excluded = std::any_of(options::exclude_pattern.begin(), options::exclude_pattern.end(),
					[&](const std::string& x_path){
						return relative_path.find(x_path) != relative_path.npos;
					});
			if(excluded)
				return true;
		}

		return false;
	}
}
