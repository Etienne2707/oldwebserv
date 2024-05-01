#include "process_request.hpp"

std::string	uint_to_str(unsigned int n)
{
	std::ostringstream	str;
	
	str << n;
	return str.str();
}

std::string	int_to_str(int n)
{
	std::ostringstream	str;
	
	str << n;
	return str.str();
}

std::string	method_to_str(int method)
{
	switch (method)
	{
		case GET:
			return "GET";
			break;
		case POST:
			return "POST";
			break;
		case DELETE:
			return "DELETE";
			break;
	}
	return "INVALID METHOD";
}

std::string	find_server_name(const Request& request, const ServerConfig& config)
{
	std::vector<std::string>	server_names = config.getServer_names();

	if (server_names.size() == 1)
		return server_names[0];
	else
		return request.getHost();
}

void	fill_env(char*** env, const std::string& var, const std::string& value)
{
	int	i = 0;
	int	j;
	char **new_env;

	if (*env)
	{
		while ((*env)[i])
			i++;
	}
	new_env = new char*[i + 2];
	std::memmove(new_env, *env, sizeof(char*) * (i + 1));
	if (*env)
		delete[] *env;
	new_env[i] = new char[var.length() + value.length() + 2];
	j = var.length();
	std::memmove(new_env[i], var.c_str(), sizeof(char) * var.length());
	new_env[i][j] = '=';
	std::memmove(new_env[i] + j + 1, value.c_str(), sizeof(char) * value.length());
	j += value.length();
	new_env[i][j + 1] = '\0';
	new_env[i + 1] = NULL;
	*env = new_env;
}

void	clear_env(char** env)
{
	for (int i = 0; env[i]; i++)
		delete[] env[i];
	delete[] env;
}

char**	create_env(const t_client& client, const std::string& script)
{
	char**	env = new char*[1];

	*env = NULL;
	if (client.request.getContentLength())
		fill_env(&env, "CONTENT_LENGTH", uint_to_str(client.request.getContentLength()));
	if (!client.request.getContentType().empty())
		fill_env(&env, "CONTENT_TYPE", client.request.getContentType());
	fill_env(&env, "GATEWAY_INTERFACE", "CGI/1.1");
	fill_env(&env, "PATH_INFO", client.request.getPathInfo());
	fill_env(&env, "PATH_TRANSLATED", client.request.getPathTranslated());
	fill_env(&env, "QUERY_STRING", client.request.getQuery());
	fill_env(&env, "REMOTE_ADDR", client.ip);
	fill_env(&env, "REQUEST_METHOD", method_to_str(client.request.getMethod()));
	fill_env(&env, "SCRIPT_NAME", script);
	fill_env(&env, "SERVER_NAME", find_server_name(client.request, client.request.getServerConfig()));
	fill_env(&env, "SERVER_PORT", int_to_str(client.port));
	fill_env(&env, "SERVER_PROTOCOL", "HTTP/1.1");
	fill_env(&env, "SERVER_SOFTWARE", "webserv");

	return env;
}

void	process_cgi(t_client& client, const std::string& script, int packet_fd, const int& epoll_fd, std::map<int, int>& ongoing_cgi)
{
	pid_t	pid;
	int		pipefd[2];
	char*	args[3];

	if (pipe(pipefd) == -1)
	{
		sendBasicErrorResponse(500, client);
		return;
	}
	pid = fork();
	if (pid == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		sendBasicErrorResponse(500, client);
		return;
	}
	else if (pid == 0)
	{
		char**	env;

		close(pipefd[0]);

		if (dup2(pipefd[1], 1) == -1)
		{
			close(pipefd[1]);
			std::cerr << "Internal error" << std::endl;
			exit(1);
		}
		close(pipefd[1]);

		env = create_env(client, script);
		args[0] = new char[script.length() + 1];
		std::memmove(args[0], script.c_str(), sizeof(char) * (script.length() + 1));
		args[1] = new char[client.request.getPathTranslated().length() + 1];
		std::memmove(args[1], client.request.getPathTranslated().c_str(), sizeof(char) * (client.request.getPathTranslated().length() + 1));
		args[2] = NULL;
		execve(script.c_str(), args, env);

		std::cerr << "Internal error" << std::endl;

		delete[] args[0];
		delete[] args[1];
		clear_env(env);
		exit(1);
	}
	else
	{
		struct epoll_event	event;

		close(pipefd[1]);

		event.events = EPOLLIN;
		event.data.fd = pipefd[0];
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipefd[0], &event) == -1)
			std::cerr << "Cool" << std::endl;
		ongoing_cgi[pipefd[0]] = packet_fd;
	}
}

bool	handle_cgi(t_client& client, const std::vector<ServerLocation>& locations, int packet_fd, const int& epoll_fd, std::map<int, int>& ongoing_cgi)
{

    for (std::vector<ServerLocation>::const_iterator it = locations.begin(); it != locations.end(); it++)
	{
		//std::cout << "Location : " << (*it).getLocation() << std::endl; // DEBUG PURPOSE
		for (std::map<std::string, std::string>::const_iterator jt = it->getCgi().begin(); jt != it->getCgi().end(); jt++)
		{
			//std::cout << "Client getPath : " << client.request.getPath() << " | jt->first : " << jt->first << std::endl; //DEBUG PURPOSE
			if (client.request.getPath().length() > jt->first.length() && !client.request.getPath().compare(client.request.getPath().length() - jt->first.length(), jt->first.length(), jt->first))
			{
				process_cgi(client, jt->second, packet_fd, epoll_fd, ongoing_cgi);
				return true;
			}
		}
	}
	return false;
}
