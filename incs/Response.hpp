#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "AHTTP.hpp"
#include "ServerConfig.hpp"

#include <sstream>
#include <iostream>

class Response: public AHTTP
{
    private :
        std::map<int,std::string> _error_codes;
    public:
        std::map<int,std::string>   getErrorCodes() const ;
        std::string		            getRaw(const int& status) const;
        int                         http_error_template(std::string &response, int error,const ServerConfig & config);
        std::string                 itos(int integer);
        void                        initializeErrorCodes(void);
		void						clear();
};

#endif
