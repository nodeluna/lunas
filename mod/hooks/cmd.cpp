#include <stdio.h>
#include <cerrno>
#include <sys/wait.h>

#include <expected>
#include <string>
#include <utility>
#include <system_error>

#include "cmd.hpp"

namespace lunas
{
	pipe::~pipe()
	{
		if (p != nullptr)
		{
			pclose(p);
		}
	}

	std::expected<int, lunas::error> pipe::exit_status()
	{
		if (p != nullptr)
		{
			int rt = pclose(p);
			p      = nullptr;
			return WEXITSTATUS(rt);
		}

		return std::unexpected(lunas::error("tried to get status of a pipe that is already closed"));
	}

	FILE* pipe::data(void)
	{
		return p;
	}

	std::expected<std::pair<std::string, int>, lunas::error> cmd(const std::string_view& command)
	{
		std::string output;
		class pipe  pipe_(command.data() + std::string(" 2>&1"), "r");
		if (not pipe_.data())
		{
			std::error_code ec = std::make_error_code(( std::errc ) errno);
			return std::unexpected(lunas::error(ec.message(), lunas::error_type::cmd_execution));
		}

		std::array<char, 1024> buffer;

		while (fgets(buffer.data(), buffer.size(), pipe_.data()) != NULL)
		{
			output.append(buffer.data());
		}

		if (not output.empty() && output.back() == '\n')
		{
			output.pop_back();
		}

		auto status = pipe_.exit_status();
		if (not status)
		{
			return std::unexpected(status.error());
		}

		return std::make_pair(output, status.value());
	}
}
