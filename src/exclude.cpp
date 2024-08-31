#include <string>
#include <algorithm>
#include "config.h"


namespace utils{
	bool exclude(const std::string& path, const std::string& input_path){
		if(options::exclude.empty())
			return false;

		auto itr = options::exclude.find(path.substr(input_path.size() , path.size()));
		if(itr != options::exclude.end())
			return true;

		std::string relative_path = path.substr(input_path.size(), path.size());

		return std::any_of(options::exclude.begin(), options::exclude.end(), 
				[&](const std::string& x_path){
					return relative_path.size() > x_path.size() && relative_path.substr(0, x_path.size()) == x_path;
				});
	}
}
