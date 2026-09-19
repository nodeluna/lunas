#pragma once

#include <stdio.h>
#include <cerrno>
#include <sys/wait.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <expected>
#	include <string>
#	include <memory>
#	include <utility>
#	include <system_error>
#endif

#include "error/error.hpp"

namespace lunas
{
	class pipe {
		public:
			pipe(const std::string_view& command, const std::string_view& mode) : p(popen(command.data(), mode.data()))
			{
			}

			~pipe();

			std::expected<int, lunas::error> exit_status();

			FILE*				 data(void);

		private:
			FILE* p = nullptr;
	};

	std::expected<std::pair<std::string, int>, lunas::error> cmd(const std::string_view& command);
}
