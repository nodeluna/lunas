#pragma once

#include <libssh/sftp.h>
#include <libssh/libssh.h>

#include <string>

enum key_type_t {
	none	    = 1 << 0,
	public_key  = 1 << 1,
	private_key = 1 << 2,
};

struct ssh_key_data {
		std::string	  path;
		ssh_auth_callback auth_fn;
		void*		  userdata   = nullptr;
		char*		  passphrase = nullptr;
		key_type_t	  key_type   = key_type_t::none;
};

namespace raii
{
	namespace sftp
	{
		class channel {
				ssh_channel* _channel;

			public:
				explicit channel(ssh_channel* channel_);
				channel(const channel&)		   = delete;
				channel& operator=(const channel&) = delete;
				~channel();
		};
	}

	namespace ssh
	{
		class key {
				ssh_key key_t;
				bool	free_key = false;
				int	retry	 = 3;

			public:
				explicit key();
				int	       import_key(const struct ssh_key_data& data);
				const ssh_key& get();
				const int&     get_retry_countdown();
				// void	       set_retry_countdown(const int& retries);
				~key();
		};
	}
}
