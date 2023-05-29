#include "webserv.hpp"

bool str_is_positive_digit(std::string str)
{
    size_t i = 0;
   
    while (str[i])
    {
        if (isdigit(str[i]))
            i++;
        else
		{
			Logger(ERROR) << "No only digit found-> " << str << " char " << str[i];
			return (false);
		}
    }
    return (true);
}
bool is_comment(std::string &str)
{
	if (!str.empty() && str[0] == '#')
		return (true);
	return (false);
}

void ParsingFile::real_split(std::string &src, std::vector<std::string> &vec)
{
	size_t  pos = 0;
	size_t  start = 0;
	while ((start = src.find_first_not_of(DELIM, pos)) != std::string::npos)
	{        
		pos = src.find_first_of(DELIM, start + 1);
		vec.push_back(src.substr(start, pos - start));
	}
}

bool my_split(std::string &src, std::string &key, std::string &data, std::string &delims)
{
	size_t  pos = 0;
	size_t  start = 0;
	start = src.find_first_not_of(delims, pos);
	if (start < src.length())
	{
		pos = src.find_first_of(delims, start + 1);
		key = src.substr(start, pos - start);
		if (pos < src.length())
			data = src.substr(pos + 1);
		else
		{
			Logger(DEBUG) << "My split error";
			return (false);
		}
	}
	else
	{
		Logger(DEBUG) << "My split error";
		return (false);
	}
	return (true);
}

bool open_file(std::string path, std::ifstream &file)
{
	file.open(path.c_str(), std::fstream::in);
	if (!file.is_open())
	{
		Logger(ERROR) << "Open file failed\n";
		return (false);
	}
	return (true);
}

bool ParsingFile::check_loc_key(std::string &key)
{
	for (unsigned int i = 0; i <= MAX_LOC_KEY; i++)
	{
		if (key == _keyword_location[i])
			return (true);
	}
	return (false);
}

bool ParsingFile::check_server_key(std::string &key)
{
	for (unsigned int i = 0; i <= MAX_SERV_KEY; i++)
	{
		if (key == _keyword_server[i])
			return (true);
	}
	return (false);
}
