module;

#include <system_error>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string>
#	include <map>
#	include <expected>
#	include <array>
#	include <fstream>
#	include <stack>
#	include <cstdlib>
#	include <cstring>
#	include <cerrno>
#	include <print>
#endif

export module lunas.config.file:luco;
import lunas.stdout;

export namespace lunas {
	namespace config_file {
		class luco {
			public:
				explicit luco(const std::string& path);

				virtual ~luco() {
				}

				const std::string&				  any_errors(void);
				const std::multimap<std::string, std::string>&	  parse(void);
				const std::multimap<std::string, std::string>&	  get_map(void);
				std::multimap<std::string, std::string>		  preset(const std::string& name);
				std::multimap<std::string, std::string>::iterator find_preset(const std::string& name);

			protected:
				virtual std::string				read_file(const std::string& path);
				virtual size_t					get_token_type(const std::string& data, const size_t& i);
				virtual std::expected<std::string, std::string> reg_nest(const std::string& data, size_t& i);
				virtual std::pair<std::string, std::string>	reg_optval(const std::string& data, size_t& i);
				virtual std::string				pop_parent_nest(const std::string& parent_nest);
				virtual void					strerror(const std::string& err, const int& code);
				virtual bool duplicate_nested_nest(std::string& parent_nest, const std::string& nest);
				virtual bool duplicate_nest(const std::string& nest_name, const size_t& line_number);

				std::multimap<std::string, std::string> ldata;
				std::string				data;
				std::string				error;
				unsigned int				counter	    = 2;
				size_t					line_number = 1;
		};
	}
}

namespace lunas {
	namespace config_file {
		enum token_type {
			NEST_NAME		     = 1 << 0,
			OPTION_VALUE		     = 1 << 1,
			OPTION			     = 1 << 2,
			VALUE			     = 1 << 3,
			NEST			     = 1 << 4,
			END_NEST		     = 1 << 5,
			EMPTY_LINE		     = 1 << 6,
			SYNTAX_ERROR_SIGN	     = 1 << 7,
			SYNTAX_ERROR_CLOSING_BRACKET = 1 << 8,
			COMMENT			     = 1 << 9,
			UNKNOWN			     = 1 << 10
		};

		namespace lstring {
			constexpr char newline	= '\n';
			constexpr char newline2 = ';';

			bool is_line_empty(const std::string& string, const size_t& start) {
				for (size_t index = start; index < string.size(); index++) {
					if (string[index] == newline || string[index] == newline2)
						return true;
					else if (string[index] != ' ' && string[index] != '\t')
						return false;
				}

				return true;
			}

			size_t get_end_line(const std::string& data, const size_t& start, size_t& line_number) {
				if (data.empty())
					return 0;

				size_t index = start;
				for (; index < data.size(); index++) {
					if (data[index] == newline) {
						line_number++;
						break;
					} else if (data[index] == newline2 && (index > 0 && data[index - 1] != '\\'))
						break;
					else if (data[index] == '}')
						break;
				}

				return index;
			}

			bool is_comment(const std::string& data, const size_t& start) {
				for (size_t index = start; index < data.size(); index++) {
					if (data[index] == '#')
						return true;
					else if (data[index] == ' ' || data[index] == '\t')
						continue;
					else
						return false;
				}

				return false;
			}

			void strip(std::string& data) {
				if (data.empty())
					return;

				for (size_t i = data.size() - 1;; i--) {
					if (data[i] == ' ' || data[i] == '\t')
						data.pop_back();
					else
						break;

					if (i == 0)
						break;
				}
			}

			size_t endline(const std::string& data, const size_t& start) {
				if (data.empty())
					return 0;

				size_t index = start;
				for (; index < data.size(); index++) {
					if (data[index] == newline)
						break;
					else if (data[index] == newline2 && (index > 0 && data[index - 1] != '\\'))
						break;
				}

				return index;
			}

		}

		luco::luco(const std::string& path) {
			data = luco::read_file(path);
		}

		std::multimap<std::string, std::string> luco::preset(const std::string& name) {
			std::multimap<std::string, std::string> preset_map;

			for (const auto& i : ldata) {
				if (i.first.substr(0, name.size() + 2) == (name + "::") && i.first != (name + "::"))
					preset_map.insert({i.first.substr(name.size() + 2, i.first.size()), i.second});
			}

			return preset_map;
		}

		std::multimap<std::string, std::string>::iterator luco::find_preset(const std::string& name) {
			return ldata.find(name + "::");
		}

		const std::multimap<std::string, std::string>& luco::get_map(void) {
			return ldata;
		}

		const std::string& luco::any_errors(void) {
			return error;
		}

		std::string luco::read_file(const std::string& config_file) {
			constexpr int		      BUFFER_SIZE = 262144;
			std::array<char, BUFFER_SIZE> buffer;
			unsigned long int	      position = 0;

			std::fstream conf_file;
			conf_file.open(config_file, std::ios::in);
			if (conf_file.is_open() == false)
				luco::strerror("couldn't open config file '" + config_file + "', " + std::strerror(errno), -1);

			while (conf_file.eof() == false) {
				conf_file.seekg(position);
				if (conf_file.fail() == true || conf_file.bad() == true)
					luco::strerror("error reading config file '" + config_file + "', " + std::strerror(errno), -1);

				conf_file.read(buffer.data(), BUFFER_SIZE);
				if (conf_file.bad() == true)
					luco::strerror("error reading config file '" + config_file + "', " + std::strerror(errno), -1);

				position += conf_file.gcount();
				data.append(buffer.data(), conf_file.gcount());
			}

			conf_file.close();
			if (conf_file.is_open() == true)
				lunas::warn("couldn't close config file '{}', ", config_file, std::strerror(errno));

			return data;
		}

		size_t luco::get_token_type(const std::string& data, const size_t& start) {
			size_t endline = lstring::endline(data, start);

			if (lstring::is_line_empty(data, start))
				return token_type::EMPTY_LINE;

			size_t token = 0;

			if (lstring::is_comment(data, start))
				token |= token_type::COMMENT;

			size_t i;

			if ((i = data.find('{', start)) != data.npos && i < endline)
				token |= token_type::NEST_NAME;

			if ((i = data.find('=', start)) != data.npos && i < endline)
				token |= token_type::OPTION_VALUE;

			if ((i = data.find('}', start)) != data.npos && i < endline)
				token |= token_type::END_NEST;

			if (token == 0)
				token |= token_type::UNKNOWN;

			return token;
		}

		std::expected<std::string, std::string> luco::reg_nest(const std::string& data, size_t& i) {
			size_t	    endline = lstring::endline(data, i);
			std::string temp;
			bool	    spaces_begin = true, spaces_end = false;

			for (size_t& j = i; j < endline; j++) {
				if (data[j] == '{') {
					j++;
					break;
				} else if ((data[j] == ' ' || data[j] == '\t') && spaces_begin)
					continue;
				else if ((data[j] == ' ' || data[j] == '\t') && not spaces_begin) {
					spaces_end = true;
					continue;
				} else if ((data[j] != ' ' && data[j] != '\t') && spaces_end)
					return std::unexpected("nests name can't have empty spaces");
				else if (data[j] == '\\' && data[j + 1] == ';')
					continue;

				temp += data[j];
				spaces_begin = false;
			}

			return temp.append("::");
		}

		std::pair<std::string, std::string> luco::reg_optval(const std::string& data, size_t& i) {
			size_t	    endline = lstring::endline(data, i);
			std::string option, value;

			bool first_char = false;
			for (; i < endline; i++) {
				if (data[i] == '=') {
					i++;
					break;
				} else if ((data[i] == ' ' || data[i] == '\t') && not first_char)
					continue;
				if (data[i] == '\\' && data[i + 1] == ';')
					continue;
				option += data[i];
				first_char = true;
			}

			first_char = false;
			for (; i < endline; i++) {
				if (data[i] == '}')
					break;
				else if ((data[i] == ' ' || data[i] == '\t') && not first_char)
					continue;
				else if (data[i] == '\\' && data[i + 1] == ';')
					continue;

				value += data[i];
				first_char = true;
			}

			lstring::strip(option);
			lstring::strip(value);

			return std::make_pair(option, value);
		}

		std::string luco::pop_parent_nest(const std::string& parent_nest) {
			if (parent_nest.empty())
				return "";

			bool   poped = false;
			size_t i     = parent_nest.size() - 1;
			for (;; i--) {
				if (parent_nest[i] == ':' && !poped) {
					poped = true;
					i--;
				} else if (parent_nest[i] == ':' && poped) {
					i++;
					break;
				}

				if (i == 0)
					break;
			}

			return parent_nest.substr(0, i);
		}

		void luco::strerror(const std::string& err, const int& code) {
			if (code == -1) {
				throw std::runtime_error(err);
			}
		}

		bool luco::duplicate_nested_nest(std::string& parent_nest, const std::string& nest) {
			auto itr = ldata.find(parent_nest + nest);
			if (itr != ldata.end() && parent_nest.find("::") != parent_nest.npos) {
				parent_nest += std::to_string(counter) + "_";
				counter++;
				return true;
			} else
				counter = 2;
			return false;
		}

		bool luco::duplicate_nest(const std::string& nest_name, const size_t& line_number) {
			if (auto it = ldata.find(nest_name); it != ldata.end()) {
				luco::strerror("duplicate nest '" + nest_name.substr(0, nest_name.size() - 2) +
						   "' line: " + std::to_string(line_number),
				    -1);
				return false;
			}
			return true;
		}

		const std::multimap<std::string, std::string>& luco::parse() {
			std::stack<std::pair<std::string, size_t>> nest_stack;
			std::string				   parent_nest	 = "";
			bool					   skipping_nest = false;

			for (size_t i = 0; i < data.size(); i++) {
				i		 = std::min(data.find_first_not_of("\t ", i), data.size() - 1);
				size_t tokentype = luco::get_token_type(data, i);

				if (tokentype & token_type::COMMENT || skipping_nest) {
					if (tokentype & token_type::NEST_NAME) {
						skipping_nest	      = true;
						auto	    nest_name = reg_nest(data, i);
						std::string nest_val  = nest_name ? nest_name.value() : nest_name.error();
						nest_stack.push(
						    {nest_val.find("::") != nest_val.npos ? nest_val.substr(0, nest_val.size() - 2)
											  : nest_val,
							line_number});
					}
					if (tokentype & token_type::END_NEST) {
						skipping_nest = false;
						nest_stack.pop();
					}
					goto end;
				}
				if (tokentype & token_type::NEST_NAME) {
					auto nest_name = reg_nest(data, i);
					if (not nest_name)
						luco::strerror(
						    "formatting error: " + nest_name.error() + ": " + std::to_string(line_number), -1);
					luco::duplicate_nest(nest_name.value(), line_number);

					nest_stack.push({nest_name.value().find("::") != nest_name.value().npos
							     ? nest_name.value().substr(0, nest_name.value().size() - 2)
							     : nest_name.value(),
					    line_number});

					luco::duplicate_nested_nest(parent_nest, nest_name.value());
					ldata.insert(std::make_pair(parent_nest + nest_name.value(), ""));
					parent_nest = parent_nest + nest_name.value();
				}
				if (tokentype & token_type::OPTION_VALUE) {
					std::pair<std::string, std::string> pair = reg_optval(data, i);
					if (pair.first.empty())
						luco::strerror("empty option '' line: " + std::to_string(line_number), -1);
					else if (pair.second.empty())
						luco::strerror(
						    "empty option '" + pair.first + "' line: " + std::to_string(line_number), -1);
					ldata.insert(std::make_pair(parent_nest + pair.first, pair.second));
				}
				if (tokentype & token_type::END_NEST) {
					if (nest_stack.empty())
						luco::strerror(
						    "formatting error: extra seperator, line: " + std::to_string(line_number), -1);
					else
						nest_stack.pop();
					parent_nest = pop_parent_nest(parent_nest);
				}
				if (tokentype == token_type::UNKNOWN)
					luco::strerror("formatting error: missing seperator, line: " + std::to_string(line_number), -1);
			end:
				i = lstring::get_end_line(data, i, line_number);
			}

			if (!nest_stack.empty()) {
				luco::strerror("missing closing bracket in nest '" + nest_stack.top().first +
						   "' line: " + std::to_string(nest_stack.top().second),
				    -1);
			}

			return ldata;
		}
	}
}
