#include "webserv.hpp"

	// if (req.is_bad_syntax())
	// {
	// 	_status_code = 400;
	// 	_reason_phrase = "Bad Syntax";
	// }
	// else if (req.get_http_version() > HTTP1_1)
	// {
	// 	_status_code = 505;
	// 	_reason_phrase = "HTTP Version not supported";
	// }
	// else
	// {
	// 	_status_code = 200;
	// 	_reason_phrase = "OK";
	// }

Response::Response():
	_status_code(0), _php_cgi(false)
{
	_http_version.major_version = 1;
	_http_version.minor_version = 1;
	_http_version.protocol = "HTTP";
}

Response::~Response() {}

Response::Response(const Response &copy)
{
	*this = copy;
}

Response	&Response::operator=(const Response &rhs)
{
	if (this == &rhs)
		return (*this);
	_status_code = rhs.get_status_code();
	_reason_phrase = rhs.get_reason_phrase();
	_http_version = rhs.get_http_version();
	_header = rhs.get_header();
	_body = rhs.get_body();
	_php_cgi = rhs.get_php_cgi();
	return (*this);
}

void	Response::set_body(std::vector<char> &body)
{
	_body = body;
}

void	Response::set_body(std::string &body)
{
	_body = std::vector<char>(body.begin(), body.end());
}

void	Response::set_status_code(unsigned short code)
{
	_status_code = code;
}

void	Response::set_reason_phrase(const std::string str)
{
	_reason_phrase = str;
}

void	Response::set_http_version(const HTTPVersion http_version)
{
	_http_version = http_version;
}

char	*Response::to_str(size_t *size) const
{
	std::ostringstream	ss;
	ss << Request::http_version_to_str(_http_version);
	ss << ' ' << _status_code << ' ' << _reason_phrase << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _header.begin(); it != _header.end(); it++)
		ss << it->first << ": " << it->second << "\r\n";
	if (!_php_cgi)
		ss << "\r\n";
	char	*response = new char[ss.str().size() + _body.size() + 1];
	size_t	i;
	for (i = 0; i < ss.str().size(); i++)
		response[i] = ss.str()[i];
	for (size_t j = 0; j < _body.size(); j++, i++)
		response[i] = _body[j];
	response[i] = 0;
	*size = i - 1;
	return (response);
}

const unsigned short	&Response::get_status_code() const
{
	return (_status_code);
}

const std::string	&Response::get_reason_phrase() const
{
	return (_reason_phrase);
}

const HTTPVersion	&Response::get_http_version() const
{
	return (_http_version);
}

const std::map<std::string, std::string>	&Response::get_header() const
{
	return (_header);
}

const std::vector<char>	&Response::get_body() const
{
	return (_body);
}

const bool	&Response::get_php_cgi() const
{
	return (_php_cgi);
}


void	Response::set_header_value(std::string key, std::string value)
{
	_header[key] = value;
}

void	Response::set_php_cgi(bool value)
{
	_php_cgi = value;
}
