#include "webserv.hpp"

#define MAX_EVENT 5000

Socket::Socket()
{
}

Socket::Socket(const Socket& copy) :
	_socketfd(copy._socketfd), _addr(copy._addr)
{
}

Socket::~Socket()
{
}

Socket& Socket::operator=(const Socket& copy)
{
	if (this == &copy)
		return (*this);
	_addr = copy._addr;
	_socketfd = copy._socketfd;
	return (*this);
}

int Socket::setup(const sockaddr_in& sin)
{
	int					opt = 1;

	_addr = sin;
	Logger(DEBUG) << "Socket::add_socket begin for " << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port);

	_socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (_socketfd < 0)
	{
		Logger(ERROR) << "Error encountered while creating a socket";
		return (1);
	}

	if (setsockopt(_socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
	{
		Logger(ERROR) << "Socket option SO_REUSEADDR failed to apply";
		close(_socketfd);
		return (1);
	}

	if (bind(_socketfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		Logger(ERROR) << "Error binding socket for " << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port);
		close(_socketfd);
		return (1);
	}

	if (listen(_socketfd, SOMAXCONN) < 0)
	{
		Logger(ERROR) << "Error listening";
		close(_socketfd);
		return (1);
	}
	return (0);
}

int	Socket::get_socket_fd() const
{
	return _socketfd;
}

const sockaddr_in&	Socket::get_addr() const
{
	return (_addr);
}
