#ifndef PARSINGFILE_HPP
#define PARSINGFILE_HPP

#include "webserv.hpp"
/* #include <iostream> */
/* #include <fstream> */
/* #include <cstdlib> */
/* #include <arpa/inet.h> */
/* #include "Logger.hpp" */
/* #include "ServerContext.hpp" */
/* #include "Response.hpp" */

# define MAX_LOC_KEY 7
# define MAX_SERV_KEY 9

class ParsingFile
{
    private :
        const   std::string _file;
        static  std::string _keyword_server[];
        static  std::string _keyword_location[];
        static  int         _status_code[];
        static  std::string DELIM;
        static  int         nb_serv_block;

        ParsingFile&    operator=(const ParsingFile &ref);
        ParsingFile(const ParsingFile &copy);
        bool fill_server_block(std::vector<std::string> &serv_vec, server_block &serv);
        bool build_serv_block(std::vector<std::string> &vec, std::vector<std::string>::iterator &it, int nb);
        void find_block(std::vector<std::string> &file);
        bool parse_config_file(std::ifstream &file, std::vector<std::string> &config);
        bool fill_serv_data(std::string &key, std::string &data, server_block &serv);
        bool fill_location_block(std::vector<std::string> &block, server_block &serv, location_block &loc_bloc, location_block *parent_bloc);
        bool fill_loc_data(std::string &key, std::string &data, location_block *block);
        bool check_loc_doc_key(std::string &key);
        bool build_location_block(std::vector<std::string> &serv_block, std::vector<std::string>::iterator &it, server_block &block, std::string &loc_args, location_block *parent_block);
        bool check_server_key(std::string &key);
        bool check_loc_key(std::string &key);
        void real_split(std::string &src, std::vector<std::string> &vec);
        bool add_method(std::string &data, block *block);
        bool add_redirect(std::string &data, block *block);
        bool add_index(std::string &data, block *block);
        bool add_server_name(std::string &data, server_block &serv);
    	bool add_default_page(std::string &data, block *block);
        bool add_cgi(std::string &data, block *block);
        bool finish_fill_block(std::vector<server_block> &serv);
        void fill_empty_data(block &parent_block, location_block &block);
        void display_result(std::vector<server_block> serv);
        
    public :
        // ParsingFile(const char *str);
        ParsingFile(void);
        ~ParsingFile(void);
        bool parsing(const char *str);
};

void display_serv_blockinfo(std::vector<server_block> &my_serv_vec);
void init_serv_block(server_block &serv_block, int nb);
void init_location_block(location_block &block, server_block &serv, std::string &loc_args, location_block *parent_block);
bool is_comment(std::string &str);
bool my_split(std::string &src, std::string &key, std::string &data, std::string &delims);
bool open_file(std::string path, std::ifstream &file);

bool path_is_dir(const char *path);
std::string directory_listing(const char *file_path, std::string path);
bool update_path(std::string *path, location_block loc, Response *response);
bool str_is_positive_digit(std::string str);


template <typename T, typename U>
void update_map(std::map<T, U> &current, std::map<T, U> &parrent)
{
	for (typename std::map<T, U>::iterator it = parrent.begin(); it != parrent.end(); it++)
	{
		if (current.find(it->first) == current.end())
			current.insert(std::pair<T, U>(*it));
	}
}

#endif
