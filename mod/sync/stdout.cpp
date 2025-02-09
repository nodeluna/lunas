module;

#include <string>
#include <print>
#include <cmath>

export module lunas.sync:stdout;
export import :types;
export import lunas.file_types;

export namespace lunas {
	void print_sync(const std::string& src, const std::string& dest, const struct lunas::syncmisc& misc) {
		if (misc.options.quiet)
			return;
		unsigned long padding_length = std::ceil(std::log10(misc.progress_stats.total_to_be_synced));
		if (std::log10(misc.progress_stats.total_to_be_synced) ==
		    static_cast<std::uintmax_t>(std::log10(misc.progress_stats.total_to_be_synced)))
			padding_length++;

		padding_length -= std::ceil(std::log10(misc.progress_stats.total_synced));
		if (std::log10(misc.progress_stats.total_synced) ==
		    static_cast<std::uintmax_t>(std::log10(misc.progress_stats.total_synced)))
			padding_length--;

		std::string padding(padding_length, ' ');

		std::string count = "(" + padding + std::to_string(misc.progress_stats.total_synced) + std::string("/") +
				    std::to_string(misc.progress_stats.total_to_be_synced) + ") ";
		std::string space(count.size() - 3, ' ');
		if (misc.file_type == lunas::file_types::directory) {
			if (misc.options.verbose)
				std::println("{} [Dir]   '{}''\n{}-> [At]   '{}'", count, src, space, dest);
			else
				std::println("{} [Dir]   '{}'", count, dest);
		} else {
			if (misc.options.verbose)
				std::println("{} [File]  '{}''\n{}-> [To]   '{}'", count, src, space, dest);
			else
				std::println("{} [File]  '{}'", count, dest);
		}
	}
}
