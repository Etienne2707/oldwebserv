#include "Request.hpp"

Request::Request(void): _empty(true), _full(false), _headerfields(false), _body_size(0), _chunked_length(0), _req_line(false)
{
    _content_length = 0;
}

Request::~Request(void) {}

Request::Request(const Request& src)
{
    *this = src;
}

const Request&  Request::operator=(const Request& src)
{
    if (this != &src)
    {
        _http_method = src._http_method;
        _path =  src._path;
        _empty = src._empty;
        _full = src._full;
        _fieldbuffer = src._fieldbuffer;
        _fields = src._fields;
		_body = src._body;
		_content_length = src._content_length;
		_chunked_length = src._chunked_length;
		_body_size = src._body_size;
		_headerfields = src._headerfields;
		_location = src._location;
		_query = src._query;
		_path_info = src._path_info;
		_path_translated = src._path_translated;
		_config = src._config;
		_req_line = src._req_line;
    }
    return *this;
}

bool	Request::get_http_line(std::string& str, std::string& line) const
{
	std::string::size_type	start, end;

	if (str.empty())
		return false;
	start = 0;
	end = str.find(HTTP_ENDL);
	if (end == std::string::npos)
	{
		line = str.substr(start);
		str = "";
	}
	else
	{
		line = str.substr(start, end - start);
		str = str.substr(end + HTTP_ENDL.length());
	}
	return true;
}

int	Request::process_header_line(std::string& line)
{
	std::string::size_type	pos;
	std::string	str;

	pos = line.find(": ");
	if (pos == std::string::npos)
		return 400;
	str = line.substr(0, pos);
	line.erase(0, pos + 2);
	if (str == "Host")
	{
		pos = line.find(':');
		if (pos != std::string::npos)
			line.erase(pos);
		_host = line;
	}
    if (str == "Content-Length")
    {
        std::stringstream ss;
        ss << line;
        ss >> _content_length;
        if (ss.fail())
        {
            std::cout << "Error: stringstream parsing failed" << std::endl;
            return (500);
        }
    }
	if (str == "Transfer-Encoding")
		_transfer_encoding = line;
	if (str == "Content-Type")
		_content_type = line;
	addField(str, line);
	return 0;
}

bool    Request::is_directory( void ) const
{
    struct stat fileInfo;
    std::string path = getPathTranslated();

    if (stat(path.c_str(), &fileInfo) == 0)
	{
		if (S_ISDIR(fileInfo.st_mode))
		{
            return true;
		}
	}
    return false;
}




std::string get_remove_query(std::string & path)
{
    if (path.find('?') == std::string::npos)
        return "";
    std::string query = path.substr(path.find_last_of('?') + 1,path.size());
    if (query.size() == 0)
        return "";
    path.erase(path.find_last_of('?'),path.size());
    return query;
}

int	Request::process_req_line(std::string& line)
{
	std::string::size_type	pos;
	std::string	str;

	pos = line.find(" ");
	if (pos == std::string::npos){
		//std::cout << "process_req_line" << std::endl; //DEBUG PURPOSE
		return 400;
	}
		
	str = line.substr(0, pos);
	if (str == "GET")
		_http_method = GET;
	else if (str == "POST")
		_http_method = POST;
	else if (str == "DELETE")
		_http_method = DELETE;
	else
		return 501;
	line.erase(0, pos + 1);
	pos = line.find(" ");
	if (pos == std::string::npos){
		std::cout << "test2" << std::endl; //DEBUG PURPOSE
		return 400;
	}
	_path = line.substr(0, pos);
    _query = get_remove_query(_path);
	line.erase(0, pos + 1);
	if (line != HTTP_VERSION)
		return 505;
	_req_line = true;
	return 0;
}

bool		Request::isSet(const std::string& header) const
{
	std::map<std::string, std::string>::const_iterator it = _fields.find(header);
	if (it == _fields.end())
		return false;
	return true;
}

int			Request::checkHeaders(void)
{
	if (!isSet("Host") || \
	(isSet("Content-Length") && isSet("Transfer-Encoding"))){
		//std::cout << "checkheaders" << std::endl;
		return (400);
	}
	return (0);
}

int		Request::fillHeaders(std::string& to_process)
{
	size_t      endl_pos;
	std::string line;
	
	endl_pos = _fieldbuffer.find(HTTP_ENDL + HTTP_ENDL);
	if (endl_pos == std::string::npos)
		endl_pos = _fieldbuffer.rfind(HTTP_ENDL);
	else
		endl_pos += 2;
	if (endl_pos == std::string::npos && _fieldbuffer.size() <= 8000)
		return (0);
	else if (endl_pos == std::string::npos)
		return (414);
	if (endl_pos != _fieldbuffer.size() - 2)
	{
		to_process = _fieldbuffer.substr(0, endl_pos + 2);
		_fieldbuffer.erase(0, endl_pos + 2);
	}
	else
	{
		to_process = _fieldbuffer;
		_fieldbuffer.clear();
	}
	if (_empty)
	{
		get_http_line(to_process, line);
		if (int error = process_req_line(line))
		{
			return (error);
		}
		_empty = false;
	}
	while (get_http_line(to_process, line) && line != HTTP_ENDL && line != "")
	{
		if (int error = process_header_line(line)) {
			return (error);
		}
	}
	if (line == "")
	{
		_headerfields = 1;
		_fieldbuffer.append(to_process);
		_req_line = true;
		if (checkHeaders())
			return (400);
	}
	return (1);
}

int	Request::fillBody()
{
	if (_body.size() + _fieldbuffer.size() > static_cast<unsigned int>(_config.getBody_size()))
		return (413);
	_body += _fieldbuffer;
	_body_size += _fieldbuffer.length();
	_fieldbuffer.clear();
	return (0);
}

int		Request::hexStringToInt(const std::string& hex_string) const
{
	std::istringstream 	iss(hex_string);
	int					result = 0;

	iss >> std::hex >> result;
	if (iss.fail())
		return (-1);
	return (result);
}

int		Request::fillBodyChunked(void)
{
	size_t		endl_pos;
	std::string	hex_string;

	if (_body.size() + _fieldbuffer.size() > static_cast<unsigned int>(_config.getBody_size()))
		return (413);
	while (!_fieldbuffer.empty())
	{
		if (_chunked_length == 0)
		{
			endl_pos = _fieldbuffer.find(HTTP_ENDL);
			if (endl_pos == std::string::npos)
				return (0);
			hex_string = _fieldbuffer.substr(0, endl_pos);
			if (hex_string == "0" && _fieldbuffer.substr(0, HTTP_ENDL.size() * 2 + 1) == ("0" + HTTP_ENDL + HTTP_ENDL))
			{
				_full = true;
				return (0);
			}
			_chunked_length = hexStringToInt(hex_string);
			if (_chunked_length == -1)
				return (500);
			_fieldbuffer.erase(0, endl_pos + HTTP_ENDL.size());
		}
		if ((_chunked_length) < static_cast<int>(_fieldbuffer.size()))
		{
			_body += _fieldbuffer.substr(0, _chunked_length); 
			_fieldbuffer.erase(0, _chunked_length + HTTP_ENDL.size());
			_chunked_length = 0;
		}
		else if ((_chunked_length) > static_cast<int>(_fieldbuffer.size()))
		{
			_body += _fieldbuffer;
			_chunked_length -= _fieldbuffer.size();
			_fieldbuffer.clear();
		}
	}
	return (1);
}

int    Request::addRaw(std::string str, std::vector<ServerLocation>& locations, Request& request, const int& port, const std::vector<ServerConfig>& config_list)
{
    std::string to_process;
	int			error;

	_fieldbuffer.append(str);
	if (_headerfields == false)
	{
		error = fillHeaders(to_process);
		if (_headerfields)
			match_server(locations, request, port, config_list);
		if (error != 1)
			return (error);
	}
    if (_headerfields && _body_size < _content_length)
	{
		if (int error = fillBody())
			return (error);
	}
	else if (_headerfields && _transfer_encoding == "chunked")
	{
		error = fillBodyChunked();
		if (error != 1)
			return (error);
	}
	if (_headerfields && _body_size == _content_length && _transfer_encoding != "chunked")
		_full = true;
    return (0);
}

void    Request::clear(void)
{
	_body_size = 0;
	_chunked_length = 0;
	_content_length = 0;
	_transfer_encoding.clear();
	_body.clear();
	_path.clear();
    _fields.clear();
    _fieldbuffer.clear();
	_host.clear();
	_content_type.clear();
	_headerfields = false;
    _empty = true;
    _full = false;
	_req_line = false;
}

bool    Request::isFull(void) const
{
    return _full;
}

bool    Request::isEmpty(void) const
{
    return _empty;
}

const std::string&	Request::getQuery(void) const
{
	return _query;
}

const int&	Request::getMethod(void) const
{
	return _http_method;
}

const std::string&	Request::getPath(void) const
{
	return _path;
}

void                Request::setServerLocations(const std::vector<ServerLocation>& location)
{
    _location = location;
}

void                Request::setServerConfig(const ServerConfig& config)
{
    _config = config;
}

void                Request::setPathInfo(const std::string& path_info)
{
    _path_info = path_info;
}

void                Request::setPathTranslated(const std::string& path_translated)
{
    _path_translated = path_translated;
}

const std::vector<ServerLocation>&   Request::getServerLocation(void) const
{
    return _location;
}

const ServerConfig&     Request::getServerConfig(void) const
{
    return _config;
}

const std::string&      Request::getPathInfo(void) const
{
    return _path_info;
}

const std::string&      Request::getPathTranslated(void) const
{
    return _path_translated;
}

bool					Request::getReqline(void) const
{
	return _req_line;
}

void match_location(std::vector<ServerLocation>& locations, const Request& request)
{
	const std::vector<ServerLocation>& configLocations = request.getServerConfig().getLocations();
    size_t lastSlashPos;
	std::string path(request.getPath());
	if (!path.empty() && path[path.length() - 1] != '/')
	{
		lastSlashPos = path.find_last_of('/');
		path = path.substr(0, lastSlashPos + 1);
	}
    while (!path.empty())
    {
        for (std::vector<ServerLocation>::const_iterator it = configLocations.begin(); it != configLocations.end(); it ++)
        {
            if ((*it).getLocation() == path)
            {
                locations.push_back(*it);
                break;
            }
        }
        path.erase(path.end() - 1);
        path = path.substr(0, path.find_last_of('/') + 1);
    }
}

void match_server(std::vector<ServerLocation>& locations, Request& request, const int& port, const std::vector<ServerConfig>& config_list)
{
	for (std::vector<ServerConfig>::const_iterator it = config_list.begin(); it != config_list.end(); it++)
	{
		std::vector<std::string> server_names = it->getServer_names();
		if (port != it->getPort())
			continue;
		for (std::vector<std::string>::const_iterator it2 = server_names.begin(); it2 != server_names.end(); it2 ++)
		{
			if (request.getHost() == *it2)
			{
				request.setServerConfig(*it);
				match_location(locations, request);
				request.setServerLocations(locations);
				return ;
			}
		}
	}
	for (std::vector<ServerConfig>::const_iterator it = config_list.begin(); it != config_list.end(); it++)
	{
		if (port == it->getPort() && it->isDefault())
		{
			request.setServerConfig(*it);
			match_location(locations, request);
			request.setServerLocations(locations);
			return ;
		}
	}
}

