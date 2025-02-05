module;

#include <expected>
#include <vector>
#include <string>
#include <print>
#include <variant>

export module lunas.presync:misc;

import lunas.error;
import lunas.stdout;
import lunas.ipath;
import lunas.sftp;

export namespace lunas {
	namespace presync {
		std::expected<std::monostate, lunas::error> input_paths_are_different(
		    const std::vector<struct lunas::ipath::input_path>& ipaths);
	}
}

namespace lunas {
	namespace presync {
		std::string fmt_err(const std::string& path1, const std::string& path2) {
			std::string err = "";
			lunas::fmterr_multiline(err, "input paths are the same");
			lunas::fmterr_multiline(err, "{}", path1);
			lunas::fmterr_multiline(err, "{}", path2);
			return err;
		}

		std::expected<std::monostate, lunas::error> input_paths_are_different(
		    const std::vector<struct lunas::ipath::input_path>& ipaths) {

			size_t out = 0, in = 0;
			for (const auto& path1 : ipaths) {
				in = 0;
				for (const auto& path2 : ipaths) {
					if (out == in)
						goto end;
					else if ((path1.is_remote() && not path2.is_remote()) ||
						 (not path1.is_remote() && path2.is_remote()))
						goto end;

					if (not path1.is_remote() && not path2.is_remote() && path1.path == path2.path) {
						std::string err = fmt_err(path1.path, path2.path);
						return std::unexpected(lunas::error(err, lunas::error_type::presync_same_input_path));
					}
#ifdef REMOTE_ENABLED
					else if (path1.is_remote() && path2.is_remote() && path1.path == path2.path) {
						const std::string hostname1 = path1.sftp->get_hostname();
						const std::string hostname2 = path2.sftp->get_hostname();
						if (hostname1 == hostname2) {
							std::string err = fmt_err(path1.sftp->get_ip(), path2.sftp->get_ip());
							return std::unexpected(
							    lunas::error(err, lunas::error_type::presync_same_input_path));
						}
					}
#endif // REMOTE_ENABLED
				end:
					in++;
				}
				out++;
			}

			return std::monostate();
		}
	}
}
