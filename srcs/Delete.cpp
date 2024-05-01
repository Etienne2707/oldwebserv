#include "Delete.hpp"


//Need to check return code for file
int            html_delete_request(std::string &response)
{
    response += "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "    <meta charset=\"UTF-8\">\n"
    "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "    <style>\n"
    "        /* Style de l'en-tête */\n"
    "        header {\n"
    "            background-color: green; /* green */\n"
    "            color: white;\n"
    "            text-align: center;\n"
    "            padding: 10px;\n"
    "        }\n"
    "    </style>\n"
    "</head>\n"
    "<header>\n"
    "    <h1>File has been deleted"
    "</h1>\n"
    "</header>\n"
    "</html>\n";
    return (200);
}

int            handle_DELETE(t_client& client_data, std::string& body)
{
	Request			request = client_data.request;
	ServerConfig	config = request.getServerConfig();
    std::string 	path = request.getPathTranslated();

    if (request.is_directory() == true && path.find_last_of('/') != path.size() - 1)
    	return 409;
    else if (request.is_directory() == true || path.find_last_of('/') == path.size() - 1)
    {
        if (access(path.c_str(),W_OK) != 0)
            return 403;
        if (std::remove(path.c_str()) == 0)
        {
            html_delete_request(body);
            return 200;
        }
        else
            return 500;
    }
    if (std::remove(path.c_str()) != 0)
        return 403;
    html_delete_request(body);
    return 200;

}