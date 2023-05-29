#include "webserv.hpp"

std::vector<server_block> ServerContext::serverBlocks;

ServerContext::ServerContext()
{
}

ServerContext::~ServerContext()
{
}

std::vector<server_block> &ServerContext::getServerBlocks(void)
{
	return ServerContext::serverBlocks;
}

/* std::vector<std::string>	split_with_str(const std::string& str, const std::string& delim) */
/* { */

/* } */

std::vector<std::string>	ServerContext::split_with_charset(const std::string& str, const std::string& charset)
{
	std::vector<std::string>	result;

	size_t start = 0;
	// Exception for path split. A bit ugly, but w/e
	if (charset == "/" && !str.empty() && str[0] == '/')
		start = 1;
	size_t end = str.find_first_of(charset, start);
	while (end != std::string::npos)
	{
		if (end != start)
		result.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(charset, start);
	}
	// Exception for path split. A bit ugly, but w/e
	if (charset != "/" || start != str.size())
		result.push_back(str.substr(start));
	return (result);
}

std::string	ServerContext::get_server_path(std::string request_path, block* loc)
{
	std::string	loc_path = loc->location_args;

	if (request_path.size() > 0 && request_path[0] == '/')
		request_path.erase(request_path.begin());
	if (request_path.size() > 0 && request_path[request_path.size() - 1] == '/')
		request_path.erase(--request_path.end());

	if (loc_path.size() > 0 && loc_path[0] == '/')
		loc_path.erase(loc_path.begin());
	if (loc_path.size() > 0 && loc_path[loc_path.size() - 1] == '/')
		loc_path.erase(--loc_path.end());

	size_t i = 0;
	while (i < loc_path.size() && loc_path[i] == request_path[i])
		++i;
	request_path = request_path.substr(i);
	std::string root_path = loc->root;
	if (request_path.size() > 0 && request_path[0] == '/')
		request_path.erase(request_path.begin());
	if (root_path.size() > 0 && root_path[root_path.size() - 1] == '/')
		root_path.erase(--root_path.end());
	return (root_path + "/" + request_path);
}


unsigned short	ServerContext::getDomainPrecision(std::vector<std::string>& hostname_parts, const std::string& servername)
{
	typedef std::vector<std::string>::reverse_iterator	iterator;
	
	std::vector<std::string>	servername_parts = split_with_charset(servername, ".");
	size_t	matching_precision = 0;

	iterator host_ite = hostname_parts.rbegin();
	iterator server_ite = servername_parts.rbegin();
	while (	host_ite != hostname_parts.rend() &&
			server_ite != servername_parts.rend() &&
			*host_ite == *server_ite)
	{
		++matching_precision;
		++host_ite;
		++server_ite;
	}
	if (host_ite == hostname_parts.rend())
	{
		if (server_ite == servername_parts.rend()
			|| (*server_ite == "*" && (server_ite + 1) == servername_parts.rend()))
			return (matching_precision);
	}
	else if (server_ite == servername_parts.rend())
	{
		if (host_ite == hostname_parts.rend())
			return (matching_precision);
	}
	else if (*server_ite == "*" && (server_ite + 1) == servername_parts.rend())
	{
		return (matching_precision);
	}
	return (0);
}

server_block	*ServerContext::getMatchingServerBlock(const std::string& hostname, const sockaddr_in& host)
{
	size_t i;
	size_t j;
	server_block	current;
	sockaddr_in		currentListen;
	std::vector<int>	indexList;

	for(i = 0; i < serverBlocks.size(); ++i)
	{
		current = serverBlocks[i];
		for (j = 0; j < current.listen.size(); ++j)
		{
			currentListen = current.listen[j];
			if (currentListen.sin_addr.s_addr == host.sin_addr.s_addr
				&& currentListen.sin_port == host.sin_port
				&& currentListen.sin_family == host.sin_family)
			{
				indexList.push_back(i);
			}
		}
	}
	if (!indexList.size())
	{
		Logger(WARN) << "No configuration match found for " << inet_ntoa(host.sin_addr) << ":" << ntohs(host.sin_port);
		return (NULL);
	}
	if (indexList.size() == 1)
	{
		return (&serverBlocks[indexList[0]]);
	}

	std::vector<std::string>	host_parts = split_with_charset(hostname, ".");

	server_block*	bestMatch = NULL;
	unsigned short	bestMatchPrecision = 0;
	unsigned short	tmpPrecision;
	for (i = 0; i < indexList.size(); ++i)
	{
		for(j = 0; j < serverBlocks[indexList[i]].server_name.size(); j++)
		{	
			if (hostname == serverBlocks[indexList[i]].server_name[j])
				return (&serverBlocks[indexList[i]]);
			tmpPrecision = getDomainPrecision(host_parts, serverBlocks[indexList[i]].server_name[j]);
			if (tmpPrecision > bestMatchPrecision)
			{
				bestMatch = &serverBlocks[indexList[i]];
				bestMatchPrecision = tmpPrecision;
			}
		}
	}
	if (!bestMatch)
		bestMatch = &serverBlocks[indexList[0]];
	return (bestMatch);
}

block*	ServerContext::getMatchingLocationBlock(block* parent, const std::string& path)
{
	typedef	std::vector<location_block>::iterator	Iterator;

	std::vector<std::string> path_parts = ServerContext::split_with_charset(path, "/");
	Iterator it;

	block*	bestMatch = NULL;
	short bestMatchPrecision = -1;
	short tmpPrecision;

	for(it = parent->loc_block.begin(); it != parent->loc_block.end(); ++it)
	{
		tmpPrecision = getPathPrecision(path_parts, it->location_args);
		if (tmpPrecision > bestMatchPrecision)
		{
			bestMatch = &(*it);
			bestMatchPrecision = tmpPrecision;
		}
	}
	if (!bestMatch)
		return (parent);
	bestMatch = getMatchingLocationBlock(bestMatch, path);
	if (bestMatch->root.empty())
		return (NULL);
	return (bestMatch);
}

short	ServerContext::getPathPrecision(std::vector<std::string>& dest_parts, const std::string& location_path)
{
	typedef std::vector<std::string>::iterator	Iterator;

	std::vector<std::string>	loc_parts = ServerContext::split_with_charset(location_path, "/");
	size_t matching_precision = 0;

	Iterator dest_it = dest_parts.begin();
	Iterator loc_it = loc_parts.begin();
	while (	dest_it != dest_parts.end()
			&& loc_it != loc_parts.end()
			&& *dest_it == *loc_it)
	{
		++matching_precision;
		++loc_it;
		++dest_it;
	}
	if (loc_it == loc_parts.end())
		return (matching_precision);
	return (-1);
}

char **ServerContext::env;

void	ServerContext::set_env(char **env)
{
	ServerContext::env = env;
}

char	**ServerContext::get_env()
{
	return (ServerContext::env);
}
