#include "webserv.hpp"
#include <map>
#include <utility>
#include <sys/wait.h>

Server::Server()
{
	build_http_error(500, "Server Internal Error", NULL, &_internalError);
}

int	Server::setup_sockets()
{
	std::vector<server_block>&	serverBlockList = ServerContext::getServerBlocks();
	server_block*				serverBlock;

	for (int i = 0; i < (int)serverBlockList.size(); ++i)
	{
		serverBlock = &serverBlockList[i];
		for (int j = 0; j < (int)serverBlock->listen.size(); ++j)
		{
			if (get_socketfd_for_addr(serverBlock->listen[j]) == -1)
			{
				Socket	sock;
				if (sock.setup(serverBlock->listen[j]))
					return (1);
				int fd = sock.get_socket_fd();
				_socketMap.insert(std::make_pair(fd, sock));
			}
		}
	}
	return (0);
}

int	Server::get_socketfd_for_addr(const sockaddr_in& addr)
{
	typedef std::map<int, Socket>::iterator iterator;
	struct sockaddr_in	current_addr;

	for (iterator it = _socketMap.begin(); it != _socketMap.end(); ++it)
	{
		current_addr = it->second.get_addr();
		if (current_addr.sin_addr.s_addr == addr.sin_addr.s_addr
			&& current_addr.sin_port == addr.sin_port
			&& current_addr.sin_family == addr.sin_family)
		{
			return (it->first);
		}
	}
	return (-1);
}

int Server::add_fd(fd_set* set, int fd, int maxfd)
{
	FD_SET(fd, set);
	if (fd > maxfd)
		return (fd);
	return (maxfd);
}

int	Server::update_fd_sets(fd_set* readset, fd_set* writeset)
{
	int maxfd = 0;

	FD_ZERO(writeset);
	FD_ZERO(readset);

	for(std::map<int, Socket>::iterator it = _socketMap.begin(); it != _socketMap.end(); ++it)
		maxfd = add_fd(readset, it->first, maxfd);
	for (ClientIterator it = _fromClient.begin(); it != _fromClient.end(); ++it)
		maxfd = add_fd(readset, (*it)->get_fd(), maxfd);
	for (ClientIterator it = _fromCgi.begin(); it != _fromCgi.end(); ++it)
		maxfd = add_fd(readset, (*it)->get_cgi_fd_out(), maxfd);

	for (ClientIterator it = _toCgi.begin(); it != _toCgi.end(); ++it)
		maxfd = add_fd(writeset, (*it)->get_cgi_fd_in(), maxfd);
	for (ClientIterator it = _toClient.begin(); it != _toClient.end(); ++it)
		maxfd = add_fd(writeset, (*it)->get_fd(), maxfd);

	return (maxfd);
}

int	Server::run()
{
	fd_set			read_set;
	fd_set			write_set;
	struct timeval	timeout;
	int				ret = 0;
	int				maxfd;


	while (1)
	{
		ret = 0;
		while (ret == 0)
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			maxfd = update_fd_sets(&read_set, &write_set);

			ret = select(maxfd + 1, &read_set, &write_set, NULL, &timeout);
		}
		if (ret < 0)
		{
			Logger(DEBUG) << "Select interrupted";
			close_server();
			return (1);
		}
		if (ret > 0)
		{
			handle_requests_to_accept(read_set);
			handle_requests_to_parse(read_set);
			handle_write_to_cgi(write_set);
			handle_read_from_cgi(read_set);
			handle_requests_to_answer(write_set);
		}
	}
}

void	Server::handle_requests_to_accept(fd_set& read_set)
{
	std::map<int, Socket>::iterator copy;

	for (std::map<int, Socket>::iterator it = _socketMap.begin(); it != _socketMap.end(); ++it)
	{
		if (FD_ISSET(it->first, &read_set))
		{
			Logger(DEBUG) << "Accepting client";
			Client*	client = new Client();
			if (!client || client->accept(it->first, it->second.get_addr()))
			{
				close (it->first);
				if (client)
					delete client;
				copy = it;
				++copy;
				if (copy != _socketMap.end())
				{
					int next_fd = copy->first;
					_socketMap.erase(it);
					it = _socketMap.find(next_fd);
				}
			}
			else
			{
				client->set_current_step(FROM_CLIENT);
				Logger(DEBUG) << "Adding client to _fromClient";
				_fromClient.push_back(client);
				_clientList.push_back(client);
			}
		}
	}
}

void	Server::handle_requests_to_parse(fd_set& read_set)
{
	Client*	client;
	int		fd;
	int		ret;

	for (ClientIterator it = _fromClient.begin(); it != _fromClient.end(); ++it)
	{
		client = *it;
		fd = client->get_fd();
		if (FD_ISSET(fd, &read_set))
		{
			ret = client->read_request();
			if (ret == 2)
			{
				Logger(DEBUG) << "Error on socket, closing client and removing it from vectors";
				_clientList.remove(client);
				delete client;
				it = _fromClient.erase(it);
				if (it == _fromClient.end())
					break;
				continue;
			}
			if (ret == 1 || client->get_response())
				client->set_current_step(TO_CLIENT);
			if (client->is_parsing_complete() && client->serve_request())
				client->set_current_step(TO_CLIENT);

			ProcessStep next_step = client->get_current_step();
			if (next_step != FROM_CLIENT)
			{
				if (next_step == TO_CGI)
					_toCgi.push_front(client);
				else if (next_step == TO_CLIENT)
					_toClient.push_front(client);
				Logger(DEBUG) << "Removing client from _fromClient";
				it = _fromClient.erase(it);
				if (it == _fromClient.end())
					break;
			}
		}
	}
}

void	Server::handle_write_to_cgi(fd_set &write_set)
{
	Client* client;

	for (ClientIterator it = _toCgi.begin(); it != _toCgi.end(); ++it)
	{
		client = *it;
		if (FD_ISSET(client->get_cgi_fd_in(), &write_set))
		{
			/* Logger(DEBUG) << "CGI write"; */
			if (write_to_cgi(client))
			{
				if (client->get_cgi_pid() != -1)
				{
					kill(SIGKILL, client->get_cgi_pid());
					client->set_cgi_pid(1);
				}
				client->set_current_step(TO_CLIENT);
				_toClient.push_front(client);
			}
			else
			{
				client->set_current_step(FROM_CGI);
				_fromCgi.push_front(client);
			}
			it = _toCgi.erase(it);
			if (it == _toCgi.end())
				break;
		}
	}
}

void	Server::handle_read_from_cgi(fd_set& read_set)
{
	Client* client;

	for (ClientIterator it = _fromCgi.begin(); it != _fromCgi.end(); ++it)
	{
		client = *it;

		if (!FD_ISSET(client->get_cgi_fd_out(), &read_set))
			continue;

		if (client->get_current_step() == FROM_CGI)
		{
			Logger(DEBUG) << "CGI wait";
			if (read_cgi_output(client) || client->get_response())
				client->set_current_step(TO_CLIENT);
		}
		if (client->get_current_step() == TO_CLIENT)
		{
			_toClient.push_front(client);
			it = _fromCgi.erase(it);
			if (it == _fromCgi.end())
				break;
		}
	}

}

void	Server::handle_requests_to_answer(fd_set& write_set)
{
	Client* 	client;
	Response*	response;

	for (ClientIterator it = _toClient.begin(); it != _toClient.end(); ++it)
	{
		client = *it;
		if (FD_ISSET(client->get_fd(), &write_set))
		{
			Logger(DEBUG) << "Answering client";

			response = client->get_response();
			if (!response)
				response = &_internalError;

			size_t	response_size = 0;
			char	*response_plain = response->to_str(&response_size);
			Logger(INFO) \
				<< "\n----------------------------------- BEGIN RESPONSE ---------------------------------\n" \
				<<  response_plain \
				<< "\n------------------------------------ END RESPONSE ----------------------------------";
			if (write(client->get_fd(), response_plain, response_size) == -1)
			{
				Logger(ERROR) << "Error writing response";
			}

			delete [] response_plain;

			if (response->get_status_code() != 100)
			{
				_clientList.remove(client);
				delete client;
			}
			else
			{
				if (response != &_internalError)
					delete response;
				client->set_response(NULL);
				_fromClient.push_front(client);
			}
			it = _toClient.erase(it);
			if (it == _toClient.end())
				break;
		}
	}
}

void	Server::close_server()
{
	for (ClientIterator it = _clientList.begin(); it != _clientList.end(); ++it)
		delete *it;
	for (std::map<int, Socket>::iterator it = _socketMap.begin(); it != _socketMap.end(); ++it)
		close(it->first);
	Logger(INFO) << "Server closed succesfully";
}
