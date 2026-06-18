#include "Server.hpp"

Server::Server(std::string port, std::string password) : port(port), password(password)
{
}

Server::~Server()
{
}

Server::Server(const Server &copy)
{
    *this = copy;
}

Server &Server::operator=(const Server &other)
{
    if (this != &other)
    {
        this->port = other.port;
        this->password = other.password;
    }
    return *this;
}