#include "webserv.hpp"

Client::Client() :
	_fd(-1),
	_cgi_fd_in(-1),
	_cgi_fd_out(-1),
	_server_conf(NULL),
	_location_conf(NULL),
	_req(NULL),
	_response(NULL),
	_header_done(false),
	_complete(false),
	_last_chunk(-1),
	_body_left(-1),
	_cgi_pid(-1)
{	
}
Client::Client(const Client& copy) : 
	_addr(copy._addr),
	_socket_addr(copy._socket_addr),
	_current_step(copy._current_step),
	_fd(copy._fd),
	_cgi_fd_in(copy._cgi_fd_in),
	_cgi_fd_out(copy._cgi_fd_out),
	_server_conf(copy._server_conf),
	_location_conf(copy._location_conf),
	_req(copy._req),
	_response(copy._response),
	_headers(copy._headers),
	_body(copy._body),
	_unchunked_body(copy._unchunked_body),
	_header_done(copy._header_done),
	_complete(copy._complete),
	_last_chunk(copy._last_chunk),
	_body_left(copy._body_left),
	_cgi_pid(copy._cgi_pid)
{
}

Client&	Client::operator=(const Client& copy)
{
	if (this == &copy)
		return (*this);
	_addr = copy._addr;
	_socket_addr = copy._socket_addr;
	_fd = copy._fd;
	_cgi_fd_in = copy._cgi_fd_in;
	_cgi_fd_out = copy._cgi_fd_out;
	_req = copy._req;
	_response = copy._response;
	_headers = copy._headers;
	_body = copy._body;
	_header_done = copy._header_done;
	_complete = copy._complete;
	_last_chunk = copy._last_chunk;
	_body_left = copy._body_left;
	_server_conf = copy._server_conf;
	_location_conf = copy._location_conf;
	_cgi_pid = copy._cgi_pid;
	return (*this);
}

Client::~Client()
{
	if (_fd != -1)
		close(_fd);
	if (_cgi_fd_in != -1)
		close(_cgi_fd_in);
	if (_cgi_fd_out != -1)
		close(_cgi_fd_out);
	if (_cgi_pid != -1)
		kill(SIGKILL, _cgi_pid);
	if (_response)
		delete _response;
	if (_req)
		delete _req;
	if (_cgi.env != NULL)
		delete_env(&_cgi.env);
	if (_cgi.response != NULL)	
		delete _cgi.response;
}

int Client::accept(int socket_fd, const sockaddr_in& socket_addr)
{
	socklen_t size = sizeof(sockaddr_in);
	_fd = ::accept(socket_fd, (struct sockaddr *)&_addr, &size);
	if (_fd < 0)
	{
		Logger(ERROR) << "Error encountered while accepting a new client.";
		return (1);
	}
	_socket_addr = socket_addr;
	return (0);
}

int Client::read_request()
{
	int		ret;
	char	buffer[READ_SIZE];
	int		to_read = READ_SIZE - 1;

	if (_body_left != -1 && _body_left < to_read)
		to_read = _body_left;

	ret = read(_fd, buffer, to_read);
	buffer[ret] = 0;
	if (!ret || ret == -1)
	{
		if (!ret)
			Logger(INFO) << "Client closed the connection";
		else
			Logger(ERROR) << "Error reading socket " << inet_ntoa(_addr.sin_addr) << ":" << ntohs(_addr.sin_port);
		return (2);
	}

	size_t end_header = 0;
	size_t old_size = 0;
	
	if (!_req || !_header_done)
	{
		old_size = _headers.size();
		_headers.append(buffer);
	}
	if (!_req)
	{
		size_t end_start_line = _headers.find("\r\n");
		if (end_start_line != std::string::npos)
		{
			_req = new Request();
			if (!_req)
			{
				Logger(ERROR) << "Allocation error";
				return (1);
			}
			if (!_req->init_start_line(_headers.substr(0, end_start_line)))
			{
				_response = build_http_error(400, "Bad Request", _location_conf);
				return (1);
			}
			if (_req->get_http_version().major_version != 1 || _req->get_http_version().minor_version > 1)
			{
				_response = build_http_error(505, "HTTP Version not supported", _location_conf);
				return (1);
			}
		}
	}
	if (_req && !_header_done)
	{
		end_header = _headers.find("\r\n\r\n");
		if (end_header != std::string::npos)
		{
			_header_done = true;
			// +2 to include one \r\n
			_headers.resize(end_header + 2);
			// +4 to skip \r\n\r\n and go to body
			end_header = end_header - old_size + 4;
			if (parse_headers())
				return (1);
		}
	}
	
	if (_last_chunk != -1 || _body_left != -1)
	{
		size_t old_size = _body.size();
		size_t to_add = ret - end_header;
		if (_location_conf->max_body != -1 && (old_size + to_add) > (size_t)_location_conf->max_body)
		{
			Logger(ERROR) << "Body too large";
			_response = build_http_error(413, "Request Entity Too Large", _location_conf);
			return (1);
		}
		if (parse_body(old_size, to_add, &buffer[end_header]))
			return (1);
	}
	if (_complete)
		print_request();
	return (0);
}

int Client::parse_headers()
{
	if (!_req)
	{
		Logger(ERROR) << "Allocation error";
		_response = build_http_error(500, "Internal server error", NULL);
		return (1);
	}
	if (!_req->init_header(_headers))
	{
		_response = build_http_error(400, "Bad request", _location_conf);
		return (1);
	}

	std::string host = _req->get_host();
	_server_conf = ServerContext::getMatchingServerBlock(host, _socket_addr);
	if (!_server_conf) {
		Logger(ERROR) << "Server corresponding to fd not found";
		_response = build_http_error(500, "Internal server error", NULL);
		return (1);
	}
	_location_conf = ServerContext::getMatchingLocationBlock(_server_conf, _req->get_ressource_path());
	if (!_location_conf) {
		Logger(ERROR) << "Location not found";
		_response = build_http_error(404, "Not found", _server_conf);
		return (1);
	}

	std::string method = _req->get_method_str();
	std::vector<std::string> vecMethod = _location_conf->method;
	if (!vecMethod.empty() && std::find(vecMethod.begin(), vecMethod.end(), method) == vecMethod.end())
	{
		Logger(ERROR) << "Method not allowed";
		_response = build_http_error(405, "Method Not Allowed", _location_conf);
		return (1);
	}

	std::string value;
	if (!(value = _req->get_header_value("content-length")).empty()) {
		_body_left = atol(value.c_str());
		_body.reserve(_body_left);
		if (_body_left < 0 || (_location_conf->max_body != -1 && _body_left > _location_conf->max_body)) {
			Logger(ERROR) << "Content-length too large";
			_response = build_http_error(413, "Request Entity Too Large", _location_conf);
			return (1);
		}
		if (!_body_left)
			_complete = true;
	}
	else if (!(value = _req->get_header_value("transfer-encoding")).empty()
			&& value == "chunked")
		_last_chunk = 0;
	else
		_complete = true;
	if (!(value = _req->get_header_value("expect")).empty()
			&& value == "100-continue") {
		Logger(DEBUG) << "100 Continue ! ";
		_response = new Response();
		if (!_response) {
			Logger(ERROR) << "Allocation error";
			return (1);
		}
		_current_step = TO_CLIENT;
		_response->set_status_code(100);
		_response->set_reason_phrase("Continue");
		_response->set_header_value("Connection", "keep-alive");
	}
	return (0);
}

int Client::parse_body(size_t old_size, size_t to_add, char* buffer)
{
	if (to_add > 0)
	{
		_body.resize(old_size + to_add);
		std::memcpy(&_body[old_size], buffer, to_add);
	}
	if (_last_chunk != -1)
	{
		if (parse_body_by_chunk())
			return (1);
	}
	else if (_body_left != -1)
	{
		_body_left -= to_add;
		if (_body_left <= 0)
			_complete = true;
	}
	if (_complete)
		_req->set_body(_body);
	return (0);
}

int Client::parse_body_by_chunk()
{
	int		content_left;
	char*	data;
	int		chunk_size;
	size_t	old_size;

	data= &_body[_last_chunk];
	content_left = _body.size() - _last_chunk;
	chunk_size = get_chunk_size(data, content_left);
	while (chunk_size > 0 && content_left >= chunk_size)
	{
		old_size = _unchunked_body.size();
		_unchunked_body.resize(old_size + chunk_size);
		std::memcpy(&_unchunked_body[old_size], &data[_last_chunk], chunk_size);
		_last_chunk += chunk_size + 2;
		content_left = _body.size() - _last_chunk;
		chunk_size = get_chunk_size(&data[_last_chunk], content_left);
	}
	if (chunk_size == -2)
		return (1);
	if (chunk_size == 0)
	{
		_complete = true;
		_body = _unchunked_body;
		_unchunked_body.clear();
	}
	return (0);
}

int	Client::get_chunk_size(char* data, int content_left)
{
	int i = 0;
	int	to_add;
	int	total = 0;

	while (i < content_left && data[i] != '\r')
	{
		to_add = hexa_to_int(data[i]);
		if (to_add == -1)
		{
			Logger(ERROR) << "Non hexa or /r/n value  in the chunks' header";
			_response = build_http_error(400, "Bad Request", _location_conf);
			return (-2);
		}
		total = (total * 16) + to_add;
		++i;
	}
	if (i + 1 >= content_left)
	{
		Logger(DEBUG) << "Chunk was not fully read, waiting for next call";
		return (-1);
	}
	if (data[i] != '\r' || data[i + 1] != '\n')
	{
		Logger(ERROR) << "Non hexa or /r/n value  in the chunks' header";
		_response = build_http_error(400, "Bad Request", _location_conf);
		return (-2);
	}
	_last_chunk += i + 2;
	return (total);
}

int Client::serve_request()
{
	Logger(INFO) << "Serving request";
	std::string path = _req->get_ressource_path();
	if ((_response = http_redir(path, _location_conf)))
	{
		_current_step = TO_CLIENT;
		return (0);
	}
	std::string file_path = ServerContext::get_server_path(path, _location_conf);

	if (file_path.size() == 0 || file_path[0] != '/')
		file_path.insert(file_path.begin(), '/');
	if (access(file_path.c_str(), F_OK) == -1)
	{
		_response = build_http_error(404, "Not found", _location_conf);
		return (1);
	}
	if (path_is_dir(file_path.c_str()) && _req->get_method() != DELETE) // change
	{
		if (_location_conf->index.size() > 0)
		{
			std::string old_file_path = file_path;
			file_path = return_index_page(file_path, _location_conf);
			if (file_path == old_file_path)
			{
				if ((_response = directory_listing(file_path, path, _location_conf)))
				{
					_current_step = TO_CLIENT;
					return (0);
				}
			}
		}
		else if ((_response = directory_listing(file_path, path, _location_conf)))
		{
			_current_step = TO_CLIENT;
			return (0);
		}
		else
		{
			_response = build_http_error(404, "Not Found", _location_conf);
			return (1);
		}
	}
	if (_req->get_method() == DELETE)
	{
		if (access(file_path.c_str(), F_OK) == -1)
		{
			_response = build_http_error(404, "Not Found", _location_conf);
			return (1);
		}
		if (access(file_path.c_str(), W_OK) == -1)
		{
			_response = build_http_error(403, "Forbidden", _location_conf);
			return (1);
		}
		if (std::remove(file_path.c_str()))
			return (1);
		_response = new Response();
		if (!_response)
			return (1);
		_response->set_status_code(204);
		_response->set_reason_phrase("No Content");
		_response->set_header_value("connection", "close");
		_current_step = TO_CLIENT;
		return (0);
	}

	std::map<std::string, std::string> &cgi_map = _location_conf->cgi;
	std::string	extension = get_extension(file_path);
	std::map<std::string, std::string>::const_iterator	cgi_path;
	if ((cgi_path = cgi_map.find(extension)) != cgi_map.end())
	{
		if (!setup_cgi(extension, cgi_path->second, file_path, path, this))
			return (0);
		return (1);
	}

	std::vector<char> content;
	if (build_body_from_file(&content, file_path))
	{
		_response = build_http_error(403, "Forbidden", _location_conf);
		Logger(INFO) << "Serving";
		return (1);
	}

	_response = new Response();
	if (!_response)
		return (1);
	_current_step = TO_CLIENT;
	if (file_path.find('.') < file_path.size())
	{
		std::string content_type = set_content_type(file_path.substr(file_path.find('.')));
		_response->set_header_value("content-type", content_type);
	}
	else
		_response->set_header_value("content-type", "text/html");
	_response->set_status_code(200);
	_response->set_reason_phrase("Ok");
	_response->set_body(content);
	_response->set_header_value("connection", "close");
	return (0);
}

void Client::print_request()
{
	std::vector<char> copy = _body;
	copy.push_back(0);
	Logger(INFO) \
		<< "\n----------------------------------- BEGIN REQUEST ------------------------------\n" \
		<< _headers << "\r\n" << &copy[0] \
		<< "------------------------------------ END REQUEST -------------------------------"; \
}

bool	Client::is_parsing_complete() const
{
	return (_complete);
}

int		Client::get_fd() const
{
	return (_fd);
}

int		Client::get_cgi_fd_in() const
{
	return (_cgi_fd_in);
}

void	Client::set_cgi_fd_in(int fd)
{
	_cgi_fd_in = fd;
}

int		Client::get_cgi_fd_out() const
{
	return (_cgi_fd_out);
}

void	Client::set_cgi_fd_out(int fd)
{
	_cgi_fd_out = fd;
}

server_block*	Client::get_server_conf() const
{
	return _server_conf;
}

block* Client::get_location_conf() const
{
	return _location_conf;
}

Request*	Client::get_request() const
{
	return _req;
}

Response*	Client::get_response()
{
	return _response;
}

void		Client::set_response(Response* response)
{
	_response = response;
}

const struct sockaddr_in	&Client::get_addr() const
{
	return (_addr);
}		

const struct sockaddr_in	&Client::get_socket_addr() const
{
	return (_socket_addr);
}

CGI_save		&Client::get_cgi_save()
{
	return (_cgi);
}

ProcessStep	Client::get_current_step() const
{
	return _current_step;
}

void		Client::set_current_step(ProcessStep step)
{
	_current_step = step;
}

int	Client::get_cgi_pid() const
{
	return (_cgi_pid);
}

void Client::set_cgi_pid(int pid)
{
	_cgi_pid = pid;
}
