module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string>
#	include <cstddef>
#	include <limits>
#endif

export module lunas.presync.fill_tree:types;
import lunas.config.options;
import lunas.ipath;

export namespace lunas {
	struct fill_tree_type {
			const struct lunas::ipath::input_path* ipath	    = nullptr;
			size_t				       path_index   = std::numeric_limits<size_t>::max();
			size_t				       ipaths_count = 0;
			const lunas::config::options*	       options	    = nullptr;
	};

	namespace presync {
		fill_tree_type prepare_fill_tree_data(const struct lunas::ipath::input_path* ipath, size_t index, size_t ipaths_count,
		    const lunas::config::options* options) {

			struct fill_tree_type data;

			data.ipath	  = ipath;
			data.path_index	  = index;
			data.ipaths_count = ipaths_count;
			data.options	  = options;

			return data;
		}

		bool is_lspart(const std::string& path) {
			if (path.size() > 8 && path.substr(path.size() - 8, path.size()) == ".ls.part")
				return true;
			return false;
		}
	}
}
