#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webserv.hpp"

class Response
{
private:
	unsigned short	_status_code;
	std::string		_reason_phrase;


	HTTPVersion			_http_version;
	std::map<std::string, std::string>	_header;

	std::vector<char>	_body;
	bool			_php_cgi;

public:
	Response();
	Response(const Response &);
	Response	&operator=(const Response &);
	~Response();

	void	set_http_version(const HTTPVersion http_version);
	void	set_reason_phrase(const std::string str);
	void	set_status_code(unsigned short code);
	void	set_body(std::vector<char> &body);
	void	set_body(std::string &body);
	const unsigned short						&get_status_code() const;
	const std::string							&get_reason_phrase() const;
	const HTTPVersion							&get_http_version() const;
	const std::map<std::string, std::string>	&get_header() const;
	const std::vector<char>						&get_body() const;
	const bool									&get_php_cgi() const;

	void	set_header_value(std::string key, std::string value);
	void	set_php_cgi(bool value);

	char	*to_str(size_t *size) const;
};

#endif
