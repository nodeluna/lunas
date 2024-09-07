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
			if(string[index] == '\n')
				return true;
			else if(string[index] != ' ' && string[index] != '\t')
				return false;
		}

		return false;
	}

	size_t get_next_line(const std::string& data, const size_t& start){
		for(size_t index = start; index < data.size() ; index++){
			if(data[index] == '\n')
				return index + 1;
		}
		return std::numeric_limits<size_t>::max();
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

	token_type luco::get_token_type(const std::string& data, const size_t& start){
		size_t endline = data.find('\n', start);
		if(endline == data.npos)
			endline = data.size() - 1;

		if(lstring::is_line_empty(data, start))
			return token_type::EMPTY_LINE;

		size_t i;
		if((i = data.find('{', start)) != data.npos && i < endline)
			return token_type::NEST_NAME;
		else if((i = data.find('}', start)) != data.npos && i < endline)
			return token_type::END_NEST;
		else if((i = data.find('=', start)) != data.npos && i < endline)
			return token_type::OPTION_VALUE;
		else
			return token_type::UNKNOWN;
	}

	std::string luco::reg_nest(const std::string& data, const size_t& i){
		size_t newline = data.find('\n', i);
		if(newline == data.npos)
			newline = data.size() - 1;

		std::string temp;
		bool word_break = false;
		for(size_t j = i; j < newline; j++){
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
		size_t newline = data.find('\n', i);
		if(newline == data.npos)
			newline = data.size() - 1;
		std::string option, value;
		bool word_break = false;
		size_t j = i;
		for(; j < newline; j++){
			if(data[j] == '=')
				break;
			else if((data[j] == ' ' || data[j] == '\t') && !word_break){
				word_break = true;
				continue;
			}else if((data[j] == ' ' || data[j] == '\t') && word_break){
				return std::make_pair<std::string, std::string>("", "");
			}
			option += data[j];
		}

		bool first_char = false;
		word_break = false;

		for(size_t k = j + 1; k < newline; k++){
			if(data[k] == '=')
				break;
			else if((data[k] == ' ' || data[k] == '\t') && !word_break){
				if(first_char)
					word_break = true;
				continue;
			}else if((data[k] == ' ' || data[k] == '\t') && word_break)
				return std::make_pair(option, "");
			
			value += data[k];
			first_char = true;
		}

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

		for(size_t i = 0; i < data.size(); i++){
			token_type tokentype = luco::get_token_type(data, i);
			if(tokentype == token_type::EMPTY_LINE)
				goto end;
			fnechar = lstring::first_non_empty_char(data, i);

			if(tokentype == token_type::NEST_NAME){
				std::string nest_name = reg_nest(data, fnechar);
				if(nest_name.empty())
					luco::strerror("formatting error: more than one nest name is provided: " + std::to_string(line_number), -1);
				luco::duplicate_nest(nest_name, line_number);
				nest_stack.push({nest_name.find("::") != nest_name.npos ? nest_name.substr(0, nest_name.size() - 2) : nest_name, line_number});
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
				nest_stack.pop();
				parent_nest = pop_parent_nest(parent_nest);
			}else if(tokentype == token_type::UNKNOWN)
				luco::strerror("formatting error: missing seperator, line: " + std::to_string(line_number), -1);

			i = lstring::get_next_line(data, fnechar) - 1;
end:
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
