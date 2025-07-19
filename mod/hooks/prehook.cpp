module;

#include <cassert>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <expected>
#	include <string>
#	include <memory>
#	include <variant>
#endif

export module lunas.hooks:pre;
import :parser;
import :cmd;
import :types;
import lunas.file_types;
export import lunas.file;

export namespace lunas
{
	class prehook {
		private:
			std::string				 command;
			std::vector<std::pair<size_t, size_t>>	 parsed_brackets;

			std::expected<std::string, lunas::error> translate_syntax_to_data(const std::string&		parsed_bracket,
											  const lunas::directory_entry& attributes) const
			{
				if (parsed_bracket.find("attributes.mtime") != parsed_bracket.npos)
				{
					return std::to_string(attributes.mtime.value());
				}
				else if (parsed_bracket.find("attributes.size") != parsed_bracket.npos)
				{
					return std::to_string(attributes.file_size);
				}
				else if (parsed_bracket.find("name") != parsed_bracket.npos)
				{
					return attributes.filename;
				}
				else if (parsed_bracket.find("path") != parsed_bracket.npos)
				{
					return attributes.path.string();
				}
				else if (parsed_bracket.find("extension") != parsed_bracket.npos)
				{
					return attributes.path.extension();
				}
				else if (parsed_bracket.find("attributes.type") != parsed_bracket.npos)
				{
					return std::to_string(file_types_to_int(attributes.file_type.value()));
				}
				else
				{
					return std::unexpected(lunas::error(
					    lunas::error_type::syntax, "[prehook] '{{{}' isn't a recognized hook option", parsed_bracket));
				}
			}

		public:
			prehook()
			{
			}

			std::expected<std::monostate, lunas::error> parse(const std::string& command)
			{
				this->command								 = command;
				std::expected<std::vector<std::pair<size_t, size_t>>, lunas::error> data = lunas::hook_parser(command);
				if (not data)
				{
					return std::unexpected(data.error());
				}

				parsed_brackets = std::move(data.value());

				return std::monostate();
			}

			std::expected<std::pair<std::string, hook_action>, lunas::error> operator|(const lunas::directory_entry& attr) const
			{
				std::string requested_data;
				size_t	    i		  = 0;
				std::string final_command = "";

				for (const auto& begin_end : parsed_brackets)
				{
					requested_data = command.substr(begin_end.first + 1, begin_end.second - begin_end.first + 1);

					final_command += command.substr(i, begin_end.first - i);
					final_command += "\"";
					auto data = this->translate_syntax_to_data(requested_data, attr);
					if (not data)
					{
						return std::unexpected(data.error());
					}
					final_command += data.value();
					final_command += "\"";

					i = begin_end.second + 1;
				}

				final_command += command.substr(i, command.size());

				return this->exec(final_command);
			}

			std::expected<std::pair<std::string, enum hook_action>, lunas::error> exec(const std::string& command) const
			{

				std::expected<std::pair<std::string, int>, lunas::error> cmd_return = lunas::cmd(command);
				if (not cmd_return)
				{
					return std::unexpected(cmd_return.error());
				}
				else if (cmd_return.value().second == 0)
				{
					return std::make_pair(cmd_return.value().first, hook_action::sync);
				}
				else
				{
					return std::make_pair(cmd_return.value().first, hook_action::dont_sync);
				}
			}

			static std::expected<hook_action, lunas::error> pipe_hook(const std::vector<prehook>&	prehooks,
										  const lunas::directory_entry& attributes,
										  const config::options&	options)
			{
				for (const auto& prehook : prehooks)
				{
					auto ok = prehook | attributes;
					if (not ok)
					{
						return std::unexpected(ok.error());
					}
					else
					{
						if (ok.value().second == lunas::hook_action::dont_sync)
						{
							lunas::printerr("[prehook] {}", ok.value().first);
							return hook_action::dont_sync;
						}
						else if (ok.value().second == lunas::hook_action::sync && not ok.value().first.empty())
						{
							lunas::println(options.quiet, "[prehook] {}", ok.value().first);
						}
					}
				}

				return hook_action::sync;
			}
	};
}
