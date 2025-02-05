module;

#include <expected>
#include <print>

export module lunas.config;
export import lunas.config.file;
export import lunas.config.cliarg;

import lunas.ipath;
import lunas.stdout;
import lunas.error;
import lunas.path;

export namespace lunas {
	namespace config {
		std::expected<struct lunas::parsed_data, lunas::error> parse_cliarg(const int argc, const char* argv[]);
	}
}

namespace lunas {
	namespace config {
		std::expected<struct lunas::parsed_data, lunas::error> parse_cliarg(const int argc, const char* argv[]) {
			std::expected<struct cliarg::cliopts, lunas::error> parsed_data = lunas::cliarg::fillopts(argc, argv);
			if (not parsed_data)
				return std::unexpected(parsed_data.error());

			struct cliarg::cliopts& cliopts = parsed_data.value();
			if (auto ok = lunas::config_file::preset("global", cliopts.options, cliopts.ipaths); not ok)
				return std::unexpected(ok.error());

			for (auto& preset : cliopts.presets) {
				auto ok = lunas::config_file::preset(preset, cliopts.options, cliopts.ipaths);
				if (not ok)
					return std::unexpected(ok.error());
			}

			struct lunas::parsed_data data;

			for (auto& ipath : cliopts.ipaths) {
				if (std::holds_alternative<struct lunas::ipath::local_path>(ipath)) {
					auto& local_path = std::get<struct lunas::ipath::local_path>(ipath);
					lunas::path::append_seperator(local_path.path);
					struct lunas::ipath::input_path ipath(local_path.path);
					ipath.srcdest = local_path.srcdest;
					data.ipaths_emplace_back(std::move(ipath));

				} else if (std::holds_alternative<struct lunas::ipath::remote_path>(ipath)) {
					auto&				remote_path = std::get<struct lunas::ipath::remote_path>(ipath);
					struct lunas::ipath::input_path ipath;
					ipath.srcdest				   = remote_path.srcdest;
					remote_path.session_data.options.log_level = cliopts.options.ssh_log_level;
					remote_path.session_data.options.dry_run   = cliopts.options.dry_run;
					remote_path.session_data.options.timeout   = cliopts.options.timeout_sec;
					if (cliopts.options.compression)
						remote_path.session_data.options.compression_level = cliopts.options.compression_level;
					try {
						auto ok = ipath.init_sftp(remote_path.session_data);
						if (not ok) {
							auto& err = ok.error().message();
							return std::unexpected(lunas::error(err, lunas::error_type::init_sftp_error));
						}
					} catch (std::exception& e) {
						return std::unexpected(lunas::error(e.what(), lunas::error_type::init_sftp_error));
					}
					data.ipaths_emplace_back(std::move(ipath));
				}
			}

			data.options = std::move(cliopts.options);

			return data;
		}
	}
}
