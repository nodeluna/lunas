#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include "base.h"

std::set<path>::iterator find_in_tree(const std::set<path>& tree, const std::string& target){
	auto itr = std::find_if(tree.begin(), tree.end(), [&] (const path& a){
			return a.name == target;
			});
	return itr;
}
