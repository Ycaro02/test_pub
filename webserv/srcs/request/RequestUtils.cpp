#include "webserv.hpp"

static std::string build_default_error_page(int status_code, const std::string& reason_phrase)
{
	std::stringstream 	stream;
	std::string			str_status;
	std::string			result;

	stream << status_code;
	stream >> str_status;

	result += "<html><head><title>";
	result += str_status + " " + reason_phrase;
	result += "</title></head><body><style>p{color: red;margin-left: auto;margin-right:auto;width: 6em;font-size: xx-large;}</style><p>";
	result += str_status + " " + reason_phrase;
	result += "</p></body></html>";
	return (result);
}

// Any unsafe characters are gonna be automatically encoded by the browser,
// so we only check for % chars, which cause problems and are not encoded
// automatically. For example if a file contains "%20" in his name, the browser
// will send it as is in the url, and we have no way of knowing if the file name
// contains a space (encoded as %20), or the actual % char.
std::string	url_encoding(const std::string& path)
{
	std::string encoded_path;
	size_t current;
	size_t previous = 0;

	while (previous < path.size())
	{
		current = path.find("%", previous);
		if (current != std::string::npos)
		{
			encoded_path += path.substr(previous, current - previous);
			encoded_path += "%25";
			previous = current + 1;
		}
		else
		{
			encoded_path += path.substr(previous);
			previous = path.size();
		}
	}
	return (encoded_path);
}

std::string	url_decoding(const std::string& path)
{
	std::string	decoded_path;
	size_t		current;
	size_t		previous = 0;

	while (previous < path.size())
	{
		current = path.find("%", previous);
		if (current != std::string::npos)
		{
			decoded_path += path.substr(previous, current - previous);
			decoded_path += (char)(hexa_to_int(path[current + 1]) * 16 + hexa_to_int(path[current + 2]));
			previous = current + 3;
		}
		else
		{
			decoded_path += path.substr(previous);
			previous = path.size();
		}
	}
	Logger(DEBUG) << "URL ENCODING MANAGEMENT\nEncoded : " << path << "\nDecoded : " << decoded_path;
	return (decoded_path);
}

int	build_body_from_file(std::vector<char>* result, const std::string path)
{
	if (!result)
	{
		Logger(ERROR) << "build_vector_from_file has no result to populate";
		return (1);
	}

	if (access(path.c_str(), F_OK) == -1)	
	{
		Logger(ERROR) << "File " << path << " could not be found";
		return (1);
	}

	std::ifstream	file(path.c_str(), std::ifstream::binary);
	if (!file)
	{
		Logger(ERROR) << "File " << path << "  could not be opened";
		return (1);
	}
	
	file.seekg(0, file.end);
	long size = file.tellg();
	if (size == -1)
	{
		Logger(ERROR) << "File " << path << " could not be read";
		return (1);
	}
	file.seekg(0, file.beg);
	result->resize(size);
	char*	buffer = &(*result)[0];
	file.read(buffer, size);
	if (!file)
	{
		Logger(ERROR) << "File " << path << " could not be read";
		return (1);
	}
	return (0);
}

Response	*build_http_error(int status_code, const std::string& reason_phrase, block* conf, Response* tochange)

{
	std::string 		path;
	std::vector<char>	body;
	Response*			response;

	if (tochange)
		response = tochange;
	else
		response = new Response();
	if (!response)
	{
		Logger(ERROR) << "Fatal : allocation error";
		return (NULL);
	}
	response->set_status_code(status_code);
	response->set_reason_phrase(reason_phrase);
	response->set_header_value("Connection", "close");
	if (conf && conf->default_page.find(status_code) != conf->default_page.end())
	{
		path = conf->default_page.find(status_code)->second;
		if (!build_body_from_file(&body, path))
		{
			response->set_body(body);
			return (response);
		}
	}
	std::string page = build_default_error_page(status_code, reason_phrase);
	response->set_body(page);
	return (response);
}
