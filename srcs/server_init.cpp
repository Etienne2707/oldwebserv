#include "server_init.hpp"

bool	g_stop = false;

void	handle_signal(int sig)
{
	(void) sig;
	g_stop = true;
}

void	bind_server_socket(std::map<int, int>& server_sockets, const int& port, const int& epoll_fd)
{
	struct sockaddr_in	server_addr;
	struct epoll_event	event;
	int					enable = 1;
	int					server_sock;


	if (server_sockets.count(port))
		return;
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0)
		throw std::runtime_error(strerror(errno));
	std::cout << "TCP server socket successfully created." << std::endl;

	fcntl(server_sock, F_SETFL, O_NONBLOCK);

	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) 
		throw std::runtime_error(strerror(errno));
	if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw std::runtime_error(strerror(errno));
	std::cout << "Bind to the port number: " << port << std::endl;
	event.events = EPOLLIN;
	event.data.fd = server_sock;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock, &event) == -1)
		throw std::runtime_error(strerror(errno));
	if ( listen(server_sock, SOMAXCONN) < 0)
		throw std::runtime_error(strerror(errno));
	std::cout << "Waiting client connection" << std::endl;
	server_sockets[port] = server_sock;
}

void	add_client_to_epoll(const int& epoll_fd, const int& client_sock)
{
	struct epoll_event event;

    event.events = EPOLLIN | EPOLLOUT;
    event.data.fd = client_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &event) == -1)
		std::cerr << "Error adding client socket to epoll: " << strerror(errno) << std::endl;
}

int	match_socket(const std::map<int, int>& server_sockets, const int& fd)
{
	for (std::map<int, int>::const_iterator it = server_sockets.begin(); it != server_sockets.end(); it++)
	{
		if (fd == it->second)
			return it->first;
	}

	return -1;
}

void	register_client(const int& epoll_fd, const int& server_socket, std::map<int, t_client>& ongoing_requests, int port)
{
	struct sockaddr_in	client_addr;
	std::ostringstream 	ip_stream;
	std::string 		ip_str;


	socklen_t addr_size = sizeof(client_addr);
	int	client_sock;
	client_sock = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
	std::cout << "Client connected." << std::endl;
    unsigned char *ip_bytes = (unsigned char *)&(client_addr.sin_addr);
    ip_stream << static_cast<int>(ip_bytes[0]) << "." 
              << static_cast<int>(ip_bytes[1]) << "." 
              << static_cast<int>(ip_bytes[2]) << "." 
              << static_cast<int>(ip_bytes[3]);
    ip_str = ip_stream.str();
	fcntl(client_sock, F_SETFL, O_NONBLOCK);
	add_client_to_epoll(epoll_fd, client_sock);

	t_client	client;
	client.port = port;
	client.ip = ip_str;
	ongoing_requests[client_sock] = client;

}

void	handle_client(const std::vector<ServerConfig> &config, const int& epoll_fd, struct epoll_event event, t_ongoing& ongoing)
{
	if (event.events & EPOLLIN)
	{
		char buffer[8192];
		ssize_t bytes_received;
		bytes_received = recv(event.data.fd, buffer, sizeof(buffer), 0);
		if (bytes_received == -1)
		{
			std::cerr << "recv error: " << strerror(errno) << std::endl; //DEBUG PURPOSE
			ongoing.requests[event.data.fd].request.clear();
			if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, NULL) < 0)
				std::cout << "epollctl error : " << strerror(errno) <<std::endl; //DEBUG PURPOSE
			close(event.data.fd);
		}
		else if (event.events & (EPOLLHUP | EPOLLERR) || bytes_received == 0)
		{
				std::cout << "Client disconnected." << std::endl;
				ongoing.requests[event.data.fd].request.clear();
				if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, NULL) < 0)
					std::cout << "epollctl error : " << strerror(errno) <<std::endl; //DEBUG PURPOSE
				close(event.data.fd);

		}
		else
		{
			std::string	raw(buffer, buffer + bytes_received);
			if (process_request(config, raw, ongoing, event.data.fd, epoll_fd) < 0 )
				ongoing.requests[event.data.fd].request.clear();
		}
	}
	else if (event.events & EPOLLOUT)
	{
		send(event.data.fd, ongoing.requests[event.data.fd].raw_response.c_str(), ongoing.requests[event.data.fd].raw_response.length(), 0);
	}
}

void	handle_cgi_client(const int& epoll_fd, struct epoll_event event, t_ongoing& ongoing)
{
	char 	buffer[8192];
	ssize_t bytes_received;

	bytes_received = read(event.data.fd, buffer, sizeof(buffer));
	if (bytes_received == -1)
	{
		std::cerr << "read error: " << strerror(errno) << std::endl;
		sendBasicErrorResponse(500, ongoing.requests[ongoing.cgis[event.data.fd]]);
		ongoing.requests[ongoing.cgis[event.data.fd]].request.clear();
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, NULL);
		close(event.data.fd);
		ongoing.cgis.erase(event.data.fd);
	}
	else
	{
		std::string	raw(buffer, buffer + bytes_received);
		std::cout << "CGI returned :" << std::endl << "\t---" << std::endl << raw << std::endl << "\t---" << std::endl;
		sendBasicErrorResponse(302, ongoing.requests[ongoing.cgis[event.data.fd]]);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, NULL);
		close(event.data.fd);
		ongoing.cgis.erase(event.data.fd);
	}
}

void	server_main_loop(int& epoll_fd, const std::vector<ServerConfig>& config, const std::map<int, int>& server_sockets)
{
	std::vector<struct epoll_event> events(MAX_EVENTS);
	int 							num_events;
	t_ongoing						ongoing;

	while (!g_stop)
	{
        num_events = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        if (num_events == -1)
            std::cerr << "Error in epoll_wait: " << strerror(errno) << std::endl;
        for (int i = 0; i < num_events; ++i)
		{
			if (ongoing.requests.count(events[i].data.fd))
				handle_client(config, epoll_fd, events[i], ongoing);
			else if (ongoing.cgis.count(events[i].data.fd))
				handle_cgi_client(epoll_fd, events[i], ongoing);
			else
			{
				int port = match_socket(server_sockets, events[i].data.fd);
				if (port == -1)
					std::cerr << "Error : unable to match request" << std::endl;
				else
					register_client(epoll_fd, server_sockets.at(port), ongoing.requests, port);
			}
 	  	}
	}
	for (int i = 0; i < num_events; ++i)
		close(events[i].data.fd);
}

int server_init(const std::vector<ServerConfig> &config)
{
	std::map<int, int>	server_sockets;

	// Init epoll
	int epoll_fd = epoll_create(1);
	if (epoll_fd == -1)
	{
		std::cerr << "Error : " << strerror(errno) << std::endl;
		return 1;
	}

	// Init sockets
	try
	{
		for (std::vector<ServerConfig>::const_iterator it = config.begin(); it != config.end(); it++)
			bind_server_socket(server_sockets, it->getPort(), epoll_fd);
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
		return 1;
	}

	// Allow CTRL + C to exit
	std::signal(SIGINT, &handle_signal);

	server_main_loop(epoll_fd, config, server_sockets);

	// Cleanup
	for (std::map<int, int>::iterator it = server_sockets.begin(); it != server_sockets.end(); it++)
		close(it->second);
	close(epoll_fd);

    return 0;
}
