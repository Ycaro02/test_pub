/* #include <cctype> */
#include "webserv.hpp"

int	hexa_to_int(char a)
{
	std::string base = "0123456789abcdef";
	int	count;

	count = 0;
	while (base[count])
	{
		if (tolower(a) == base[count])
			return (count);
		count++;
	}
	return (-1);
}

std::string	str_to_lower(const std::string str)
{
	std::string	res = str;
	for (size_t i = 0; i < str.size(); i++)
		res[i] = std::tolower(res[i]);
	return (res);
}

void str_trim(std::string &str, const std::string white_space)
{
		size_t len = str.size();
		size_t start = str.find_first_not_of(white_space);
		size_t end = str.find_last_not_of(white_space);

		if (start > len || end > len)
		{
				str.clear();
				return;
		}
		std::string tmp = str.substr(start, len - start - (len - end) + 1);
		str = tmp;
}

bool	compare_ignore_case(const std::string first, const std::string second)
{
	return (str_to_lower(first) == str_to_lower(second));
}

std::string	get_next_line(std::string &str, const std::string delim)
{
	size_t	nl_index = str.find(delim);
	if (nl_index == std::string::npos)
		return ("");
	std::string	new_str = str.substr(0, nl_index);
	str.erase(0, nl_index + delim.size());
	return (new_str);
}

bool	str_end_with(std::string str, std::string end)
{
	return (str.size() >= end.size() && std::equal(end.rbegin(), end.rend(), str.rbegin()));
}

std::string	to_str(unsigned int nb)
{
	std::stringstream	ss;
	ss << nb;
	return (ss.str());
}

std::string	get_extension(std::string str)
{
	size_t	ext_pos = str.rfind('.');
	if (ext_pos == std::string::npos)
		return ("");
	return (str.substr(ext_pos, str.size() - ext_pos));
}
