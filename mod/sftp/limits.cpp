module;

#include <stdexcept>
#include <string>
#include <libssh/sftp.h>

export module lunas.sftp:limits;

export namespace lunas {
	class sftp_limits {
		private:
			::sftp_limits_t limits = nullptr;

		public:
			sftp_limits(const sftp_session& sftp);
			~sftp_limits();
			uint64_t max_packet_length();
			uint64_t max_read_length();
			uint64_t max_write_length();
			uint64_t max_open_handles();
	};
}

namespace lunas {
	sftp_limits::sftp_limits(const sftp_session& sftp) {
		limits = ::sftp_limits(sftp);
		if (limits == NULL)
			throw std::runtime_error(std::string("couldn't get sftp_limits, ") + ssh_get_error(sftp->session));
	}

	uint64_t sftp_limits::max_packet_length() {
		return limits->max_packet_length;
	}

	uint64_t sftp_limits::max_read_length() {
		return limits->max_read_length;
	}

	uint64_t sftp_limits::max_write_length() {
		return limits->max_write_length;
	}

	uint64_t sftp_limits::max_open_handles() {
		return limits->max_open_handles;
	}

	sftp_limits::~sftp_limits() {
		if (limits != NULL) {
			sftp_limits_free(limits);
		}
	}
}
