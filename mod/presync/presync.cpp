module;

#include <expected>
#include <vector>
#include <variant>
#include <set>

export module lunas.presync;
export import :misc;
export import lunas.presync.fill_tree;
export import lunas.file_table;
export import lunas.content;

import lunas.error;
import lunas.ipath;
import lunas.stdout;

export namespace lunas {
	std::expected<std::variant<lunas::content, std::monostate>, lunas::error> presync_operations(const lunas::parsed_data& cliopts);
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

	std::expected<std::variant<lunas::content, std::monostate>, lunas::error> presync_operations(const lunas::parsed_data& cliopts) {

		if (auto ok = presync::input_paths_are_different(cliopts.get_ipaths()); not ok)
			return std::unexpected(ok.error());

		const auto&    ipaths = cliopts.get_ipaths();
		lunas::content content;

		for (size_t index = 0; index < ipaths.size(); index++) {
			struct lunas::fill_tree_type data =
			    lunas::presync::prepare_fill_tree_data(&ipaths[index], index, ipaths.size(), &cliopts.options);

			auto ok = lunas::presync::input_directory_check(data);
			if (not ok)
				return std::unexpected(ok.error());

			if (more_than_one_source(ipaths)) {
				lunas::println(cliopts.options.quiet, "--> reading directory {}", ipaths[index].path);
				auto ok = lunas::presync::readdir(content.files_table, ipaths[index].path, data);
				if (not ok)
					return std::unexpected(ok.error());
			}
		}

		if (not more_than_one_source(ipaths))
			return std::monostate();

		content.to_be_synced = lunas::presync::to_be_synced_counter(content.files_table);

		return content;
	}
}
