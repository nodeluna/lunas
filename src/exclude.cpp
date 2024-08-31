#include <string>
#include "config.h"


namespace utils{
	bool exclude(const std::string& path, const std::string& input_path){
		if(options::exclude.empty())
			return false;

		auto itr = options::exclude.find(path.substr(input_path.size() , path.size()));
		if(itr != options::exclude.end())
			return true;

		std::string relative_path = path.substr(input_path.size(), path.size());
		for(const auto& exclude : options::exclude){
			if(relative_path.size() > exclude.size() && relative_path.substr(0, exclude.size()) == exclude)
				return true;
		}
		return false;
	}
}
