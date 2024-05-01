#include "process_request.hpp"

bool isMethodAllowed(const ServerLocation& matched_location,Response &response, const ServerConfig &config ,const int& method, int packet_fd)
{
	std::string	raw_response;
	std::string content;
	std::string allowed_methods;
    std::string method_names[] = {"GET", "POST", "DELETE"};

	if (method & matched_location.getHttp_methods())
		return (true);
	int methods[] = {GET, POST, DELETE};
    for (int i = 0; i < 3; i++)
	{
     	if (matched_location.getHttp_methods() & methods[i])
		{
         	allowed_methods.append(method_names[i] + ' ');
     	}
	}
	response.http_error_template(content,405,config);
	response.addField("Allowed", allowed_methods);
	response.addField("Content-Length",response.itos(content.size()));
	response.setBody(content);
	raw_response = response.getRaw(405);
	send(packet_fd, raw_response.c_str(), raw_response.length(), 0);
	return false;
}

void    print_matching(const ServerConfig& config, const std::vector<ServerLocation>& locations) //just for testing purposes
{
    std::cout << "\nPrinting matching config found" << std::endl;
    std::cout << "\tLocations of matching config : " << std::endl;
    for (std::vector<ServerLocation>::const_iterator it = config.getLocations().begin(); it != config.getLocations().end(); it++)
		std::cout << "\t\t" << (*it).getLocation() << std::endl;
	std::cout << "\tPort of matching config : " << config.getPort() << std::endl;
	std::cout << "\tServer names of matching config : " << std::endl;
	for (std::vector<std::string>::const_iterator it = config.getServer_names().begin(); it != config.getServer_names().end(); it++)
		std::cout << "\t\t" << *it << std::endl;
	std::cout << "\tLocations config that matches request : " << std::endl;
    for (std::vector<ServerLocation>::const_iterator it = locations.begin(); it != locations.end(); it++)
		std::cout << "\t\t" << (*it).getLocation() << std::endl;
}

void	handle_paths(Request& request, const std::vector<ServerLocation>& locations)
{
	std::string location = locations[0].getLocation();
	std::string	root = locations[0].getRoot();
	std::string path = request.getPath();

	path.erase(0, location.length());
	request.setPathInfo(path);
	request.setPathTranslated(locations[0].getRoot() + path);
}

bool handle_rewrite(std::vector<s_rewrite> rewrite, Request& request, Response & response)
{
	std::string path = "/" + request.getPathInfo();
	if (rewrite.empty())
		return false;
	
	for (std::vector<s_rewrite>::iterator it = rewrite.begin(); it != rewrite.end(); it++)
	{
		if (it->first.compare(path) == 0)
		{
			response.addField("Location",it->second);
			response.addField("Content-Length","0");
			return (true);
		}
	}
	return (false);
}

void	sendBasicErrorResponse(int error_code, int packet_fd)
{
	Response	response;
	std::string	raw_response;

	response.addField("Content-Length", "0");
	raw_response = response.getRaw(error_code);
	send(packet_fd, raw_response.c_str(), raw_response.length(), 0);
}

std::string itos(int integer)
{
    std::ostringstream convert;

    convert << integer;
    return convert.str();
}

int	get_multi_id(std::string & line, std::vector<Cookie> & client_cookie, Cookie client_id)
{
	std::size_t found;
	std::string tmp;
	(void)client_cookie;

	found = line.find(";");
	tmp = line.substr(0,found);
	std::cout << "TMP = " << tmp << std::endl;
	line.erase(0,tmp.size() + 2);
	std::cout << "LINE ERASE = " << line << std::endl;
	found = tmp.find("=");
	if (found == std::string::npos)
	{
		std::cout << "pas de egal" << std::endl;
		std::cout << found << std::endl;
		return 400;
	}
	client_id.setName(tmp.substr(0,found));
	std::cout << red << client_id.getName() << reset << std::endl;
	client_id.setId(tmp.substr(found + 1,tmp.size() - found));
	client_cookie.push_back(client_id);
	return 0;
}

int		get_client_cookie(std::vector<Cookie> & client_cookie, Request & request, std::map<std::string, std::string> & fields)
{
	std::map<std::string, std::string>::iterator	fit;
	std::string										fields_cookies;
	std::string										tmp;
	Cookie											client_id;
	int												error = 0;
	fields_cookies = fields["Cookie"];
	if (fields_cookies == "")
	{
		int id = rand() % (1000000 + 1);
		client_id.setId(itos(id));
		client_cookie.push_back(client_id);
		return 1;
	}
	while (fields_cookies != "" && error != 400)
	{
		error = get_multi_id(fields_cookies, client_cookie,client_id);
	}
	(void)client_cookie;
	(void)request;
	return 0;
	
}

void	prepare_parameter(Cookie cookie)
{
	std::map<std::string, std::string> parameter;



}

std::vector<Cookie>		get_matched_cookie(std::vector<Cookie> & cookie, Request & request)
{
	std::vector<Cookie>								client_cookie;
	std::vector<Cookie>::iterator					cit;
	std::vector<Cookie>::iterator					cit2;
	std::map<std::string, std::string>				fields = request.getFields();
	std::map<std::string, std::string>::iterator	fit;

	std::cout << "hello" << std::endl;
	get_client_cookie(client_cookie, request, fields);
	std::cout << client_cookie.begin()->getId() << std::endl;
	for (cit = client_cookie.begin(); cit != client_cookie.end(); cit++)
	{
		std::cout << cit->getId() << std::endl;
		for (cit2 = cookie.begin(); cit2 != cookie.end(); cit2++)
		{
			if (cit2->getId() == cit->getId())
			{
				cit = cit2;
				std::cout << green << "Id : " << cit->getId() << "Deja existant !" << reset << std::endl;
				break;
			}
				
		}
		if (cit2 == cookie.end())
		{
			int id = rand() % (1000000 + 1);
			std::cout << cyan << "Creation de l'id : " << id << " if no match" << reset << std::endl;
			cit->setId(itos(id));
			
		}
	}

	return (client_cookie);
}

int process_request(const std::vector<ServerConfig> &config_list, const std::string& raw_input, t_client& client_data,std::vector<Cookie> cookie, int packet_fd, const int& epoll_fd)
{

  	std::vector<ServerLocation> locations;
	ServerLocation				matched_location;
	std::vector<Cookie>			matched_cookie;
	Response 					response;
	std::string					raw_response;
	
	std::cout << "hello" << std::endl;
  	if (int error_code = client_data.request.addRaw(raw_input, locations, client_data.request, client_data.port, config_list))
	{
		client_data.request.clear();
		sendBasicErrorResponse(error_code, packet_fd);
		return (-1);
	}
	if (!client_data.request.isFull())
		return (0);
	matched_location = client_data.request.getServerLocation()[0];
	response.initializeErrorCodes();
	if (!isMethodAllowed(matched_location,response,client_data.request.getServerConfig(), client_data.request.getMethod(), packet_fd))
		return (-1); 
	handle_paths(client_data.request, client_data.request.getServerLocation());
	if (handle_rewrite(matched_location.getRewrite(),client_data.request,response) == true)
	{
		raw_response = response.getRaw(301);
		send(packet_fd,raw_response.c_str(), raw_response.length(), 0);
		return (0);
	}//print_matching(client_data.request.getServerConfig(), locations);
	matched_cookie = get_matched_cookie(cookie,client_data.request);
	if (!handle_cgi(client_data, locations, packet_fd, epoll_fd))
	{
		if (client_data.request.getMethod() == GET)
		{
			handle_GET(packet_fd, client_data.request , client_data.request.getServerConfig(),matched_location,response);
		}
		else if (client_data.request.getMethod() == DELETE)
		{
			handle_DELETE(packet_fd, client_data.request , client_data.request.getServerConfig(),response);
		}
		else if (client_data.request.getMethod() == POST)
		{
			if (int error_code = handle_POST(packet_fd, client_data.request,client_data.request.getServerConfig(),matched_location))
				sendBasicErrorResponse(error_code, packet_fd);
		}

	}
	client_data.request.clear();
	return (0);
}
