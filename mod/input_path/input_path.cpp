module;

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <variant>
#include <expected>
#include <cstddef>

#include <print>

export module lunas.ipath;
import lunas.config.options;
import lunas.sftp;
import lunas.path;

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

				void increment_stats_synced_files() {
					sync_stats.synced_files++;
				}

				void increment_stats_synced_dirs() {
					sync_stats.synced_dirs++;
				}

				void increment_stats_removed_files() {
					sync_stats.removed_files++;
				}

				void increment_stats_removed_dirs() {
					sync_stats.removed_dirs++;
				}

				void increment_stats_removed_size(const std::uintmax_t size) {
					sync_stats.removed_size += size;
				}

				void increment_stats_synced_size(const std::uintmax_t size) {
					sync_stats.synced_size += size;
				}

				input_path(input_path&& other) noexcept {
					this->sftp = std::move(other.sftp);
					other.sftp.reset();
					this->path = std::move(other.path);
					srcdest	   = other.srcdest;
					sync_stats = std::move(other.sync_stats);
				}

				std::expected<std::monostate, lunas::error> init_sftp(const struct session_data& data) {
					try {
						sftp = std::make_unique<lunas::sftp>(data);
					} catch (const std::exception& e) {
						throw;
					}
					auto ok = sftp->absolute_path();
					if (ok) {
						path = ok.value();
						lunas::path::append_seperator(path);
					} else
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

				bool is_src_only() const {
					return srcdest == srcdest::src;
				}

				bool is_dest_only() const {
					return srcdest == srcdest::dest;
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
		private:
			std::vector<struct lunas::ipath::input_path> ipaths;

		public:
			struct lunas::config::options options;

			const std::vector<struct lunas::ipath::input_path>& get_ipaths() const {
				return ipaths;
			}

			struct lunas::ipath::input_path& get_ipath(size_t index) {
				return ipaths.at(index);
			}

			void ipaths_emplace_back(struct lunas::ipath::input_path&& ipath) {
				ipaths.emplace_back(std::move(ipath));
			}
	};
}
