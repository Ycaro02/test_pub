#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"

#define HTTP_VERSION_NB 3
#define METHOD_NB 8

struct	HTTPVersion {
	std::string		protocol;
	unsigned int	major_version;
	unsigned int	minor_version;
};

enum	Method {
	OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT, NOMETHOD
};

struct ContentType
{
	std::string	type;
	std::string	boundary;
};

class Request
{
private:
	
	static std::string	_method_str[];
	static std::string	_http_version_str[];

	Method				_method;
	std::string			_ressource_path;
	std::string			_query_string;
	HTTPVersion			_http_version;

	std::string							_host;
	ContentType							_content_type;
	std::vector<std::string>			_transfert_encoding;
	std::map<std::string, std::string>	_header;

	std::vector<char>	_body;

	bool	init_abs_path(std::string &abs_path);
	bool	init_http_version(std::string &http_version);
	bool	init_header_field(std::string &line);
	void	init_host(std::string &value);
	bool	init_transfert_encoding(std::string &value);
	bool	init_content_type(std::string &value);

public:
	Request();
	Request(const Request &);
	Request	&operator=(const Request &);
	~Request();

	bool	init_start_line(std::string start_line);
	bool	init_header(std::string header);
	
	static std::string	method_to_str(Method method);
	static std::string	http_version_to_str(HTTPVersion http_version);

	bool	init_method(std::string &method);

	const Method								&get_method() const;
	const std::string							get_method_str() const;
	const std::string							&get_ressource_path() const;
	const std::string							&get_query_string() const;
	const HTTPVersion							&get_http_version() const;
	const std::string							&get_host() const;
	const ContentType							&get_content_type() const;
	const std::vector<std::string>				&get_transfert_encoding() const;
	const std::map<std::string, std::string>	&get_header() const;
	const std::vector<char>						&get_body() const;
	std::string									get_header_value(std::string key);

	void	set_body(std::vector<char> &body);
};

#endif
