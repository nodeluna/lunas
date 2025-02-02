#include <expected>
#include <vector>

import lunas.sftp;
import lunas.config;
import lunas.stdout;
import lunas.error;
import lunas.ipath;

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

	for (auto& input_path : cliopts.value().ipaths) {
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

	return 0;
}
