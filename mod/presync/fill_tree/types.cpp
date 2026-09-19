#include <string>
#include <cstddef>

#include "types.hpp"

namespace lunas
{
	namespace presync
	{
		fill_tree_type prepare_fill_tree_data(const struct lunas::ipath::input_path* ipath, size_t index, size_t ipaths_count,
						      const lunas::config::options* options)
		{

			struct fill_tree_type data;

			data.ipath	  = ipath;
			data.path_index	  = index;
			data.ipaths_count = ipaths_count;
			data.options	  = options;

			return data;
		}

		bool is_lspart(const std::string& path)
		{
			if (path.size() > 8 && path.substr(path.size() - 8, path.size()) == ".ls.part")
			{
				return true;
			}
			return false;
		}
	}
}
