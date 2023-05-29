#include "webserv.hpp"

void init_serv_block(server_block &serv_block, int nb)
{
	Logger(INFO) << "server block initialised";
	serv_block.default_page.clear();
	serv_block.listen.clear();
	serv_block.loc_block.clear();
	serv_block.server_name.clear();
	serv_block.max_body = -1;
	serv_block.method.clear();
	serv_block.http_redirect.clear();
	serv_block.root.clear();
	serv_block.cgi.clear();
	serv_block.index.clear();
	serv_block.directory_listing = -1;
	serv_block.location_args = "/";
	serv_block.nb = nb;
}

void init_location_block(location_block &block, server_block &serv, std::string &loc_args, location_block *parent_block)
{
	Logger(INFO) << "location block initialised\n";
	if (parent_block != NULL)
	{
		block.location_args = loc_args;
		block.method = parent_block->method;
		block.http_redirect = parent_block->http_redirect;
		block.root = parent_block->root;
		block.index = parent_block->index;
		block.cgi = parent_block->cgi;
		block.directory_listing = parent_block->directory_listing;
		block.default_page = parent_block->default_page;
		block.max_body = parent_block->max_body;
		block.loc_block.clear();
	}
	else
	{		
		block.location_args = loc_args;
		block.method = serv.method;
		block.http_redirect = serv.http_redirect;
		block.root = serv.root;
		block.index = serv.index;
		block.cgi = serv.cgi;
		block.directory_listing = serv.directory_listing;
		block.default_page = serv.default_page;
		block.max_body = serv.max_body;
		block.loc_block.clear();
	}
}

bool ParsingFile::add_server_name(std::string &data, server_block &serv)
{
	std::vector<std::string> tmp;
	real_split(data, tmp);
	if (tmp.size() == 0)
	{
		Logger(ERROR) << "Argument expected after instruction server_name";
		return (false);
	}
	for(std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end(); it++)
		serv.server_name.push_back(*it);
	return (true);
}

static bool get_ip_addrr(std::string &data, size_t sep, struct sockaddr_in &ip_addr)
{
	struct in_addr loc;
	if (inet_aton(data.substr(0, sep).c_str(), &loc) == 0)
	{
		Logger(ERROR) << "invalid adress\n";
		return (false);
	}
	ip_addr.sin_addr = loc;
	return (true);
}

static bool get_port(std::string &data, size_t sep, in_port_t &port)
{
	if (str_is_positive_digit(data.substr(sep + 1)) == false)
		return (false);
	if (strtod(data.substr(sep + 1).c_str(), NULL) < 0.0 || strtod(data.substr(1).c_str(), NULL) > 65535)
	{
		Logger(ERROR) << "Invalid number for port";
		return (false);
	}
	port = htons(atoi(data.substr(1).c_str())); // checked with ntohs
	return (true);
}

static bool add_listen(std::string &data, server_block &serv)
{
	size_t sep = data.find(':');
	struct sockaddr_in ip_addr;
	ip_addr.sin_family = AF_INET;
	ip_addr.sin_addr.s_addr = INADDR_ANY;
	in_port_t port = 0;
	if (sep <= data.length())
	{
		if (sep == data.length() - 1 && sep != 0)
		{
			Logger(ERROR) << "Number of port expected in listen instruction";
			return (false);
		}
		else if (sep == 0)
		{
			if (get_port(data, sep, port) == false)
				return (false);
			ip_addr.sin_port = port;
		}
		else
		{
			std::string tmp = data.substr(sep);
			if (get_ip_addrr(data, sep, ip_addr) == false || get_port(tmp, 0, port) == false)
				return (false);
			ip_addr.sin_port = port;
		}
		if (port == 0)
		{
			Logger(ERROR) << "No port found for listen inctruction (port == 0)";
			return (false);
		}
		serv.listen.push_back(ip_addr);
	}
	else
	{
		Logger(ERROR) << " ':' expected in listen args";
		return (false);
	}
	return (true);
}

bool ParsingFile::add_method(std::string &data, block *block)
{
	std::string tmp = data;
	std::vector<std::string> vec;

	real_split(tmp, vec);
	if (vec.size() == 0)
	{
		Logger(ERROR) << "Argument expected after http method instruction";
		return (false);
	}
	for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++)
	{
		if (*it == "GET" || *it == "POST" || *it == "DELETE")
			block->method.push_back(*it);
		else
		{
			Logger(ERROR) << "Unknow method " << *it << '\n';
			return (false);
		}
	}
	return (true);
}

bool ParsingFile::add_redirect(std::string &data, block *block)
{
	std::vector<std::string> vec;
	redirection redir;
	real_split(data, vec);
	if (vec.size() != 3)
	{
		Logger(ERROR) << "3 argument expected for http_redirect";
		return (false);
	}
	if (str_is_positive_digit(vec[0]) == false)
		return (false);
	double my_double = strtod(vec[0].c_str(), NULL);
	if (my_double != 301 && my_double != 307) // to_check
	{
		Logger(ERROR) << "Expected a valid http status code (301 || 307) at first argument for http_redirect";
		return (false);
	}
	redir.statut_code = my_double;
	redir.redirect = vec[2];
	block->http_redirect.insert(std::pair<std::string, redirection>(vec[1], redir));
	return (true);
}

static bool add_directory_listing(std::string &data, block *block)
{
	if (data == "on")
		block->directory_listing = 0;
	else if (data == "off")
		block->directory_listing = 1;
	else
	{
		Logger(ERROR) << "Bad parametre for directory_listing expected on/off\n";
		return (false);
	}
	return (true);
}

bool ParsingFile::add_index(std::string &data, block *block)
{
	std::string tmp = data;
	std::vector<std::string> vec;

	real_split(tmp, vec);
	if (vec.size() == 0)
	{
		Logger(ERROR) << "Argument expected after index instruction";
		return (false);
	}
	for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++)
		block->index.push_back(*it);
	return (true);
}

static bool add_max_body(std::string data, block *block)
{
	if (str_is_positive_digit(data) == false)
		return (false);
	double size = strtod(data.c_str(), NULL);
	if (size < 16777216.0 || size > 0) // 16mb max body_size maybe change
		block->max_body = size;
	else
	{
		Logger(ERROR) << "Invalid patern found for max_body " << data << '\n';
		return (false);
	}
	return (true);
}

static bool add_root(std::string &data, block *block)
{
	if (data.size() > 0 && data[data.size() - 1] != '/')
		data += "/";
	block->root = data;
	return (true);
}

bool ParsingFile::add_default_page(std::string &data, block *block)
{
	std::vector<std::string>	vec;
	std::string 				tmp = data;
	std::ifstream 				file;
	double 						status_code = 0;

	real_split(tmp, vec);
	if (vec.size() != 2)
	{
		Logger(ERROR) << "Error, 2 argument expected for default page (code path)\n";
		return (false);
	}
	if (str_is_positive_digit(vec[0]) == false)
		return (false);
	status_code = strtod(vec[0].c_str(), NULL);
	if (status_code < 100.0 || status_code > 527.0) // status code too ckeck
	{
		Logger(ERROR) << "Status code error for default page :: " << status_code << "\n";
		return (false);
	}
	if (open_file(vec[1], file) == false)
	{
	    Logger(ERROR) << "Can't open in add_default_page" << tmp << '\n';
	    return (false);
	}
	file.close();
	block->default_page.insert(std::pair<int, std::string>(status_code, vec[1]));
	return (true);
}

bool ParsingFile::add_cgi(std::string &data, block *block)
{
	std::vector<std::string> vec;
	real_split(data, vec);
	if (vec.size() != 2)
	{
		Logger(ERROR) << "2 argument expected after cgi instruction";
		return (false);
	}
	if (vec[0][0] != '.')
		vec[0].insert(vec[0].begin(), '.');
	if (access(vec[1].c_str(), F_OK | X_OK) == -1)
		Logger(ERROR) << "Incorect path for cgi : " << vec[1];
	else
		block->cgi.insert(std::pair<std::string, std::string>(vec[0], vec[1]));
	return (true);
}

bool ParsingFile::fill_loc_data(std::string &key, std::string &data, location_block *block)
{
	if (key == "http_methods")
		return (add_method(data, block));
	else if (key == "http_redirect")
		return (add_redirect(data, block));
	else if (key == "root")
		return (add_root(data, block));
	else if (key == "index")
		return (add_index(data, block));
	else if (key == "directory_listing")
		return (add_directory_listing(data, block));
	else if (key == "cgi")
		return (add_cgi(data, block));
	else if (key == "default_page")
		return (add_default_page(data, block));
	else if (key == "max_body")
		return (add_max_body(data, block));
	else if (key == "location")
		return (true);
	else
		return (false);
	return (true);
}

bool ParsingFile::fill_serv_data(std::string &key, std::string &data, server_block &serv)
{
	if (data.empty())
	{  
		Logger(ERROR) << "Empty data for key : " << key << "\n";
		return (false);
	}
	if (key == "listen")
		return (add_listen(data, serv));
	else if (key == "server_name")
		return (add_server_name(data, serv));
	else if (key == "max_body")
		return (add_max_body(data, &serv));
	else if (key == "default_page")
		return (add_default_page(data, &serv));
	else if (key == "http_methods")
		return (add_method(data, &serv));
	else if (key == "http_redirect")
		return (add_redirect(data, &serv));
	else if (key == "root")
		return (add_root(data, &serv));
	else if (key == "cgi")
		return (add_cgi(data, &serv));
	else if (key == "index")
		return (add_index(data, &serv));
	else if (key == "directory_listing")
		return (add_directory_listing(data, &serv));
	else
		return (false);
	return (true);
}

static void update_string_vec(std::vector<std::string> &parent, std::vector<std::string> &current)
{
	for (size_t i = 0; i < parent.size(); i++)
	{
		for (size_t j = 0; j < current.size(); j++)
		{
			if (parent[i] == current[j])
				break;
			if (j == current.size() - 1)
				current.push_back(parent[i]);
		}
	}
}

void ParsingFile::fill_empty_data(block &parent_block, location_block &block)
{
	if (block.cgi.size() == 0)
		block.cgi = parent_block.cgi;
	else
		update_map(block.cgi, parent_block.cgi);
	if (block.default_page.size() == 0)
		block.default_page = parent_block.default_page;
	else
		update_map(block.default_page, parent_block.default_page);
	if (block.http_redirect.size() == 0)
		block.http_redirect = parent_block.http_redirect;
	else
		update_map(block.http_redirect, parent_block.http_redirect);
	if (block.index.size() == 0)
		block.index = parent_block.index;
	else
		update_string_vec(parent_block.index, block.index);
	if (block.directory_listing == -1)
		block.directory_listing = parent_block.directory_listing;
	if (block.method.size() == 0)
		block.method = parent_block.method;
	else
		update_string_vec(parent_block.method, block.method);		
	if (block.max_body == -1)
		block.max_body = parent_block.max_body;
	if (block.root.empty())
		block.root = parent_block.root;
	if (block.loc_block.size() != 0)
	{
		for (size_t i = 0; i != block.loc_block.size(); i++)
			fill_empty_data(block, block.loc_block[i]);
	}
}

void check_root(std::vector<server_block> &serv)
{
	size_t i = 0;
	while (i < serv.size())
	{
		bool erased = false;
		for (size_t j = 0; j != serv[i].loc_block.size(); j++)
		{	
			if (serv[i].loc_block[j].root.empty())
			{
				Logger(DEBUG) << "In location block : " << serv[i].loc_block[j].location_args;
				Logger(ERROR) << "Error root value expected for each location block";
				serv.erase(serv.begin() + i);
				erased = true;
				break;
			}
		}
		if (erased == false)
			i++;
		else
			i = 0;
	}
}

bool ParsingFile::finish_fill_block(std::vector<server_block> &serv)
{

	for (size_t i = 0; i != serv.size(); i++)
	{
		for (size_t j = 0; j != serv[i].loc_block.size(); j++)
			fill_empty_data(serv[i], serv[i].loc_block[j]);
	}
	check_root(serv);
	return (true);
}
