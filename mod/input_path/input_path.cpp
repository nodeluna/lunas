module;

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <variant>
#include <expected>

#include <print>

export module lunas.ipath;
import lunas.config.options;
import lunas.sftp;

export namespace lunas {
	namespace ipath {
		struct sync_stats {
				std::uintmax_t synced_size   = 0;
				std::uintmax_t synced_files  = 0;
				std::uintmax_t synced_dirs   = 0;
				std::uintmax_t removed_files = 0;
				std::uintmax_t removed_dirs  = 0;
				std::uintmax_t removed_size  = 0;
		};

		enum class srcdest {
			src,
			dest,
			srcdest,
		};

		struct local_path {
				std::string	      path;
				lunas::ipath::srcdest srcdest;
		};

		struct remote_path {
				struct lunas::session_data session_data;
				lunas::ipath::srcdest	   srcdest;
		};

		struct input_path {
				std::string		     path;
				srcdest			     srcdest;
				struct sync_stats	     sync_stats;
				std::unique_ptr<lunas::sftp> sftp;

				input_path(const std::string& name) : path(name) {
				}

				input_path() {
				}

				input_path(input_path&& other) noexcept {
					this->sftp = std::move(other.sftp);
					other.sftp.reset();
					this->path = std::move(other.path);
				}

				std::expected<std::monostate, lunas::error> init_sftp(const struct session_data& data) {
					try {
						sftp = std::make_unique<lunas::sftp>(data);
					} catch (const std::exception& e) {
						throw;
					}
					auto ok = sftp->absolute_path();
					if (ok)
						path = ok.value();
					else
						return std::unexpected(ok.error());

					return std::monostate();
				}

				bool is_src() const {
					return srcdest == srcdest::src || srcdest == srcdest::srcdest;
				}

				bool is_dest() const {
					return srcdest == srcdest::dest || srcdest == srcdest::srcdest;
				}

				bool is_srcdest() const {
					return srcdest == srcdest::srcdest;
				}

				bool is_remote() const {
					return sftp != nullptr;
				}

				bool operator=(const input_path& other) const {
					return path == other.path;
				}

				bool operator<(const input_path& other) const {
					return path < other.path;
				}

				bool operator>(const input_path& other) const {
					return path > other.path;
				}
		};
	}

	struct parsed_data {
			std::vector<struct lunas::ipath::input_path> ipaths;
			struct lunas::config::options		     options;
	};
}
