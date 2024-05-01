#include "Cookie.hpp"

Cookie::Cookie(/* args */)
{
}

Cookie::~Cookie()
{
}

Cookie::Cookie(const Cookie & src)
{
    *this = src;
}

const   Cookie & Cookie::operator=(const Cookie &src)
{
    if (this != &src)
    {
        _id = src._id;
        _name = src._name;
        _parameter = src._parameter;
        _date = src._date;
    }
    return *this;
}


void    Cookie::Handle_Cookie(Request & request)
{
    (void)request;
}

//Setter

void    Cookie::setId(std::string id)
{
    this->_id = id;
}

void    Cookie::setName(std::string name)
{
    this->_name = name;
}

void    Cookie::setParameter(std::map<std::string, std::string> parameter)
{
    this->_parameter = parameter;
}

void    Cookie::setDate(std::string date)
{
    this->_date = date;
}

//Getter

const   std::string Cookie::getName( void ) const
{
    return this->_name;
}

const   std::string Cookie::getId( void ) const
{
    return this->_id;
}

const   std::map<std::string, std::string> Cookie::getParamater( void )const
{
    return this->_parameter;
}

const   std::string Cookie::getDate( void ) const 
{
    return this->_date;
}
