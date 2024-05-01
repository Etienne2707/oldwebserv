#ifndef PROCESS_REQUEST_HPP
# define PROCESS_REQUEST_HPP

# include <string>
# include <vector>
# include <sys/socket.h>
# include <sys/epoll.h>
# include <iostream>
# include <sys/types.h>
# include <algorithm>
# include <cstring>
# include <map>
# include "AHTTP.hpp"
# include "Response.hpp"
# include "Cookie.hpp"
# include "Request.hpp"

const std::string red = "\033[0;31m";
const std::string green = "\033[1;32m";
const std::string yellow = "\033[1;33m";
const std::string cyan = "\033[0;36m";
const std::string magenta = "\033[0;35m";
const std::string reset = "\033[0m";

typedef struct	s_client
{
	int				port;
	std::string		ip;
	Request			request;
	Response		response;
	std::string		raw_response;
}	t_client;

typedef struct	s_ongoing
{
	std::map<int, t_client>	requests;
	std::map<int, int>		cgis;
}	t_ongoing;

void		sendBasicErrorResponse(int error_code, t_client& client_data);
int     	send_response_to_client(t_client& client_data, std::string &body, int error);
void    	Content_Type(Response& get_response,Request request);

int 		process_request(const std::vector<ServerConfig> &config_list, const std::string& raw_input, t_ongoing& ongoing, int packet_fd, const int& epoll_fd);

// cgi
std::string	uint_to_str(unsigned int n);
std::string	int_to_str(int n);
std::string	method_to_str(int method);
std::string	find_server_name(const Request& request, const ServerConfig& config);
void		fill_env(char*** env, const std::string& var, const std::string& value);
void		clear_env(char** env);

void		process_cgi(t_client& client, const std::string& script, int packet_fd, const int& epoll_fd, std::map<int, int>& ongoing_cgi);
bool		handle_cgi(t_client& client, const std::vector<ServerLocation>& locations, int packet_fd, const int& epoll_fd, std::map<int, int>& ongoing_cgi);

char**		create_env(const t_client& client, const std::string& script);

// get
void		get_inside_file(std::string &result, std::ifstream &file);
int 		location_auto_index_redirectory(ServerLocation location,std::string path,  std::string &response);
int			location_index_redirectory(ServerLocation location,std::string path,  std::string &response);
void    	get_fields_response(std::map<std::string, std::string> it, std::string& response, Response & get_response, Request & request);
int         handle_GET(t_client& client_data, ServerLocation& location, std::string& body);

// post
int    		handle_POST(t_client& client_data, const ServerLocation& matched_location);

// delete
int         handle_DELETE(t_client& client_data, std::string& body);


#endif
