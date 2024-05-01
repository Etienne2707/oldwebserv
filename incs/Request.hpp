#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "AHTTP.hpp"
# include "ServerConfig.hpp"

# include <sstream>
# include <sys/types.h>
# include <sys/socket.h>

class Request : public AHTTP
{
	private:
		int			            			    _http_method;
		std::string	            			    _path;
        bool                   				    _empty;
        bool                    		 		_full;
		bool									_headerfields;
        std::string              		   		_fieldbuffer;
        ServerConfig                			_config;
        std::vector<ServerLocation>             _location;
		std::string								_path_info;
		std::string								_path_translated;
        std::string                             _query;
		unsigned int							_body_size;
		int										_chunked_length;
		bool									_req_line;

		bool	get_http_line(std::string& str, std::string& line) const;
		int		process_req_line(std::string& line);
		int		process_header_line(std::string& line);

	public:
        Request();
        Request(const Request& src);
        ~Request();
        const Request&                      operator=(const Request& src);
        bool                                isEmpty(void) const;
        bool                                isFull(void) const;
        int                                 addRaw(std::string str,std::vector<ServerLocation>& locations, Request& request, const int& port, const std::vector<ServerConfig>& config_list);
        void                                clear(void);

        void                                setServerConfig(const ServerConfig& config);
        void                                setServerLocations(const std::vector<ServerLocation>& location);
		void								setPathInfo(const std::string& path_info);
		void								setPathTranslated(const std::string& path_translated);

        const std::vector<ServerLocation>&              getServerLocation(void) const;
        const ServerConfig&                 			getServerConfig(void) const;
		const int&										getMethod(void) const;
		const std::string&								getPath(void) const;
		const std::string&								getPathInfo(void) const;
		const std::string&								getPathTranslated(void) const;
		const std::string&								getQuery(void) const;
        bool                                            is_directory(void) const ;
		int												fillHeaders(std::string& to_process);
		int												fillBody();
		bool											isSet(const std::string& header) const;
		int												checkHeaders(void);
		int												hexStringToInt(const std::string& hex_string) const;
		int												fillBodyChunked(void);
		bool											getReqline(void) const;

};

void	    						match_server(std::vector<ServerLocation>& locations, Request& request, const int& port, const std::vector<ServerConfig>& config_list);
void 								match_location(std::vector<ServerLocation>& locations, const Request& request);


#endif
