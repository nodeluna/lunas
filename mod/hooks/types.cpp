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
	enum class hook_action {
		sync,
		dont_sync,
	};
}
