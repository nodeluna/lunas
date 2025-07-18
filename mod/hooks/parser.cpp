module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <expected>
#	include <string>
#	include <vector>
#endif

export module lunas.hooks:parser;
export import lunas.error;

export namespace lunas
{
	std::expected<std::vector<std::pair<size_t, size_t>>, lunas::error> hook_parser(const std::string& command)
	{
		std::vector<std::pair<size_t, size_t>> input_variables;

		bool				       found_open_bracket  = false;
		bool				       found_close_bracket = false;
		bool				       escape_bracket	   = false;

		for (size_t i = 0; i < command.size(); i++)
		{
			if (command[i] == '{' && not found_open_bracket)
			{
				found_open_bracket = true;
				input_variables.push_back({i, 0});
			}
			else if (command[i] == '{' && not escape_bracket)
			{
				if (i < command.size() - 1 && command[i + 1] == '{')
				{
					escape_bracket = true;
				}
				else
				{
					return std::unexpected(lunas::error("expected '}' but found '{'", lunas::error_type::parsing));
				}
			}
			else if (command[i] == '}' && not escape_bracket)
			{
				if (i < command.size() - 1 && command[i + 1] == '}')
				{
					escape_bracket = true;
					if (i == command.size() - 2)
					{
						return std::unexpected(
						    lunas::error("expected '}' but reach end of string", lunas::error_type::parsing));
					}
				}
				else if (found_open_bracket)
				{
					found_open_bracket = false;
				}
				else if (not found_open_bracket)
				{
					return std::unexpected(lunas::error("expected '{' but found '}'", lunas::error_type::parsing));
				}
				else
				{
					found_close_bracket = true;
				}
			}
			else if (found_open_bracket && not found_close_bracket)
			{
				if (escape_bracket)
				{
					escape_bracket = false;
				}
				else if (i == command.size() - 1)
				{
					return std::unexpected(
					    lunas::error("expected '}' but reach end of string", lunas::error_type::parsing));
				}
				input_variables.back().second = i + 1;
			}
			else if (not found_open_bracket && found_close_bracket)
			{
				return std::unexpected(lunas::error("expected '{' but found '}'", lunas::error_type::parsing));
			}
		}

		return input_variables;
	}
}
