module;

#include <expected>
#include <vector>
#include <variant>
#include <set>

export module lunas.presync;
export import :misc;
export import lunas.presync.fill_tree;
export import lunas.file_table;

import lunas.error;
import lunas.ipath;

export namespace lunas {
	std::expected<std::variant<std::set<lunas::file_table>, std::monostate>, lunas::error> presync_operations(
	    const lunas::parsed_data& cliopts);
}

namespace lunas {
	bool more_than_one_source(const std::vector<lunas::ipath::input_path>& ipaths) {
		bool found_source = false;
		for (const auto& path : ipaths) {
			if (not found_source && path.is_src())
				found_source = true;
			else if (found_source && path.is_src())
				return true;
		}

		return false;
	}

	std::expected<std::variant<std::set<lunas::file_table>, std::monostate>, lunas::error> presync_operations(
	    const lunas::parsed_data& cliopts) {

		if (auto ok = presync::input_paths_are_different(cliopts.get_ipaths()); not ok)
			return std::unexpected(ok.error());

		const auto&		    ipaths = cliopts.get_ipaths();
		std::set<lunas::file_table> content;

		for (size_t index = 0; index < ipaths.size(); index++) {
			struct lunas::fill_tree_type data =
			    lunas::presync::prepare_fill_tree_data(&ipaths[index], index, ipaths.size(), &cliopts.options);
			if (ipaths[index].is_remote()) {
				auto ok = lunas::presync::remote::input_directory_check(data);
				if (not ok)
					return std::unexpected(ok.error());

				if (more_than_one_source(ipaths)) {
					auto ok = lunas::presync::remote::readdir(content, ipaths[index].path, data);
					if (not ok)
						return std::unexpected(ok.error());
				} else
					return std::monostate();
			} else {
				auto ok = lunas::presync::local::input_directory_check(data);
				if (not ok)
					return std::unexpected(ok.error());

				if (more_than_one_source(ipaths)) {
					auto ok = lunas::presync::local::readdir(content, ipaths[index].path, data);
					if (not ok)
						return std::unexpected(ok.error());
				} else
					return std::monostate();
			}
		}

		return content;
	}
}
