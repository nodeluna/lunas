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
import lunas.sync;
import lunas.stats;
import lunas.content;

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
		return 2;
	}

	if (std::holds_alternative<lunas::content>(ret.value())) {
		lunas::content content = std::get<lunas::content>(ret.value());
		auto	       ok      = lunas::sync(cliopts.value(), content);
		if (not ok) {
			lunas::printerr("{}", ok.error().message());
			return 3;
		}
	} else {
		auto ok = lunas::sync(cliopts.value());
		if (not ok) {
			lunas::printerr("{}", ok.error().message());
			return 3;
		}
	}

	lunas::print_stats(cliopts.value());

	return 0;
}
