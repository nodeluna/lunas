#ifndef LUCO
#define LUCO

#include <string>
#include <map>

namespace luco {
	enum class token_type {
		NEST_NAME,
		OPTION_VALUE,
		OPTION,
		VALUE,
		NEST,
		END_NEST,
		EMPTY_LINE,
		SYNTAX_ERROR_SIGN,
		SYNTAX_ERROR_CLOSING_BRACKET,
		UNKNOWN
	};

	class luco {
		public:
			explicit						luco(const std::string& path);
			const std::string&					any_errors(void);
			const std::multimap<std::string, std::string>&		parse(void);
			const std::multimap<std::string, std::string>&		get_map(void);
			std::multimap<std::string, std::string>			preset(const std::string& name);
			std::multimap<std::string, std::string>::iterator	find_preset(const std::string& name);

		protected:
			virtual std::string					read_file(const std::string& path);
			virtual token_type					get_token_type(const std::string& data, const size_t& i);
			virtual std::string					reg_nest(const std::string& data, const size_t& i);
			virtual std::pair<std::string, std::string>		reg_optval(const std::string& data, const size_t& i);
			virtual std::string					pop_parent_nest(const std::string& parent_nest);
			virtual void						strerror(const std::string& err, const int& code);
			virtual bool						duplicate_nested_nest(std::string& parent_nest, const std::string& nest);
			virtual bool						duplicate_nest(const std::string& nest_name, const size_t& line_number);

		std::multimap<std::string, std::string>				ldata;
		std::string							data;
		std::string							error;
		unsigned int							counter = 2;
	};
}

#endif // LUCO
