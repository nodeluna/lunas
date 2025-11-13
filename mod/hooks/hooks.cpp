module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string>
#	include <expected>
#	include <variant>
#	include <vector>
#endif

export module lunas.hooks;
import :cmd;
export import :types;
export import :hook;
import :parser;

export namespace lunas
{
	template<typename hook_type>
	std::expected<std::vector<lunas::hook<hook_type>>, lunas::error> setup_hooks(const std::vector<std::string>& hooks)
	{
		std::vector<lunas::hook<hook_type>> parsed_hooks;

		auto init_hooks = [&parsed_hooks](const std::string& prehook) -> std::expected<std::monostate, lunas::error>
		{
			parsed_hooks.push_back({});
			auto ok = parsed_hooks.back().parse(prehook);
			if (not ok)
			{
				if constexpr (std::is_same_v<hook_type, lunas::pre>)
				{
					lunas::printerr("[prehook-parsing-error]");
				}
				else if constexpr (std::is_same_v<hook_type, lunas::post>)
				{
					lunas::printerr("[posthook-parsing-error]");
				}
				else
				{
					lunas::printerr("[hook-parsing-error]");
				}
				return std::unexpected(ok.error());
			}

			return std::monostate();
		};

		if constexpr (std::is_same_v<hook_type, lunas::pre> || std::is_same_v<hook_type, lunas::post>)
		{
			for (const auto& hook : hooks)
			{
				auto ok = init_hooks(hook);
				if (not ok)
				{
					return std::unexpected(ok.error());
				}
			}
		}
		else
		{
			static_assert(false, "unsupported tag type for lunas::hook<tag>");
		}

		return parsed_hooks;
	}
}
