module;

#include <cassert>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <vector>
#	include <memory>
#	include <expected>
#	include <variant>
#	include <type_traits>
#endif

export module lunas.hooks:hook;
export import lunas.file;
import :parser;
import :cmd;
import :types;

export namespace lunas
{
	template<typename hook_type>
	class hook;

	class pre {};

	class post {};

	using prehook  = hook<pre>;
	using posthook = hook<post>;

	struct hooks {
			std::vector<prehook>							 prehooks;
			std::vector<posthook>							 posthooks;
			std::variant<lunas::directory_entry, std::shared_ptr<lunas::attributes>> attributes;
	};

	template<typename attributes_type>
	concept attributes_type_concept =
	    std::is_same_v<attributes_type, lunas::directory_entry> || std::is_same_v<attributes_type, std::shared_ptr<lunas::attributes>>;

	template<typename hook_type>
	class hook {
		private:
			std::string			       command;
			std::vector<std::pair<size_t, size_t>> parsed_brackets;

			template<typename attributes_type_concept>
			std::expected<std::string, lunas::error>
			translate_attr_syntax_to_data(const std::string& parsed_bracket, const attributes_type_concept& attributes) const
			{
				std::string hook_type_msg = "[hook]";
				if constexpr (std::is_same_v<hook_type, class pre>)
				{
					hook_type_msg = "[prehook]";
				}
				else if constexpr (std::is_same_v<hook_type, class post>)
				{
					hook_type_msg = "[posthook]";
				}
				if constexpr (std::is_same_v<attributes_type_concept, lunas::directory_entry>)
				{
					assert(attributes.mtime.has_value() && attributes.file_type.has_value());
				}
				else
				{
					assert(attributes != nullptr);
				}

				if (parsed_bracket.find("attributes.mtime") != parsed_bracket.npos)
				{
					if constexpr (std::is_same_v<attributes_type_concept, lunas::directory_entry>)
					{
						return std::to_string(attributes.mtime.value());
					}
					else
					{
						return std::to_string(attributes->mtime());
					}
				}
				else if (parsed_bracket.find("attributes.size") != parsed_bracket.npos)
				{
					if constexpr (std::is_same_v<attributes_type_concept, lunas::directory_entry>)
					{
						return std::to_string(attributes.file_size);
					}
					else
					{
						return std::to_string(attributes->file_size());
					}
				}
				else if (parsed_bracket.find("name") != parsed_bracket.npos)
				{
					if constexpr (std::is_same_v<attributes_type_concept, lunas::directory_entry>)
					{
						return attributes.filename;
					}
					else
					{
						return attributes->name();
					}
				}
				else if (parsed_bracket.find("path") != parsed_bracket.npos)
				{
					if constexpr (std::is_same_v<attributes_type_concept, lunas::directory_entry>)
					{
						return attributes.path.string();
					}
					else
					{
						return attributes->path().string();
					}
				}
				else if (parsed_bracket.find("extension") != parsed_bracket.npos)
				{
					if constexpr (std::is_same_v<attributes_type_concept, lunas::directory_entry>)
					{
						return attributes.path.extension();
					}
					else
					{
						return attributes->path().extension();
					}
				}
				else if (parsed_bracket.find("attributes.type") != parsed_bracket.npos)
				{
					if constexpr (std::is_same_v<attributes_type_concept, lunas::directory_entry>)
					{
						return std::to_string(file_types_to_int(attributes.file_type.value()));
					}
					else
					{
						return std::to_string(file_types_to_int(attributes->file_type()));
					}
				}
				else
				{
					return std::unexpected(lunas::error(lunas::error_type::syntax,
									    "{} '{{{}' isn't a recognized hook option", hook_type_msg,
									    parsed_bracket));
				}
			}

			std::expected<std::string, lunas::error> translate_syntax_to_data(const std::string&  parsed_bracket,
											  const struct hooks& hooks) const
			{
				std::expected<std::string, lunas::error> ok;

				if (std::holds_alternative<lunas::directory_entry>(hooks.attributes))
				{
					ok = this->translate_attr_syntax_to_data(parsed_bracket,
										 std::get<lunas::directory_entry>(hooks.attributes));
				}
				else
				{
					ok = this->translate_attr_syntax_to_data(
					    parsed_bracket, std::get<std::shared_ptr<lunas::attributes>>(hooks.attributes));
				}

				return ok;
			}

			std::expected<std::pair<std::string, hook_action>, lunas::error> expand_and_execute(const struct hooks& hooks) const
			{
				std::string requested_data;
				size_t	    i		  = 0;
				std::string final_command = "";

				for (const auto& begin_end : parsed_brackets)
				{
					requested_data = command.substr(begin_end.first + 1, begin_end.second - begin_end.first + 1);

					final_command += command.substr(i, begin_end.first - i);
					final_command += "\"";
					auto data = this->translate_syntax_to_data(requested_data, hooks);
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

		public:
			hook()
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

			static std::expected<hook_action, lunas::error> pipe_hook(const struct hooks& hooks, const config::options& options)
			{
				std::string hook_type_msg = "[hook]";
				if constexpr (std::is_same_v<hook_type, class pre>)
				{
					hook_type_msg = "[prehook]";
				}
				else if constexpr (std::is_same_v<hook_type, class post>)
				{
					hook_type_msg = "[posthook]";
				}

				auto handle_hook_expansion =
				    [&](const class hook<hook_type>& hook) -> std::expected<hook_action, lunas::error>
				{
					auto ok = hook.expand_and_execute(hooks);
					if (not ok)
					{
						return std::unexpected(ok.error());
					}
					else
					{
						if (ok.value().second == lunas::hook_action::dont_sync)
						{
							lunas::printerr("{} {}", hook_type_msg, ok.value().first);
							return hook_action::dont_sync;
						}
						else if (ok.value().second == lunas::hook_action::sync && not ok.value().first.empty())
						{
							lunas::println(options.quiet, "{} {}", hook_type_msg, ok.value().first);
						}

						return hook_action::sync;
					}
				};

				if constexpr (std::is_same_v<hook_type, class pre>)
				{
					for (const auto& hook : hooks.prehooks)
					{
						auto ok = handle_hook_expansion(hook);
						if (not ok)
						{
							return std::unexpected(ok.error());
						}
						else if (ok.value() == hook_action::dont_sync)
						{
							return hook_action::dont_sync;
						}
					}
				}
				else if constexpr (std::is_same_v<hook_type, class post>)
				{
					for (const auto& hook : hooks.posthooks)
					{
						auto ok = handle_hook_expansion(hook);
						if (not ok)
						{
							return std::unexpected(ok.error());
						}
						else if (ok.value() == hook_action::dont_sync)
						{
							return hook_action::dont_sync;
						}
					}
				}
				else
				{
					static_assert(false, "unsupported tag for lunas::hook<tag>");
				}

				return hook_action::sync;
			}
	};
}
