#include "webserv.hpp"

size_t	get_tab_len(char **tab)
{
	size_t	len = 0;
	while (tab && tab[len++]);
	return (--len);	
}

char	*concate_c_str(std::string first, std::string second)
{
	char	*str = new char[first.size() + second.size() + 1];
	if (!str)
		return (NULL);
	memcpy(str, first.c_str(), first.size());
	memcpy(str + first.size(), second.c_str(), second.size());
	str[first.size() + second.size()] = 0;
	return (str);
}

void	delete_env(char ***env)
{
	for (size_t i = 0; (*env)[i]; i++)
		delete[] (*env)[i];
	delete[] *env;
	*env = NULL;
}

char	**get_required_env(std::string abs_ressource_path, std::string ressource_path, Client *client)
{
	size_t	tab_len = get_tab_len(ServerContext::get_env());
	char	**env = new char*[tab_len + NB_CGI_ENV + 1];
	if (!env)
		return (NULL);
	size_t i;
	for (i = 0; i < tab_len; i++)
	{
		env[i] = new char[strlen(ServerContext::get_env()[i]) + 1];
		if (!env[i])
		{
			delete_env(&env);	
			return (NULL);
		}
		memcpy(env[i], ServerContext::get_env()[i], strlen(ServerContext::get_env()[i]) + 1);
	}
	size_t	j = i;
	env[j++] = concate_c_str("REQUEST_METHOD=", Request::method_to_str(client->get_request()->get_method()));
	env[j++] = concate_c_str("CONTENT_TYPE=", client->get_request()->get_content_type().type);
	env[j++] = concate_c_str("CONTENT_LENGTH=", client->get_request()->get_header_value("content-length"));
	env[j++] = concate_c_str("SCRIPT_FILENAME=", abs_ressource_path);
	env[j++] = concate_c_str("SCRIPT_NAME=", ressource_path);
	env[j++] = concate_c_str("DOCUMENT_ROOT=", ressource_path.substr(0, ressource_path.rfind('/')));
	env[j++] = concate_c_str("SERVER_PROTOCOL=", "HTTP/1.1");
	env[j++] = concate_c_str("GATEWAY_INTERFACE=", "CGI/1.1");
	env[j++] = concate_c_str("REMOTE_ADDR=", inet_ntoa(client->get_addr().sin_addr));
	env[j++] = concate_c_str("REMOTE_PORT=", to_str(ntohs(client->get_addr().sin_port)));
	env[j++] = concate_c_str("SERVER_ADDR=", inet_ntoa(client->get_socket_addr().sin_addr));
	env[j++] = concate_c_str("SERVER_PORT=", to_str(ntohs(client->get_socket_addr().sin_port)));
	env[j++] = concate_c_str("SERVER_NAME=", *(client->get_server_conf()->server_name.begin()));
	env[j++] = concate_c_str("QUERY_STRING=", client->get_request()->get_query_string());
	env[j++] = concate_c_str("REDIRECT_STATUS=", "200");
	env[j++] = concate_c_str("HTTP_COOKIE=", client->get_request()->get_header_value("cookie"));
	for (size_t k = i; k < j; k++)
	{
		if (!env[k])
		{
			for (size_t l = 0; l < j; l++)
				delete[] env[l];
			delete[] env;
			return (NULL);
		}
	}			
	env[j] = NULL;
	return (env);
}

int	setup_cgi(std::string ext, std::string cgi_path, std::string abs_ressource_path, std::string ressource_path, Client *client)
{
	CGI_save	&cgi = client->get_cgi_save();
	cgi.env = get_required_env(abs_ressource_path, ressource_path, client);
	if (!cgi.env)
		return (1);
	cgi.response = new Response;
	if (!cgi.response)
	{
		delete_env(&cgi.env);
		delete cgi.response;
		return (1);
	}
	if (ext == ".php")
		cgi.response->set_php_cgi(true);
	int	fds_in[2];
	int	fds_out[2];
	if (pipe(fds_in) == -1)
	{
		delete_env(&cgi.env);
		delete cgi.response;
		cgi.response = NULL;
		return (1);
	}
	if (pipe(fds_out) == -1)
	{
		delete_env(&cgi.env);
		delete cgi.response;
		cgi.response = NULL;
		close(fds_in[0]);
		close(fds_in[1]);
		return (1);
	}
	pid_t	pid = fork();
	if (!pid)
	{
		close(fds_in[1]);
		close(fds_out[0]);
		dup2(fds_in[0], STDIN_FILENO);
		close(fds_in[0]);
		dup2(fds_out[1], STDOUT_FILENO);
		close(fds_in[1]);
		char *argv[] = {(char*)cgi_path.c_str(), (char*)abs_ressource_path.c_str(), NULL};
		execve(argv[0], argv, cgi.env);
	}
	else if (pid > 0)
	{
		close(fds_in[0]);
		close(fds_out[1]);
		client->set_cgi_fd_in(fds_in[1]);
		client->set_cgi_fd_out(fds_out[0]);
		client->set_cgi_pid(pid);
	}
	else
	{
		delete_env(&cgi.env);
		delete cgi.response;
		cgi.response = NULL;
		close(fds_in[0]);
		close(fds_in[1]);
		close(fds_out[0]);
		close(fds_out[1]);
		return (1);
	}
	client->set_current_step(TO_CGI);
	return (0);
}

int	write_to_cgi(Client *client)
{
	CGI_save	&cgi = client->get_cgi_save();
	if (write(client->get_cgi_fd_in(), client->get_request()->get_body().data(), client->get_request()->get_body().size()) == -1)
	{
		delete_env(&cgi.env);
		delete cgi.response;
		cgi.response = NULL;
		close(client->get_cgi_fd_out());
		client->set_cgi_fd_out(-1);
		close(client->get_cgi_fd_in());
		client->set_cgi_fd_in(-1);
		return (1);
	}
	close(client->get_cgi_fd_in());
	client->set_cgi_fd_in(-1);
	return (0);
}

int	read_cgi_output(Client *client)
{
	int		status;
	char	buf[READ_SIZE];

	CGI_save	&cgi = client->get_cgi_save();
	int ret = waitpid(client->get_cgi_pid(), &status, WNOHANG);
	if (ret == client->get_cgi_pid())
		client->set_cgi_pid(-1); 
	else if (ret == -1)
	{
		delete_env(&cgi.env);
		delete cgi.response;
		cgi.response = NULL;
		close(client->get_cgi_fd_out());
		return (1);
	}
	if ((cgi.nb_byte = read(client->get_cgi_fd_out(), buf, READ_SIZE)) == -1)
	{
		delete_env(&cgi.env);
		delete cgi.response;
		cgi.response = NULL;
		close(client->get_cgi_fd_out());
		client->set_cgi_fd_out(-1);
		return (1);
	}
	if (cgi.nb_byte > 0)
	{
		Logger(DEBUG) << "Appending " << cgi.nb_byte << " to body";
		ssize_t old = cgi.output.size();
		cgi.output.resize(old + cgi.nb_byte);
		memcpy(&cgi.output[old], buf, cgi.nb_byte);
	}
	if (client->get_cgi_pid() != -1)
		return (0);
	close(client->get_cgi_fd_out());
	client->set_cgi_fd_out(-1);
	delete_env(&cgi.env);
	cgi.env = NULL;
	cgi.response->set_status_code(200);
	cgi.response->set_reason_phrase("Ok");
	cgi.response->set_body(cgi.output);
	cgi.response->set_header_value("connection", "close");
	client->set_response(cgi.response);
	cgi.response = NULL;
	return (0);
}
