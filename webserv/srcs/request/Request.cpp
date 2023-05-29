#include "webserv.hpp"

std::string	Request::_method_str[] = {
	"OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"
};

std::string	Request::http_version_to_str(HTTPVersion http_version)
{
	std::stringstream	ss;
	ss << http_version.protocol;
	ss << '/';
	ss << http_version.major_version;
	ss << '.';
	ss << http_version.minor_version;
	return (ss.str());
}

std::string	Request::method_to_str(Method method)
{
	if (method < 0 || method >= METHOD_NB)
		return ("");
	return (_method_str[method]);
}

Request::Request():
	_method(NOMETHOD)
{
	_http_version.major_version = 0;
	_http_version.minor_version = 0;
}

Request::Request(const Request &copy)
{
	*this = copy;
}

Request	&Request::operator=(const Request &rhs)
{
	if (this == &rhs)
		return (*this);
	_method = rhs.get_method();
	_ressource_path = rhs.get_ressource_path();
	_query_string = rhs.get_query_string();
	_http_version = rhs.get_http_version();
	_host = rhs.get_host();
	_content_type = rhs.get_content_type();
	_transfert_encoding = rhs.get_transfert_encoding();
	_header = rhs.get_header();
	_body = rhs.get_body();
	return (*this);
}

Request::~Request() {}

bool	Request::init_method(std::string &method)
{
	for (unsigned int i = 0; i < METHOD_NB; i++)
	{
		if (method == _method_str[i])
		{
			_method = static_cast<Method>(i);
			return (true);
		}
	}
	return (false);
}

bool	Request::init_abs_path(std::string &abs_path)
{
	abs_path = url_decoding(abs_path);
	_ressource_path = get_next_line(abs_path, "?");
	if (_ressource_path.empty())
		_ressource_path = abs_path;
	else
		_query_string = abs_path;
	if (_ressource_path.empty() || _ressource_path[0] != '/')
		return (false);
	return (true);
}

bool	Request::init_http_version(std::string &http_version)
{
	_http_version.protocol = get_next_line(http_version, "/");
	if (_http_version.protocol != "HTTP")
		return (false);
	std::string	major_version_str = get_next_line(http_version, ".");
	if (major_version_str.empty() || !str_is_positive_digit(major_version_str))
		return (false);
	std::string	minor_version_str = http_version;
	if (minor_version_str.empty() || !str_is_positive_digit(minor_version_str))
		return (false);
	std::stringstream	ss;
	ss << major_version_str;
	ss >> _http_version.major_version;
	ss << minor_version_str;
	ss >> _http_version.minor_version; 
	return (true);
}

bool	Request::init_start_line(std::string start_line)
{
	std::string	crt_string = get_next_line(start_line, " ");
	if (crt_string.empty() || !init_method(crt_string))
		return (false);
	crt_string = get_next_line(start_line, " ");
	if (crt_string.empty() || !init_abs_path(crt_string))
		return (false);
	if (start_line.empty() || !init_http_version(start_line))
		return (false);
	return (true);
}

void	Request::init_host(std::string &value)
{
	_host = value;
}

bool	Request::init_transfert_encoding(std::string &lst_str)
{
	std::string	allowed_values[] = {"chunked", "compress", "deflate", "gzip"};
	std::string	value;
	while (!lst_str.empty())
	{
		value = get_next_line(lst_str, ",");
		if (value.empty())
		{
			value = lst_str;
			lst_str.clear();
		}
		str_trim(value, "\t ");
		if (!value.empty())
		{
			size_t i;
			for (i = 0; i < 4; i++)
			{
				if (value == allowed_values[i])
				{
					_transfert_encoding.push_back(value);
					break ;
				}
			}
			if (i == 4)
				return (false);
		}
	}
	return (true);
}

bool	Request::init_content_type(std::string &value)
{
	_content_type.type = value;
	std::string	type = get_next_line(value, ";");
	if (type.empty())
		value.clear();
	else
		str_trim(value);
	str_trim(_content_type.type);
	if (value.empty())
		return (true);
	while (!value.empty())
	{
		std::string param = get_next_line(value, ";");
		if (param.empty())
		{
			param = value;
			value.clear();
		}
		else
			str_trim(value);
		str_trim(param);
		std::string	param_name = get_next_line(param, "=");
		if (param_name.empty() || param.empty())
			return (false);
		if (param_name == "boundary")
			_content_type.boundary = param;
	}
	return (true);
}

bool	Request::init_header_field(std::string &line)
{
	std::string	key = get_next_line(line, ":");
	if (key.empty())
		return (false);
	str_trim(line, "\t ");
	if (compare_ignore_case(key, "host"))
		init_host(line);
	else if (compare_ignore_case(key, "transfert-encoding"))
	{
		if (!init_transfert_encoding(line))
			return (false);
	}
	else if (compare_ignore_case(key, "content-type"))
	{
		if (!init_content_type(line))
			return (false);
	}
	else
		_header[str_to_lower(key)] = line;
	return (true);
}

bool	Request::init_header(std::string header)
{
	std::string	crt_line;
	get_next_line(header);
	if (header.empty())
		return (false);
	while (!(crt_line = get_next_line(header)).empty())
		if (!init_header_field(crt_line))
			return (false);
	return (!_host.empty() && header.empty());
}

void	Request::set_body(std::vector<char> &body)
{
	_body = body;
}

const Method	&Request::get_method() const
{
	return (_method);
}

const std::string	Request::get_method_str() const
{
	return method_to_str(_method);
}

const std::string	&Request::get_ressource_path() const
{
	return (_ressource_path);
}

const std::string	&Request::get_query_string() const
{
	return (_query_string);
}

const HTTPVersion	&Request::get_http_version() const
{
	return (_http_version);
}

const std::string	&Request::get_host() const
{
	return (_host);
}

const std::map<std::string, std::string>	&Request::get_header() const
{
	return (_header);
}

const std::vector<char>	&Request::get_body() const
{
	return (_body);
}


const std::vector<std::string>	&Request::get_transfert_encoding() const
{
	return (_transfert_encoding);
}

std::string	Request::get_header_value(std::string key)
{
	std::map<std::string, std::string>::const_iterator	it;
	if ((it = _header.find(key)) == _header.end())
		return ("");
	return (it->second);
}

const ContentType	&Request::get_content_type() const
{
	return (_content_type);
}
