#pragma once
#ifndef SERVERCONTEXT_HPP
#define SERVERCONTEXT_HPP

#define MAX_CONNECTIONS	5000

struct location_block;

struct redirection
{
	std::string redirect;
	int			statut_code;
};

struct block
{
    std::map<int, std::string> default_page;
    int max_body;
    std::vector<location_block> loc_block;
	std::map<std::string, std::string> cgi;	
    std::vector<std::string> method;
    std::map<std::string, redirection> http_redirect;
    std::string root;
    std::vector<std::string> index;
    char directory_listing;	
    std::string location_args;
};

struct location_block : public block
{
};

struct server_block : public block
{
    std::vector<struct sockaddr_in>	listen;
    std::vector<std::string>		server_name;
	int 							nb;
};

class ServerContext
{
	private :
		static std::vector<server_block>	serverBlocks;
		static char 						**env;
		ServerContext();

        static unsigned short				getDomainPrecision(std::vector<std::string>& hostname_parts, const std::string& servername);
		static short						getPathPrecision(std::vector<std::string>& dest_parts, const std::string& location_path);
		
		/* Any other environment variable needed along the way */
	public :
		~ServerContext();

        static std::vector<server_block>&	getServerBlocks(void);
        static server_block*				getMatchingServerBlock(const std::string& hostname, const sockaddr_in& host);
        static block*						getMatchingLocationBlock(block* parent, const std::string& path);
		static std::vector<std::string>		split_with_charset(const std::string& str, const std::string& delim);
		static std::string					get_server_path(std::string request_path, block* loc);
		static void							set_env(char **env);
		static char 						**get_env();

};

#endif
