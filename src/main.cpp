#include <expected>
#include <variant>
#include <vector>
#include <set>

import lunas.sftp;
import lunas.config;
import lunas.stdout;
import lunas.error;
import lunas.ipath;
import lunas.presync;
import lunas.stats;

int main(const int argc, const char* argv[]) {
	std::expected<struct lunas::parsed_data, lunas::error> cliopts = lunas::config::parse_cliarg(argc, argv);
	if (not cliopts) {
		lunas::printerr("{}", cliopts.error().message());
		if (cliopts.error().value() == lunas::error_type::config_file_parsing_error) {
			lunas::printerr("parsing error");
		} else if (cliopts.error().value() == lunas::error_type::init_sftp_error) {
			lunas::printerr("sftp init error");
		}
		return 1;
	}

	auto ret = lunas::presync_operations(cliopts.value());
	if (not ret) {
		lunas::printerr("{}", ret.error().message());
		return 1;
	}

	if (std::holds_alternative<std::set<lunas::file_table>>(ret.value())) {
		std::set<lunas::file_table> content = std::get<std::set<lunas::file_table>>(ret.value());
		for (const auto& file : content) {
			lunas::println(cliopts->options.quiet, "{}", file.path);
		}
	} else {
		lunas::println(cliopts->options.quiet, "didn't read input directories, only one source was found");
	}

	for (const auto& input_path : cliopts.value().get_ipaths()) {
		lunas::println(cliopts->options.quiet, "path: {}", input_path.path);
		if (not input_path.is_remote())
			continue;

		auto ret = input_path.sftp->cmd("lunas");
		if (not ret) {
			lunas::printerr("{}", ret.error().message());
			continue;
		}
		lunas::println(cliopts->options.quiet, "{}", ret.value());
	}

	lunas::print_stats(cliopts.value());

	return 0;
}
