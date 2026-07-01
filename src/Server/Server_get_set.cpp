#include "Server.hpp"

const std::string &Server::getPort() const
{
    return this->port;
}

void Server::setPort(const std::string &port)
{
    this->port = port;
}

void Server::setPassword(const std::string &password)
{
    this->password = password;
}

const std::string &Server::getPassword() const
{
    return this->password;
}
