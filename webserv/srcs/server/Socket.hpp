#pragma once
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "webserv.hpp"

class Socket {
	private :
		int						_socketfd;
		sockaddr_in				_addr;
	public :
		Socket();
		Socket(const Socket& copy);
		~Socket();

		Socket& operator=(const Socket& copy);

		int	setup(const sockaddr_in& sin);
		int get_socket_fd() const;
		const sockaddr_in& get_addr() const;
};

#endif
