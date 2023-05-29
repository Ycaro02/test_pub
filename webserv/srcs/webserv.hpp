#pragma once
#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <list>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <dirent.h>

#include "utils/Logger.hpp"
#include "config/ServerContext.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "config/ParsingFile.hpp"
#include "server/Client.hpp"
#include "server/Socket.hpp"
#include "server/Server.hpp"

#define NB_CGI_ENV 16
#define READ_SIZE 8192

// utils/ParsingUtils.cpp
int 		hexa_to_int(char c);
std::string	str_to_lower(const std::string str);
void 		str_trim(std::string &str, const std::string white_space = "\t\n\v\f\r ");
bool		compare_ignore_case(const std::string first, const std::string second);
std::string	get_next_line(std::string &str, const std::string delim = "\r\n");
bool		str_end_with(std::string str, std::string end);
std::string	to_str(unsigned int nb);
std::string	get_extension(std::string str);

// request/RequestUtils.cpp
int			build_body_from_file(std::vector<char>* result, const std::string path);
Response*	build_http_error(int status_code, const std::string& reason_phrase, block* conf, Response* tochange = NULL);
std::string	url_encoding(const std::string& path);
std::string	url_decoding(const std::string& path);

// /DirectoryListing.cpp
std::string return_index_page(std::string file_path, block* conf);
Response* http_redir(const std::string& file_path, block *conf);
Response* directory_listing(const std::string& file_path, const std::string& query_path, block *conf);

// CGI/CGI.cpp
int	setup_cgi(std::string ext, std::string cgi_path, std::string abs_ressource_path, std::string ressource_path, Client *client);
int	write_to_cgi(Client *client);
int	check_cgi_process(Client *client);
int	read_cgi_output(Client *client);
void	delete_env(char ***env);


//exec/set_content_type.cpp
std::string set_content_type(std::string ext);

#endif
