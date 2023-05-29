#pragma once
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "webserv.hpp"

struct CGI_save {
	char**				env;
	Response			*response;
	std::vector<char>	output;
	ssize_t				nb_byte;

	CGI_save() : env(NULL), response(NULL), nb_byte(-1) {}
};

enum	ProcessStep {
	FROM_CLIENT,
	TO_CGI,
	FROM_CGI,
	TO_CLIENT
};

class Client {
	private :

		struct sockaddr_in	_addr;
		struct sockaddr_in	_socket_addr;

		ProcessStep			_current_step;

		int					_fd;
		int					_cgi_fd_in;
		int					_cgi_fd_out;

		server_block*		_server_conf;
		block*				_location_conf;

		Request*			_req;
		Response*			_response;

		/* ReadContext */
		std::string			_headers;
		std::vector<char>	_body;
		std::vector<char>	_unchunked_body;
		bool				_header_done;
		bool				_complete;
		int					_last_chunk;
		long				_body_left;

		/* CgiContext */
		int					_cgi_pid;
		//Whatever you need

		CGI_save			_cgi;

		int	parse_headers();
		int parse_body(size_t old_size, size_t to_add, char* buffer);
		int parse_body_by_chunk();
		int	get_chunk_size(char* data, int content_left);

		void print_request();
	public :
		Client();
		Client(const Client& copy);
		Client& operator=(const Client& copy);
		~Client();
		
		int 	accept(int socket_fd, const sockaddr_in& socket_addr);
		int		read_request();
		int		serve_request();


		bool			is_parsing_complete() const;

		int				get_fd() const;
		int				get_file_fd() const;
		void			set_file_fd(int fd);
		int				get_cgi_fd_in() const;
		void			set_cgi_fd_in(int fd);
		int				get_cgi_fd_out() const;
		void			set_cgi_fd_out(int fd);

		server_block*	get_server_conf() const;
		block*			get_location_conf() const;

		Request*		get_request() const;
		Response*		get_response();
		void			set_response(Response* response);

		const struct sockaddr_in	&get_addr() const;		
		const struct sockaddr_in	&get_socket_addr() const;

		CGI_save		&get_cgi_save();
		ProcessStep		get_current_step() const;
		void			set_current_step(ProcessStep step);

		int				get_cgi_pid() const;
		void			set_cgi_pid(int pid);
};

#endif
