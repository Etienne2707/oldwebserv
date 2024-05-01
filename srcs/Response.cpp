#include "Response.hpp"

void    Response::initializeErrorCodes(void)
{
    _error_codes[100] = "Continue";
    _error_codes[101] = "Switching Protocols";
    _error_codes[102] = "Processing";
    _error_codes[103] = "Early Hints";
    _error_codes[200] = "OK";
    _error_codes[201] = "Created";
    _error_codes[202] = "Accepted";
    _error_codes[203] = "Non-Authoritative Information";
    _error_codes[204] = "No Content";
    _error_codes[205] = "Reset Content";
    _error_codes[206] = "Partial Content";
    _error_codes[207] = "Multi-Status";
    _error_codes[208] = "Already Reported";
    _error_codes[226] = "IM Used";
    _error_codes[300] = "Multiple Choices";
    _error_codes[301] = "Moved Permanently";
    _error_codes[302] = "Found";
    _error_codes[303] = "See Other";
    _error_codes[304] = "Not Modified";
    _error_codes[305] = "Use Proxy";
    _error_codes[306] = "Switch Proxy";
    _error_codes[307] = "Temporary Redirect";
    _error_codes[308] = "Permanent Redirect";
    _error_codes[400] = "Bad Request";
    _error_codes[401] = "Unauthorized";
    _error_codes[402] = "Payment Required";
    _error_codes[403] = "Forbidden";
    _error_codes[404] = "Not Found";
    _error_codes[405] = "Method Not Allowed";
    _error_codes[406] = "Not Acceptable";
    _error_codes[407] = "Proxy Authentication Required";
    _error_codes[408] = "Request Timeout";
    _error_codes[409] = "Conflict";
    _error_codes[410] = "Gone";
    _error_codes[411] = "Length Required";
    _error_codes[412] = "Precondition Failed";
    _error_codes[413] = "Payload Too Large";
    _error_codes[414] = "URI Too Long";
    _error_codes[415] = "Unsupported Media Type";
    _error_codes[416] = "Range Not Satisfiable";
    _error_codes[417] = "Expectation Failed";
    _error_codes[418] = "I'm a teapot";
    _error_codes[421] = "Misdirected Request";
    _error_codes[422] = "Unprocessable Content";
    _error_codes[423] = "Locked";
    _error_codes[424] = "Failed Dependency";
    _error_codes[425] = "Too Early";
    _error_codes[426] = "Upgrade Required";
    _error_codes[428] = "Precondition Required";
    _error_codes[429] = "Too Many Requests";
    _error_codes[431] = "Request Header Fields Too Large";
    _error_codes[451] = "Unavailable For Legal Reasons";
    _error_codes[500] = "Internal Server Error";
    _error_codes[501] = "Not Implemented";
    _error_codes[502] = "Bad Gateway";
    _error_codes[503] = "Service Unavailable";
    _error_codes[504] = "Gateway Timeout";
    _error_codes[505] = "HTTP Version Not Supported";
    _error_codes[506] = "Variant Also Negotiates";
    _error_codes[507] = "Insufficient Storage";
    _error_codes[508] = "Loop Detected";
    _error_codes[509] = "Starting Ticket Required";
    _error_codes[510] = "Not Extended";
    _error_codes[511] = "Network Authentication Required";
    _error_codes[604] = "Inaccessible Request";
    _error_codes[605] = "Not Allowed";
    _error_codes[644] = "Request Deleted or Modified";
}

std::map<int,std::string>   Response::getErrorCodes() const
{
    return (_error_codes);
}


std::string Response::itos(int integer)
{
    std::ostringstream convert;

    convert << integer;
    return convert.str();
}

int    Response::http_error_template(std::string &response, int error,  const ServerConfig & config)
{
    std::map<int, std::string> map_error = config.getErr_codes();
    std::map<int, std::string>::iterator it = map_error.begin();
    if (!map_error.empty())
    {        
        for (;it != map_error.end();it++)
        {
            if (it->first == error)
            {
                std::ifstream file(it->second.c_str());
                if (it->second.find_last_of("/") == it->second.size() -1)
                    break ;
                if (file.is_open())
                {
                    get_inside_file(response,file);
                    file.close();
                    return (error);
                }
                break ;
            }
        }
    }
    response += "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "    <meta charset=\"UTF-8\">\n"
    "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "    <style>\n"
    "        /* Style de l'en-tête */\n"
    "        header {\n"
    "            background-color: black ; /* NOIR */\n"
    "            color: white;\n"
    "            text-align: center;\n"
    "            padding: 10px;\n"
    "        }\n"
    "    </style>\n"
    "</head>\n"
    "<header>\n"
    "    <h1>Error ";
    response += itos(error) + " : ";
    response += _error_codes[error];
    response += "</h1>\n"
    "</header>\n"
    "</html>\n";
    return error;

}

std::string    Response::getRaw(const int& status) const
{
    std::string         raw;
    std::stringstream   ss;
    std::map<int, std::string> error_codes = getErrorCodes();

    ss << HTTP_VERSION << " " << status << " " << error_codes[status] << HTTP_ENDL;
    for (std::map<std::string, std::string>::const_iterator it = _fields.begin(); it != _fields.end(); it++)
        ss << it->first << ": " << it->second << HTTP_ENDL;
    ss << HTTP_ENDL << _body;
    return ss.str();
}

void			Response::clear(void)
{
		_fields.clear();
		_body.clear();
}

