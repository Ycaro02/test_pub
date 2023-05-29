#include "webserv.hpp"

std::string ParsingFile::_keyword_server[] = {"listen", "server_name", "default_page", "max_body", "http_methods", "http_redirect", "root", "index", "directory_listing", "cgi"};
std::string ParsingFile::_keyword_location[] = {"http_methods", "http_redirect", "root", "index", "directory_listing", "cgi", "default_page", "max_body"};
std::string ParsingFile::DELIM = "\t ";
int			ParsingFile::nb_serv_block = -1;
// Canonical form
ParsingFile::ParsingFile(void) {}

ParsingFile::~ParsingFile(void) {}        

ParsingFile::ParsingFile(const ParsingFile& copy)
{
	*this = copy;
}

ParsingFile&	ParsingFile::operator=(const ParsingFile& ref) 
{
	if (this == &ref)
		return (*this);
	return (*this);
}

// DISPLAY DATA BOCK FOR DEBUG

void display_loc_block(location_block &block)
{
	size_t j = 0;
	Logger(DEBUG) << "[location args] " << block.location_args;
	for (std::vector<std::string>::iterator it = block.method.begin() ; it != block.method.end(); it++)
		Logger(DEBUG) << "[Method] " << *it;
	for (std::map<std::string,	redirection>::iterator it =  block.http_redirect.begin(); it != block.http_redirect.end(); it++)
			Logger(DEBUG) << "[http redirect] : " << it->first << " path : " << it->second.redirect << " code " << it->second.statut_code << " size" << block.http_redirect.size();
	Logger(DEBUG) << "[root] : " << block.root;
	for (std::vector<std::string>::iterator it = block.index.begin() ; it != block.index.end(); it++)
		Logger(DEBUG) << "[index] = " << *it;
	for (std::map<std::string, std::string>::iterator it =  block.cgi.begin(); it != block.cgi.end(); it++)
		Logger(DEBUG) << "[cgi ext]: " << it->first << " cgi path : " << it->second;
	if (block.directory_listing == 0)
		Logger(DEBUG) << "[directory listing] == on";
	else
		Logger(DEBUG) << "[directory listing] == off";
	
	for (std::map<int, std::string>::iterator it = block.default_page.begin(); it != block.default_page.end(); it++)
		Logger(DEBUG) << "[default_page code] = " << it->first << " path = " << it->second;	
	Logger(DEBUG) << "[max body] = " << block.max_body;	
	while (block.loc_block.size() != 0 && j != block.loc_block.size())
	{
		Logger(DEBUG) << "\nNEW SUBBLOCK parent args : " << block.location_args;
		display_loc_block(block.loc_block[j]);
		j++;
	}
}

void display_serv_blockinfo(std::vector<server_block> &my_serv_vec)
{
	server_block serv;
	size_t i = 0;
	for (std::vector<server_block>::iterator serv_it = my_serv_vec.begin(); serv_it != my_serv_vec.end(); serv_it++)
	{
		Logger(DEBUG) << "\nServer block : ";
		serv = *serv_it;       
		if (serv.listen.size() == 0)
		{
			Logger(ERROR) << "listen data not found\n";
			return;
		}
		for (std::vector<struct sockaddr_in>::iterator it = serv.listen.begin(); it != serv.listen.end(); it++)
			Logger(DEBUG) << "[listen ip] = " << inet_ntoa((*it).sin_addr) << " [port] = " << ntohs((*it).sin_port);
		for (std::vector<std::string>::iterator it = serv.server_name.begin(); it != serv.server_name.end(); it++)
			Logger(DEBUG) << "[server_name] : " << *it;
		Logger(DEBUG) << "[max_body] : " << serv.max_body;
		for (std::map<int, std::string>::iterator it =  serv.default_page.begin(); it != serv.default_page.end(); it++)
			Logger(DEBUG) << "[default page code] : " << it->first << " [path] : " << it->second;
		for (std::vector<std::string>::iterator it = serv.method.begin() ; it != serv.method.end(); it++)
			Logger(DEBUG) << *it;
		for (std::map<std::string,	redirection>::iterator it =  serv.http_redirect.begin(); it != serv.http_redirect.end(); it++)
			Logger(DEBUG) << "[http redirect] : " << it->first << " [path] : " << it->second.redirect << " code " << it->second.statut_code;
		Logger(DEBUG) << "[root] : " << serv.root;
		for (std::vector<std::string>::iterator it = serv.index.begin() ; it != serv.index.end(); it++)
			Logger(DEBUG) << "[index] = " << *it;
		for (std::map<std::string, std::string>::iterator it =  serv.cgi.begin(); it != serv.cgi.end(); it++)
			Logger(DEBUG) << "[cgi ext]: " << it->first << " [cgi path] : " << it->second;
		if (serv.directory_listing == 0)
			Logger(DEBUG) << "[directory listing] == on\n";
		else
			Logger(DEBUG) << "[directory listing ]== off\n";
		while (serv.loc_block.size() != 0 && i != serv.loc_block.size())
		{
			Logger(DEBUG) << "\nParent of this block is server, nb " << i;
			display_loc_block(serv.loc_block[i]);
			i++;
		}
		i = 0;
	}
}

// end DISPLAY FUNCTION

static bool check_open_bracket(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	if (it != end)
	{
		if (*it != "{")
		{
			Logger(ERROR) << "it = " << *it  << "Error : need bracket after location instruction";
			return (false);
		}
		it++;
	}
	else
	{
		Logger(ERROR) << "Error : need bracket after location instruction\n";
		return (false);
	}
	return (true);
}

bool ParsingFile::build_location_block(std::vector<std::string> &serv_block, std::vector<std::string>::iterator &it, server_block &block, std::string &loc_args, location_block *parent_block)
{
	std::vector<std::string> location;
	std::string key;
	std::string data;
	
	location_block loc_block;
	init_location_block(loc_block, block, loc_args, parent_block);	
	
	it++; // skip location
	if (check_open_bracket(it, serv_block.end()) == false)
		return (false);
	while (it != serv_block.end())
	{
		if (*it == "}")
		{
			it++;
			break;
		}
		if (my_split(*it, key, data, DELIM) == false)
			return (false);
		if (key == "location")
		{
			if (build_location_block(serv_block, it, block, data, &loc_block) == false)
				return (false);
			if (it == serv_block.end())
				break ;
			else
				continue ;
		}
		else if (check_loc_key(key) == false)
		{
			Logger(ERROR) << "Invalid instruction found key " << key;
			return (false);
		}
		location.push_back(*it);
		it++;
	}
		if (fill_location_block(location, block, loc_block, parent_block) == false) // add ref and pointer in fill lcoation block
			return (false);
	return (true);
}

bool ParsingFile::build_serv_block(std::vector<std::string> &vec, std::vector<std::string>::iterator &it, int nb)
{
	server_block my_serv;
	std::vector<std::string> serv_block;
	int nb_sub_block = 0;
  
	init_serv_block(my_serv, nb);
	it++;   // skip {
	while (it != vec.end())
	{
		if ((*it).find("{") < (*it).length())
			nb_sub_block++;
		else if (*it == "}")
		{
			if (nb_sub_block == 0)
				break;
			nb_sub_block--;
		}
		serv_block.push_back(*it);
		it++;
	}
	if (it != vec.end() && *it == "}")
		it++;       // skip '}'
	else
	{
		Logger(ERROR) << "Expected close bracket a the end of server block";
		return (false);
	}
	if (fill_server_block(serv_block, my_serv) == false)
		return (false);
	return (true);
}

bool find_next_server_block(std::vector<std::string> &file, std::vector<std::string>::iterator &it)
{
	std::vector<std::string>::iterator it_cpy = it;
	while (it_cpy != file.end())
	{
		if (*it_cpy == "server")
		{
			it_cpy--;
			it = it_cpy;
			return (true);
		}
		it_cpy++;
	}
	return (false);
}

void ParsingFile::find_block(std::vector<std::string> &file)
{
	for (std::vector<std::string>::iterator it = file.begin(); it != file.end(); it++)
	{
		if (*it == "server")
		{
			this->nb_serv_block++;
			if (it != file.end())
				it++;
			if (it != file.end() && *it == "{")
			{
				if (build_serv_block(file, it, this->nb_serv_block) == false)
				{
					if (find_next_server_block(file, it) == true)
						continue;
					break;
				}
				if (it != file.end() && (*it == "server"))
					it--;
				else if (it != file.end())
				{
					Logger(ERROR) << "Invalid instruction " << *it;
					if (find_next_server_block(file, it) == true)
						continue;
					break;
				}
			}
			else
			{
				Logger(ERROR) << "Need only bracket after instruction server : " << "\n";
				if (find_next_server_block(file, it) == true)
						continue;
				break;
			}
		}
		else if (it != file.end())
		{
			Logger(ERROR) << "Server instruction expected line =  " << *it;
			if (find_next_server_block(file, it) == true)
				continue;
			break;
		}
		if (it == file.end())
			break ;
	}
}

bool ParsingFile::fill_location_block(std::vector<std::string> &block, server_block &serv, location_block &loc_block, location_block *parent_block)
{
	std::string key;
	std::string tmp;

	for (std::vector<std::string>::iterator it = block.begin(); it != block.end(); it++)
	{
		if (my_split(*it, key, tmp, DELIM) == false)
			return (false);
		for (unsigned int i = 0; i <= MAX_LOC_KEY; i++)
		{
			if (key == _keyword_location[i])
			{
				if (fill_loc_data(key, tmp, &loc_block) == false)
					return (false);
				break;
			}
			else if (key != _keyword_location[i] && i == MAX_LOC_KEY)
			{
				Logger(ERROR) << "Bad instruction in location block key : " << key << '\n';
				return (false);
			}
		}
	}
	if (parent_block != NULL)
		parent_block->loc_block.push_back(loc_block);
	else
		serv.loc_block.push_back(loc_block);
	return (true);
}

static bool check_server_block(server_block &serv)
{
	if (serv.loc_block.size() == 0)
	{
		Logger(ERROR) << "Need at least one location block in each server";
		return (false);
	}
	if (serv.listen.empty())
	{
		Logger(ERROR) << "Invalid server_block, listen value expected";
		return (false);
	}
	return (true);
}

bool ParsingFile::fill_server_block(std::vector<std::string> &serv_vec, server_block &serv)
{
	std::string key;
	std::string data;
	location_block parent_block;
	std::vector<std::string>::iterator it= serv_vec.begin();

	while (it != serv_vec.end())
	{
		if (my_split(*it, key, data, DELIM) == true)
		{  
			if (check_server_key(key) == true)
			{
				if (fill_serv_data(key, data, serv) == false)
					return (false);
			}
			else if (key == "location")
			{
				if (build_location_block(serv_vec, it, serv, data, NULL) == false)
					return (false);
				if (it == serv_vec.end())
					break ;
				else
					continue;
			}
			else
			{
				Logger(ERROR) << "Invalid line : " << *it << '\n';
				return (false);
			}
			if (it == serv_vec.end())
				break ;
		}
		else
		{
			Logger(ERROR) << "Invalid line : " << *it << '\n';
			return (false);
		}
		if (it != serv_vec.end())
			it++;
	}
	if (check_server_block(serv) == false)
		return (false);
	ServerContext::getServerBlocks().push_back(serv);
	return (true);
}

bool ParsingFile::parse_config_file(std::ifstream &file, std::vector<std::string> &config)
{   
	char c;
	std::string str;

	while (file.get(c))
	{
		str += c;
		if (c == '\n')
		{
			str_trim(str);
			if (str.empty())
				continue;
			if (is_comment(str) == false)
			   config.push_back(str);
			str.clear();
		}
	}
	if (str.size() != 0)
	{
		str_trim(str);
		if (!str.empty() && is_comment(str) == false)
			config.push_back(str);
	}
	file.close();
	if (config.size() <= 5)
	{
		Logger(ERROR) << "Invalid config file";
		return (false);
	}
	find_block(config);
	return (true);
}

void	ParsingFile::display_result(std::vector<server_block> serv)
{
	std::vector<int> nb_find;
	int j = 0;
	for (size_t i = 0; i != serv.size(); i++)
	{
		Logger(INFO) << "Server block nb " << serv[i].nb << " succes\n";
		nb_find.push_back(serv[i].nb);
	}

	while (j <= this->nb_serv_block)
	{
		bool find = false;
		for (size_t i = 0; i != nb_find.size(); i++)
		{
			if (j == nb_find[i])
				find = true;
		}
		if (find == false)
			Logger(INFO) << "Server block nb " << j << " failed" << "\n";
		j++;
	}
}

bool ParsingFile::parsing(const char *str)
{
	std::ifstream file;
	std::vector<std::string> config;
	if (open_file(str, file) == false)
		return (false);
	
	if (parse_config_file(file, config) == false)
		return(false);
	if (finish_fill_block(ServerContext::getServerBlocks()) == false)
		return (false);
	if (ServerContext::getServerBlocks().size() == 0)
	{
		Logger(ERROR) << "No valid server_block found";
		return (false);
	}
	Logger::setCurrentLevel(INFO);
	display_result(ServerContext::getServerBlocks());
	// Logger::setCurrentLevel(DEBUG);
	// display_serv_blockinfo(ServerContext::getServerBlocks());
	return(true);
}
