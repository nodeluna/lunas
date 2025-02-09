module;

#include <set>
#include <expected>
#include <variant>
#include <ctime>
#include <string>

export module lunas.sync;
export import :types;
export import :copy;
export import :multi_source;

export import lunas.error;
export import lunas.ipath;
export import lunas.file_table;
export import lunas.file_types;
export import lunas.content;

import lunas.stdout;

export namespace lunas {
	std::expected<std::monostate, lunas::error> sync(const struct lunas::parsed_data& data);
	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data, lunas::content& content);
}

export namespace lunas {
	std::expected<std::monostate, lunas::error> sync(const struct lunas::parsed_data& data) {
		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> sync(struct lunas::parsed_data& data, lunas::content& content) {

		struct progress_stats progress_stats;
		progress_stats.total_to_be_synced = content.to_be_synced;
		std::expected<std::monostate, lunas::error> synced;

		for (auto file = content.files_table.begin(); file != content.files_table.end();) {
			auto src_index = multi_source::get_src(*file, data);
			if (not src_index && data.options.remove_extra &&
			    src_index.error().value() == lunas::error_type::source_not_found) {
				goto retained_for_remove_extra;
			} else if (not src_index)
				goto not_needed_in_the_files_table_any_longer;

			synced = multi_source::updating(*file, src_index.value(), data, progress_stats);
			if (not synced)
				lunas::printerr("{}", synced.error().message());

		not_needed_in_the_files_table_any_longer:
			file = content.files_table.erase(file);
			continue;

		retained_for_remove_extra:
			++file;
		}

		for (auto file = content.files_table.rbegin(); file != content.files_table.rend(); ++file) {
			// TODO: remove_extra
		}

		return std::monostate();
	}
}
