#include <expected>
#include <variant>
#include <string>
#include <print>

#include "kvoption.hpp"

bool is_num(const std::string& x)
{
	return std::all_of(x.begin(), x.end(),
			   [](char c)
			   {
				   return std::isdigit(c);
			   });
}

namespace lunas
{
	enum class token_type {
		key,
		looking_for_equal,
		equal,
		value,
	};

	std::expected<luco::node, lunas::error> handle_simple_value(const std::string& key, const std::string& value,
								    const luco::value_type value_type, const std::string& type_name)
	{
		if (value_type == luco::value_type::string)
		{
			return luco::node(value);
		}
		else if (value_type == luco::value_type::integer || value_type == luco::value_type::double_t)
		{
			if (not is_num(value))
			{
				return std::unexpected(lunas::error(lunas::error_type::config_invalid_option,
								    "key '{}' accepts numbers, but found '{}' a non-number value", key,
								    value));
			}
			else if (value_type == luco::value_type::double_t)
			{
				return luco::node(std::stod(value));
			}
			else
			{
				return luco::node(std::stoll(value));
			}
		}
		else if (value_type == luco::value_type::boolean)
		{
			std::println("handle_value: value: boolean");
			return luco::node(value == "on" || value == "true" ? true : false);
		}
		else
		{
			return std::unexpected(lunas::error(lunas::error_type::config_invalid_option,
							    "not allowed type '{}' of '{}' for luco::value() in kv_options", type_name,
							    value));
		}
	}

	std::expected<std::monostate, lunas::error> handle_value(const std::string& value, std::pair<std::string, luco::node>& kv_pair)
	{
		luco::node_type type = kv_pair.second.type();

		if (type == luco::node_type::object)
		{

			return std::unexpected(
			    lunas::error(lunas::error_type::config_invalid_option, "luco::object type is not allowed in cli sub-options"));
		}
		else if (type == luco::node_type::array)
		{
			std::string temp_value;
			size_t	    index = 0, array_index = 0;
			for (const auto& i : value)
			{
				index++;
				auto insert_value = [&]() -> std::expected<std::monostate, lunas::error>
				{
					auto ok = kv_pair.second.try_at(array_index);

					if (ok)
					{
						std::string arrkey = std::format("array '{}' at index '{}'", kv_pair.first, array_index);
						auto	    node   = handle_simple_value(arrkey, temp_value, ok.value().get().valuetype(),
											 ok.value().get().value_type_name());
						if (not node)
						{
							return std::unexpected(node.error());
						}

						kv_pair.second.at(array_index) = node.value();
					}
					else
					{
						kv_pair.second.push_back(temp_value);
					}

					return std::monostate();
				};

				if (i == ',')
				{
					auto ok = insert_value();
					if (not ok)
					{
						return ok;
					}

					temp_value.clear();
					array_index++;
					continue;
				}
				temp_value += i;

				if (index == value.size())
				{
					auto ok = insert_value();
					if (not ok)
					{
						return ok;
					}
				}
			}
		}
		else
		{
			luco::value_type value_type = kv_pair.second.valuetype();
			auto		 ok = handle_simple_value(kv_pair.first, value, value_type, kv_pair.second.value_type_name());
			if (not ok)
			{
				return std::unexpected(ok.error());
			}

			kv_pair.second = ok.value();
		}

		return std::monostate();
	}

	std::expected<std::vector<std::pair<std::string, luco::node>>, lunas::error>
	kvoption_parser(const std::string& data, const std::map<std::string, luco::node>& kv_map)
	{
		std::vector<std::pair<std::string, luco::node>> kv_mapped;

		std::string					key, value;
		token_type					token = token_type::key;
		uintmax_t					index = 0;
		for (const auto& i : data)
		{
			index++;
			if (token == token_type::key && i != ' ' && i != '=')
			{
				key += i;
			}
			else if (token == token_type::key && i == ' ')
			{
				token = token_type::looking_for_equal;
			}
			else if (i == '=' && (token == token_type::key || token == token_type::looking_for_equal))
			{
				token = token_type::equal;
			}
			else if (token == token_type::looking_for_equal && i != '=' && i != ' ')
			{
				return std::unexpected(lunas::error(lunas::error_type::config_invalid_option,
								    "syntax error: expected '=' found: '{}', in key/value options '{}'", i,
								    data));
			}
			else if (i == ' ' && token == token_type::equal)
			{
				token = token_type::equal;
			}
			else if (i != ' ' && (token == token_type::equal || token == token_type::value))
			{
				token = token_type::value;
				value += i;
			}
			else if ((i == ' ' || index == data.size()) && token == token_type::value)
			{
				token	 = token_type::key;
				auto itr = kv_map.find(key);
				if (itr != kv_map.end())
				{
					kv_mapped.push_back(std::make_pair(itr->first, itr->second));
					std::pair<std::string, luco::node>& new_itr = kv_mapped.back();
					auto				    ok	    = handle_value(value, new_itr);
					if (not ok)
					{
						return std::unexpected(ok.error());
					}
				}
				else
				{
					return std::unexpected(lunas::error(lunas::error_type::config_invalid_option,
									    "unknown option '{}', in key/value options '{}'", key, data));
				}
				key.clear();
				value.clear();
			}
			else
			{
				std::string key_type_name;
				switch (token)
				{
					case token_type::value:
						key_type_name = "value";
						break;
					case token_type::key:
						key_type_name = "key";
						break;
					case token_type::looking_for_equal:
						key_type_name = "looking_for_equal";
						break;
					case token_type::equal:
						key_type_name = "equal";
						break;
				}
				return std::unexpected(
				    lunas::error(lunas::error_type::config_invalid_option,
						 "unexpected parsing error at '{}', in key/value options '{}'\ntoken type: '{}'", i, data,
						 key_type_name));
			}
		}

		return kv_mapped;
	}
}
