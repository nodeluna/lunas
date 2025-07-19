module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <vector>
#	include <memory>
#	include <variant>
#endif

export module lunas.hooks:types;
export import lunas.file;

export namespace lunas
{
	template<typename prehoook_type>
	struct _hooks {
			std::vector<prehoook_type>						 prehooks;
			std::variant<lunas::directory_entry, std::shared_ptr<lunas::attributes>> attributes;
	};

	enum class hook_action {
		sync,
		dont_sync,
	};
}
