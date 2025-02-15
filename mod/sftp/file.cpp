module;

#include <libssh/sftp.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <expected>
#	include <variant>
#	include <string>
#	include <vector>
#	include <memory>
#	include <stdexcept>
#	include <filesystem>
#endif

export module lunas.sftp:file;
export import :error;
import :log;
export import lunas.error;

export namespace lunas {
	class sftp_file;

	class sftp_aio {
		private:
			::sftp_aio		  aio		  = nullptr;
			int			  bytes_requested = 0;
			[[nodiscard]] ::sftp_aio* get_aio_handle();

			friend class lunas::sftp_file;

		public:
			sftp_aio();
			void set_bytes_requested(int nbytes);
			int  get_bytes_requested();
			~sftp_aio();
	};

	class sftp_file {
		private:
			::sftp_file file = nullptr;

		public:
			sftp_file(const sftp_session& sftp, const std::string& path, int access_type, mode_t mode);
			sftp_file(const sftp_session& sftp, const std::string& path, int access_type, std::filesystem::perms mode);
			~sftp_file();
			[[nodiscard]] std::expected<std::unique_ptr<lunas::sftp_aio>, lunas::error> aio_begin_write(
			    const std::vector<char>& buffer, size_t length);
			[[nodiscard]] std::expected<int, lunas::error> aio_wait_write(std::unique_ptr<lunas::sftp_aio>& aio);

			[[nodiscard]] std::expected<std::unique_ptr<lunas::sftp_aio>, lunas::error> aio_begin_read(size_t length);
			[[nodiscard]] std::expected<int, lunas::error>				    aio_wait_read(
							 std::unique_ptr<lunas::sftp_aio>& aio, std::vector<char>& buffer, size_t length);

			std::expected<std::monostate, lunas::error> fsync();
			std::expected<std::monostate, lunas::error> seek64(uint64_t new_offset);
	};
}

namespace lunas {
	sftp_aio::sftp_aio() {
	}

	sftp_aio::~sftp_aio() {
		if (aio != NULL) {
			sftp_aio_free(aio);
		}
	}

	void sftp_aio::set_bytes_requested(int nbytes) {
		bytes_requested = nbytes;
	}

	int sftp_aio::get_bytes_requested() {
		return bytes_requested;
	}

	::sftp_aio* sftp_aio::get_aio_handle() {
		return &aio;
	}

	sftp_file::sftp_file(const sftp_session& sftp, const std::string& path, int access_type, mode_t mode) {
		file = sftp_open(sftp, path.c_str(), access_type, mode);
		if (file == NULL) {
			throw std::runtime_error("couldn't open file '" + path + "', " + ssh_get_error(sftp->session));
		}
	}

	sftp_file::sftp_file(const sftp_session& sftp, const std::string& path, int access_type, std::filesystem::perms mode) {
		file = sftp_open(sftp, path.c_str(), access_type, ( unsigned int ) mode);
		if (file == NULL) {
			throw std::runtime_error("couldn't open file '" + path + "', " + ssh_get_error(sftp->session));
		}
	}

	sftp_file::~sftp_file() {
		if (file != nullptr) {
			sftp_close(file);
			file = nullptr;
		}
	}

	[[nodiscard]] std::expected<std::unique_ptr<lunas::sftp_aio>, lunas::error> sftp_file::aio_begin_write(
	    const std::vector<char>& buffer, size_t length) {
		std::unique_ptr<lunas::sftp_aio> aio		 = std::make_unique<lunas::sftp_aio>();
		int				 bytes_requested = sftp_aio_begin_write(file, buffer.data(), length, aio->get_aio_handle());
		if (bytes_requested == SSH_ERROR)
			return std::unexpected(ssh_error(file->sftp, "couldn't sftp_aio_begin_write"));

		aio->set_bytes_requested(bytes_requested);
		return aio;
	}

	[[nodiscard]] std::expected<int, lunas::error> sftp_file::aio_wait_write(std::unique_ptr<lunas::sftp_aio>& aio) {
		int bytes_written = sftp_aio_wait_write(aio->get_aio_handle());

		if (bytes_written == SSH_ERROR)
			return std::unexpected(ssh_error(file->sftp, "error in sftp_aio_wait_write"));
		else if (bytes_written == SSH_AGAIN)
			return std::unexpected(lunas::error("ssh_again", lunas::error_type::ssh_again));

		return bytes_written;
	}

	[[nodiscard]] std::expected<std::unique_ptr<lunas::sftp_aio>, lunas::error> sftp_file::aio_begin_read(size_t length) {
		std::unique_ptr<lunas::sftp_aio> aio		 = std::make_unique<lunas::sftp_aio>();
		int				 bytes_requested = sftp_aio_begin_read(file, length, aio->get_aio_handle());
		if (bytes_requested == SSH_ERROR)
			return std::unexpected(ssh_error(file->sftp, "couldn't sftp_aio_begin_read"));

		aio->set_bytes_requested(bytes_requested);
		return aio;
	}

	[[nodiscard]] std::expected<int, lunas::error> sftp_file::aio_wait_read(
	    std::unique_ptr<lunas::sftp_aio>& aio, std::vector<char>& buffer, size_t length) {
		int bytes_written = sftp_aio_wait_read(aio->get_aio_handle(), buffer.data(), length);

		if (bytes_written == SSH_ERROR)
			return std::unexpected(ssh_error(file->sftp, "error in sftp_aio_wait_read"));
		else if (bytes_written == SSH_AGAIN)
			return std::unexpected(lunas::error("ssh_again", lunas::error_type::ssh_again));

		return bytes_written;
	}

	std::expected<std::monostate, lunas::error> sftp_file::fsync() {
		if (sftp_fsync(file) != SSH_OK)
			return std::unexpected(ssh_error(file->sftp, fmt::err_path("failed to fsync sftp_file", file->name)));

		return std::monostate();
	}

	std::expected<std::monostate, lunas::error> sftp_file::seek64(uint64_t new_offset) {
		if (sftp_seek64(file, new_offset) != SSH_OK)
			return std::unexpected(ssh_error(file->sftp, fmt::err_path("couldn't move file pointer", file->name)));

		return std::monostate();
	}
}
