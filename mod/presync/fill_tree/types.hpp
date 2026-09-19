#pragma once

#include <string>
#include <cstddef>
#include <limits>

#include "config/options.hpp"
#include "input_path/input_path.hpp"

namespace lunas
{
	struct fill_tree_type {
			const struct lunas::ipath::input_path* ipath	    = nullptr;
			size_t				       path_index   = std::numeric_limits<size_t>::max();
			size_t				       ipaths_count = 0;
			const lunas::config::options*	       options	    = nullptr;
	};

	namespace presync
	{
		fill_tree_type prepare_fill_tree_data(const struct lunas::ipath::input_path* ipath, size_t index, size_t ipaths_count,
						      const lunas::config::options* options);

		bool	       is_lspart(const std::string& path);
	}
}
