#include <print>
#include <memory>
#include <expected>
#include <optional>

import lunas.sftp;

int main(const int argc, const char* argv[]) {
	if (argc < 3) {
		std::println("{} user@ip command", argv[0]);
		return 1;
	}

	lunas::session_data session_data = {
	    .ip	      = argv[1],
	    .port     = 22,
	    .pw	      = "",
	    .key_path = "",
	    .options{
		     .compression_level = 5,
		     .timeout	   = 5,
		     .log_level	   = lunas::ssh_log_level::no_log,
		     .dry_run	   = false,
		     },
	};
	std::unique_ptr<lunas::sftp> sftp;

	try {
		sftp = std::make_unique<lunas::sftp>(session_data);
	} catch (const std::exception& e) {
		std::println("{}", e.what());
		return 1;
	}

	std::expected<std::string, lunas::ssh_error> rv = sftp->cmd(argv[2]);
	if (rv)
		std::println("{}", rv.value());
	else
		std::println("err: {}", rv.error().message());

	std::string			name	 = "lunas";
	std::optional<lunas::ssh_error> rv_mkdir = sftp->mkdir(name);
	if (rv_mkdir)
		if (rv_mkdir->value() == lunas::sftp_error_type::file_already_exists) {
			std::println("directory '{}' already exists", name);
		} else
			std::println("err: {}", rv_mkdir->message());
	else {
		std::println("{1}made directory {0}", name, (session_data.options.dry_run ? "[dry run] " : ""));
	}

	return 0;
}
