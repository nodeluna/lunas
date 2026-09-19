#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <expected>
#	include <vector>
#	include <variant>
#	include <set>
#endif

#include "presync.hpp"

namespace lunas
{
	bool more_than_one_source(const std::vector<lunas::ipath::input_path>& ipaths)
	{
		bool found_source = false;
		for (const auto& path : ipaths)
		{
			if (not found_source && path.is_src())
			{
				found_source = true;
			}
			else if (found_source && path.is_src())
			{
				return true;
			}
		}

		return false;
	}

	std::expected<std::variant<lunas::content, std::monostate>, lunas::error> presync_operations(const lunas::parsed_data& cliopts)
	{

		if (auto ok = presync::input_paths_are_different(cliopts.get_ipaths()); not ok)
		{
			return std::unexpected(ok.error());
		}

		const auto&    ipaths = cliopts.get_ipaths();
		lunas::content content;

		for (size_t index = 0; index < ipaths.size(); index++)
		{
			struct lunas::fill_tree_type data =
			    lunas::presync::prepare_fill_tree_data(&ipaths[index], index, ipaths.size(), &cliopts.options);

			auto ok = lunas::presync::input_directory_check(data);
			if (not ok)
			{
				return std::unexpected(ok.error());
			}

			if (more_than_one_source(ipaths))
			{
				lunas::println(cliopts.options.quiet, "--> reading directory {}", ipaths[index].path);
				auto ok = lunas::presync::readdir(content.files_table, ipaths[index].path, data);

				if (not ok && ok.error().value() == lunas::error_type::no_such_file && cliopts.options.dry_run)
				{
					continue;
				}
				else if (not ok)
				{
					return std::unexpected(ok.error());
				}
			}
		}

		if (not more_than_one_source(ipaths))
		{
			return std::monostate();
		}

		return content;
	}
}
