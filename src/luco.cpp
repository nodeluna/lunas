#include <string>
#include <array>
#include <map>
#include <fstream>
#include <stack>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <limits>
#include "luco.h"
#include "log.h"


namespace lstring{
	size_t first_non_empty_char(const std::string& data, const size_t& start){
		size_t i;
		for(i = start; i < data.size(); i++){
			if(data[i] != ' ' && data[i] != '\t')
				break;
		}

		return i;
	}

	bool is_line_empty(const std::string& string, const size_t& start){
		for(size_t index = start; index < string.size() ; index++){
			if(string[index] == luco::newline)
				return true;
			else if(string[index] != ' ' && string[index] != '\t')
				return false;
		}

		return true;
	}

	size_t get_end_line(const std::string& data, const size_t& start){
		if(data.empty())
			return 0;

		for(size_t index = start; index < data.size() ; index++){
			if(data[index] == luco::newline)
				return index;
		}

		return data.size() - 1;
	}

	bool is_comment(const std::string& data, const size_t& start){
		for(size_t index = start; index < data.size() ; index++){
			if(data[index] == '#')
				return true;
			else if(data[index] == ' ' || data[index] == '\t')
				continue;
			else
				return false;
		}

		return false;
	}

	void strip(std::string& data){
		if(data.empty())
			return;

		for(size_t i = data.size() - 1; ; i--){
			if(data[i] == ' ' || data[i] == '\t')
				data.pop_back();
			else
				break;

			if(i == 0)
				break;
		}
	}
}


namespace luco {
	luco::luco(const std::string& path){
		data = luco::read_file(path);
	}

	std::multimap<std::string, std::string> luco::preset(const std::string& name){
		std::multimap<std::string, std::string> preset_map;

		for(const auto& i : ldata){
			if(i.first.substr(0, name.size() + 2) == (name + "::") && i.first != (name + "::"))
				preset_map.insert({i.first.substr(name.size() + 2, i.first.size()), i.second});
		}

		return preset_map;
	}

	std::multimap<std::string, std::string>::iterator luco::find_preset(const std::string& name){
		return ldata.find(name + "::");
	}

	const std::multimap<std::string, std::string>& luco::get_map(void){
		return ldata;
	}

	const std::string& luco::any_errors(void){
		return error;
	}

	std::string luco::read_file(const std::string& config_file){
		constexpr int BUFFER_SIZE = 262144;
		std::array<char, BUFFER_SIZE> buffer;
		unsigned long int position = 0;

		std::fstream conf_file;
		conf_file.open(config_file, std::ios::in);
		if(conf_file.is_open() == false){
			error = "couldn't open config file '" + config_file + "', " + std::strerror(errno);
			return "";
		}

		while(conf_file.eof() == false){
			conf_file.seekg(position);
			if(conf_file.fail() == true || conf_file.bad() == true){
				error = "error reading config file '" + config_file + "', " + std::strerror(errno);
				break;
			}

			conf_file.read(buffer.data(), BUFFER_SIZE);
			if(conf_file.bad() == true){
				error = "error reading config file '" + config_file + "', " + std::strerror(errno);
				break;
			}

			position += conf_file.gcount();
			data.append(buffer.data(), conf_file.gcount());
		}

		conf_file.close();
		if(conf_file.is_open() == true){
			llog::warn("couldn't close config file '" + config_file + "', " + std::strerror(errno));
		}

		return data;
	}

	size_t luco::get_token_type(const std::string& data, const size_t& start){
		size_t endline = data.find(newline, start);
		if(endline == data.npos)
			endline = data.size() - 1;

		size_t i;
		size_t token = 0;

		if(lstring::is_line_empty(data, start))
			return token_type::EMPTY_LINE;
		else if(lstring::is_comment(data, start)){
			token |= token_type::COMMENT;
			if((i = data.find('{', start)) != data.npos && i < endline)
				token |= token_type::NEST_NAME;
			if((i = data.find('}', start)) != data.npos && i < endline)
				token |= token_type::END_NEST;
			if((i = data.find('=', start)) != data.npos && i < endline)
				token |= token_type::OPTION_VALUE;

			return token;
		}

		if((i = data.find('{', start)) != data.npos && i < endline)
			token |= token_type::NEST_NAME;

		if((i = data.find('=', start)) != data.npos && i < endline){
			if(token != 0)
				token |= token_type::SYNTAX_ERROR_SIGN;
			token |= token_type::OPTION_VALUE;
		}

		if((i = data.find('}', start)) != data.npos && i < endline){
			if(token != 0)
				token |= token_type::SYNTAX_ERROR_CLOSING_BRACKET;
			token |= token_type::END_NEST;
		}

		if(token == 0)
			token |= token_type::UNKNOWN;

		return token;
	}

	std::string luco::reg_nest(const std::string& data, const size_t& i){
		size_t endline = data.find(newline, i);
		if(endline == data.npos)
			endline = data.size() - 1;

		std::string temp;
		bool word_break = false;
		for(size_t j = i; j < endline; j++){
			if(data[j] == '{')
				break;
			else if((data[j] == ' ' || data[j] == '\t') && !word_break){
				word_break = true;
				continue;
			}else if((data[j] == ' ' || data[j] == '\t') && word_break){
				return "";
			}

			temp += data[j];
		}

		return temp + "::";
	}

	std::pair<std::string, std::string> luco::reg_optval(const std::string& data, const size_t& i){
		size_t endline = data.find(newline, i);
		if(endline == data.npos)
			endline = data.size() - 1;
		std::string option, value;

		size_t j = i;
		bool first_char = false;
		for(; j < endline; j++){
			if(data[j] == '=')
				break;
			else if((data[j] == ' ' || data[j] == '\t') && not first_char)
				continue;
			option += data[j];
			first_char = true;
		}

		first_char = false;
		for(size_t k = j + 1; k < endline; k++){
			if((data[k] == ' ' || data[k] == '\t') && not first_char)
				continue;
			
			value += data[k];
			first_char = true;
		}

		lstring::strip(option);
		lstring::strip(value);

		return std::make_pair(option, value);
	}

	std::string luco::pop_parent_nest(const std::string& parent_nest){
		if(parent_nest.empty())
			return "";

		bool poped = false;
		size_t i = parent_nest.size() - 1;
		for(; ; i--){
			if(parent_nest[i] == ':' && !poped){
				poped = true;
				i--;
			}else if(parent_nest[i] == ':' && poped){
				i++;
				break;
			}

			if(i == 0)
				break;
		}

		return parent_nest.substr(0, i);
	}

	void luco::strerror(const std::string& err, const int& code){
		if(code == -1){
			llog::error(err);
			exit(1);
		}
	}

	bool luco::duplicate_nested_nest(std::string& parent_nest, const std::string& nest){
		auto itr = ldata.find(parent_nest + nest);
		if(itr != ldata.end() && parent_nest.find("::") != parent_nest.npos){
			parent_nest += std::to_string(counter) + "_";
			counter++;
			return true;
		}else
			counter = 2;
		return false;
	}

	bool luco::duplicate_nest(const std::string& nest_name, const size_t& line_number){
		if(auto it = ldata.find(nest_name); it != ldata.end()){
			luco::strerror("duplicate nest '" + nest_name.substr(0, nest_name.size() - 2) + "' line: " + std::to_string(line_number), -1);
			return false;
		}
		return true;
	}

	const std::multimap<std::string, std::string>& luco::parse(){
		std::stack<std::pair<std::string, size_t>> nest_stack;
		std::string parent_nest = "";
		size_t line_number = 1, fnechar;
		bool skipping_nest = false;

		for(size_t i = 0; i < data.size(); i++){
			fnechar = lstring::first_non_empty_char(data, i);

			size_t tokentype = luco::get_token_type(data, i);

			if(tokentype & token_type::COMMENT || skipping_nest){
				if(tokentype & token_type::NEST_NAME){
					skipping_nest = true;
					std::string nest_name = reg_nest(data, fnechar);
					nest_stack.push({nest_name.find("::") != nest_name.npos ? nest_name.substr(0, nest_name.size() - 2)
							: nest_name, line_number});
				}
				if(tokentype & token_type::END_NEST){
					skipping_nest = false;
					nest_stack.pop();
				}
			}else if(tokentype == token_type::NEST_NAME){
				std::string nest_name = reg_nest(data, fnechar);
				if(nest_name.empty())
					luco::strerror("formatting error: more than one nest name is provided: " + std::to_string(line_number), -1);
				luco::duplicate_nest(nest_name, line_number);

				nest_stack.push({nest_name.find("::") != nest_name.npos ? nest_name.substr(0, nest_name.size() - 2)
						: nest_name, line_number});

				luco::duplicate_nested_nest(parent_nest, nest_name);
				ldata.insert(std::make_pair(parent_nest + nest_name, ""));
				parent_nest = parent_nest + nest_name;

			}else if(tokentype == token_type::OPTION_VALUE){
				std::pair<std::string, std::string> pair = reg_optval(data, fnechar);
				if(pair.first.empty())
					luco::strerror("empty option '' line: " + std::to_string(line_number), -1);
				else if(pair.second.empty())
					luco::strerror("empty option '" + pair.first + "' line: " + std::to_string(line_number), -1);
				ldata.insert(std::make_pair(parent_nest + pair.first, pair.second));
			}else if(tokentype == token_type::END_NEST){
				if(nest_stack.empty())
					luco::strerror("formatting error: extra seperator, line: " + std::to_string(line_number), -1);
				else
					nest_stack.pop();
				parent_nest = pop_parent_nest(parent_nest);
			}else if(tokentype == token_type::UNKNOWN)
				luco::strerror("formatting error: missing seperator, line: " + std::to_string(line_number), -1);
			else if(tokentype & token_type::SYNTAX_ERROR_SIGN)
				luco::strerror("formatting error: option isn't on its own line, line: " + std::to_string(line_number), -1);
			else if(tokentype & token_type::SYNTAX_ERROR_CLOSING_BRACKET)
				luco::strerror("formatting error: closing bracket '}' isn't on its own line, line: " + std::to_string(line_number), -1);

			i = lstring::get_end_line(data, fnechar);
			line_number++;
		}

		if(!nest_stack.empty()){
			while(!nest_stack.empty()){
				llog::error("missing closing bracket in nest '" + nest_stack.top().first +
						"' line: " + std::to_string(nest_stack.top().second));
				nest_stack.pop();
			}
			exit(1);
		}

		return ldata;
	}
}
