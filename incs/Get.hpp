#ifndef Get_HPP
#define Get_HPP

#include "process_request.hpp"

const std::string red = "\033[0;31m";
const std::string green = "\033[1;32m";
const std::string yellow = "\033[1;33m";
const std::string cyan = "\033[0;36m";
const std::string magenta = "\033[0;35m";
const std::string reset = "\033[0m";

const std::string html_start = " <!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>Liens vers d'autres pages</title>\n"
"</head>\n"
"<body>\n"
"    <header>\n"
"        <h1>Liens vers d'autres pages</h1>\n"
"    </header>\n"
"\n"
"    <main>\n"
"        <p>Choisissez une des pages suivantes :</p>\n"
"        <ul>\n";

const std::string html_end = "</ul>\n"
"    </main>\n"
"\n"
"    <footer>\n"
"        <p>Ce site a été créé par [Votre Nom]</p>\n"
"    </footer>\n"
"</body>\n"
"</html>";

#endif