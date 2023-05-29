#include "webserv.hpp"

void	sigintHandler(int signum)
{
	// We have to not do anything, to cancel SIGINT basic handling.
	// Epoll_wait will still catch the signal and return a -1, which allows
	// us to properly close the server at this point.
	(void)signum;
}

int main(int argc, char *argv[], char *env[])
{
	if (argc > 2)
	{
		Logger(ERROR) << "Too many arguments";
		return (1);
	}
	
	Server			server;
	ParsingFile		conf;
	std::string 	file;
	
	if (argc == 1)
		file = "./default.conf";
	else
		file = argv[1];
	if (!conf.parsing(file.c_str()))
	{
		Logger(ERROR) << "Error parsing configuration file";
		return (1);
	}
	if (server.setup_sockets())
		return (1);

	signal(SIGINT, sigintHandler);
	ServerContext::set_env(env);
	Logger::setCurrentLevel(INFO);
	server.run();
	return (0);
}
