#ifndef COOKIE_HPP
# define COOKIE_HPP

# include "Request.hpp"

class Cookie
{
private:
    std::string _id;
    std::string _name;
    std::map<std::string, std::string> _parameter;
    std::string                        _date;


public:
    Cookie(/* args */);
    Cookie(const Cookie & src);
    ~Cookie();
    const Cookie &  operator=(const Cookie & src);
    void    Handle_Cookie(Request & request);

    //Setter
    void    setId(std::string id);
    void    setName(std::string name);
    void    setParameter(std::map<std::string, std::string> parameter);
    void    setDate(std::string date);
    //Getter
    const std::string                           getId( void ) const;
    const std::map<std::string, std::string>    getParamater( void ) const;
    const std::string                           getName( void ) const;
    const std::string                           getDate( void ) const;


};


#endif
