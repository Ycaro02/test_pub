#include "webserv.hpp"

bool path_is_dir(const char *path)
{
	DIR *dir;
	dir = opendir(path);
	if (dir == NULL)
		return (false);
	closedir(dir);
	return (true);
}

static std::string add_download_link(std::string &path)
{
	return ("&emsp;&emsp;&emsp;<a href=\"" + path  + "\"download>" + "Download");
}

static std::string build_list(DIR *dir, std::string file_path, std::string query_path)
{
	struct dirent 	*my_dir;
	std::string		page;
	
	if (file_path.size() > 0 && file_path[file_path.size() - 1] != '/')
		file_path += '/';
	if (query_path.size() > 0 && query_path[query_path.size() - 1] != '/')
		query_path += '/';

	page = "<html><body>Directory Listing\n<ul>\n";
	while ((my_dir = readdir(dir)) != NULL)
	{
		std::string	child_file_path = file_path + my_dir->d_name;
		std::string	child_query_path = url_encoding(query_path + my_dir->d_name);
		page += "<li><a href=\"";
		page += child_query_path + "\">" + my_dir->d_name + "</a>";
		if (path_is_dir(child_file_path.c_str()) == false && access(child_file_path.c_str(), R_OK) != -1)
			page += add_download_link(child_query_path);
		page += "</li>\n";
	}
	return (page);
}

static std::string get_directory_page(const std::string& file_path, const std::string& query_path)
{
	DIR			*dir;
	std::string page;

	if (path_is_dir(file_path.c_str()) == true)
	{
		dir = opendir(file_path.c_str());
		page = build_list(dir, file_path, query_path);
		closedir(dir);
	}
	return (page);
}

Response* directory_listing(const std::string& file_path, const std::string& query_path, block *conf)
{
	if (path_is_dir(file_path.c_str()) == true)
	{
		if (conf->directory_listing == 0)
		{
			std::string page = get_directory_page(file_path, query_path);
			Response *response = new Response();
			if (!response)
			{
				Logger(ERROR) << "Fatal : Allocation error";
				return (NULL);
			}
			response->set_status_code(200);
			response->set_reason_phrase("Ok");
			response->set_body(page);
			response->set_header_value("connection", "close");
			return (response);
		}
		else
		{
			Response* response = build_http_error(404, "Not found", conf);
			return (response);
		}
	}
	return (NULL);
}
