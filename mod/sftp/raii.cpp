#include <libssh/sftp.h>
#include <libssh/libssh.h>

#include <string>

#include "raii.hpp"

namespace raii
{
	namespace sftp
	{
		channel::channel(ssh_channel* channel_) : _channel(channel_)
		{
		}

		channel::~channel()
		{
			if (*_channel == NULL)
			{
				return;
			}
			ssh_channel_close(*_channel);
			ssh_channel_free(*_channel);
		}
	}

	namespace ssh
	{
		key::key()
		{
		}

		int key::import_key(const struct ssh_key_data& data)
		{
			int rc = -1;
			if (retry <= 0)
			{
				return SSH_AUTH_DENIED;
			}

			if (data.key_type & key_type_t::public_key)
			{
				rc = ssh_pki_import_pubkey_file(data.path.c_str(), &key_t);
			}
			else if (data.key_type & key_type_t::private_key)
			{
				rc = ssh_pki_import_privkey_file(data.path.c_str(), data.passphrase, data.auth_fn, data.userdata, &key_t);
			}

			if (rc == SSH_OK)
			{
				free_key = true;
			}

			retry--;

			return rc;
		}

		const ssh_key& key::get()
		{
			return key_t;
		}

		// void key::set_retry_countdown(const int& retries) {
		// 	retry = retries;
		// }

		const int& key::get_retry_countdown()
		{
			return retry;
		}

		key::~key()
		{
			if (free_key)
			{
				ssh_key_free(key_t);
			}
		}
	}
}
