#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <expected>
#	include <variant>
#	include <vector>
#	include <set>
#endif

#include "sftp/sftp.hpp"
#include "config/config.hpp"
#include "stdout/stdout.hpp"
#include "about/about.hpp"
#include "error/error.hpp"
#include "input_path/input_path.hpp"
#include "presync/presync.hpp"
#include "sync/sync.hpp"
#include "stats/stats.hpp"
#include "content/content.hpp"

int main(const int argc, const char* argv[])
{
	std::expected<struct lunas::parsed_data, lunas::error> cliopts = lunas::config::parse_cliarg(argc, argv);
	if (not cliopts)
	{
		lunas::printerr("{}", cliopts.error().message());
		return 1;
	}

	auto ret = lunas::presync_operations(cliopts.value());
	if (not ret)
	{
		lunas::printerr("{}", ret.error().message());
		return 2;
	}

	if (cliopts.value().get_ipaths().empty())
	{
		lunas::println(false, "{}", lunas::about::smol_help);
		return 3;
	}

	if (std::holds_alternative<lunas::content>(ret.value()))
	{
		lunas::content content = std::get<lunas::content>(ret.value());
		auto	       ok      = lunas::sync(cliopts.value(), content);
		if (not ok)
		{
			lunas::printerr("{}", ok.error().message());
			return 4;
		}
	}
	else
	{
		auto ok = lunas::sync(cliopts.value());
		if (not ok)
		{
			lunas::printerr("{}", ok.error().message());
			return 4;
		}
	}

	lunas::print_stats(cliopts.value());

	return 0;
}
