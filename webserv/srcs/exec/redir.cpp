#include "webserv.hpp"

Response* http_redir(const std::string& file_path, block *conf)
{
	if (conf->http_redirect.find(file_path) != conf->http_redirect.end())
	{
		std::string new_uri = conf->http_redirect.find(file_path)->second.redirect;
		Response* response = new Response();
		if (conf->http_redirect.find(file_path)->second.statut_code == 301)
		{
			response->set_status_code(301);
			response->set_reason_phrase("Moved permanently");
		}
		else
		{
			response->set_status_code(307);
			response->set_reason_phrase("Temporary Redirect");
		}
		response->set_header_value("Location", new_uri);
		return (response);
	}
	return (NULL);
}


std::string return_index_page(std::string file_path, block* conf)
{
	std::string result;
	std::string index_path;

	for (size_t i = 0; i < conf->index.size(); i++)
	{
		index_path = conf->index[i];
		if (index_path.size() > 0 && index_path[0] == '/')
			index_path = index_path.substr(1);
		result = file_path + index_path;
		if (access(result.c_str(), F_OK) == -1)
			continue;
		else
			return (result);
	}
	result = file_path;
	return (result);
}
