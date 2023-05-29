#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"

class Server {
	private :
		std::map<int, Socket>			_socketMap;
		std::list<Client*>				_clientList;

		std::list<Client*>				_fromClient;
		std::list<Client*>				_toCgi;
		std::list<Client*>				_fromCgi;
		std::list<Client*>				_toClient;

		Response						_internalError;

		int		get_socketfd_for_addr(const sockaddr_in& addr);
		int		add_fd(fd_set* set, int fd, int maxfd);
		int		update_fd_sets(fd_set* reaset, fd_set*writeset);
	public :
		Server();

		typedef std::list<Client*>::iterator	ClientIterator;

		int			setup_sockets();
		int			run();
		void		handle_requests_to_accept(fd_set& read_set);
		void		handle_requests_to_parse(fd_set& read_set);
		void		handle_requests_to_answer(fd_set& to_write);
		void		handle_write_to_cgi(fd_set& to_write);
		void		handle_read_from_cgi(fd_set& read_set);
		void		close_server();
};

#endif
