#ifndef LUCO
#define LUCO

#include <string>
#include <map>
#include <expected>

namespace luco {
	constexpr char newline	= '\n';
	constexpr char newline2 = ';';

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

#endif // LUCO
