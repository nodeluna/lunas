#pragma once

#include <string>
#include <fstream>
#include <queue>
#include <thread>
#include <cstring>
#include <cstdint>
#include <memory>
#include <utility>
#include <expected>

#include "../types.hpp"
#include "file_types/file_types.hpp"
#include "error/error.hpp"
#include "config/options.hpp"
#include "cppfs/cppfs.hpp"
#include "stdout/stdout.hpp"

#define LOCAL_BUFFER_SIZE 262144

struct lbuffque {
		std::vector<char> buffer;
		long int	  bytes_read = 0;

		explicit lbuffque(std::uint64_t size) : buffer(size)
		{
		}
};

namespace lunas
{
	class jthread {
		private:
			std::thread thread;

		public:
			jthread(const jthread&)		   = delete;
			jthread& operator=(const jthread&) = delete;

			template<typename function, typename... args_t>
			jthread(function&& func, args_t&&... args) : thread(std::forward<function>(func), std::forward<args_t>(args)...)
			{
			}

			~jthread()
			{
				if (thread.joinable())
				{
					thread.join();
				}
			}
	};

	void fstream_aio_read_begin(std::unique_ptr<std::fstream>& src_file, std::queue<lbuffque>* queue, std::uintmax_t position,
				    size_t queue_limit);
	struct lbuffque fstream_aio_wait_read(std::queue<lbuffque>& queue);

	namespace local_to_local
	{
		std::expected<struct syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
								  const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> link(const std::string& src, const std::string& dest,
								  const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> rfile(const std::string& src, const std::string& dest,
								   const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> mkdir(const std::string& src, const std::string& dest,
								   const struct syncmisc& misc);

		std::expected<struct syncstat, lunas::error> symlink(const std::string& src, const std::string& dest,
								     const struct syncmisc& misc);
	}
}
