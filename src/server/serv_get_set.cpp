#include "server.hpp"

const std::string &server::getPort() const
{
    return this->port;
}

void server::setPort(const std::string &port)
{
    this->port = port;
}

void server::setPassword(const std::string &password)
{
    this->password = password;
}

const std::string &server::getPassword() const
{
    return this->password;
}
