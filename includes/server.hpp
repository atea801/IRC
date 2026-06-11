#pragma once

#include <string>
#include <sys/types.h>
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <poll.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

class server {
    private:
    std::string port;
    std::string password;
    public:
    server(std::string port, std::string password);
    ~server();
    server(const server &copy);
    server &operator=(const server &other);
    int check_port(char *port);
    int init_server(char **av);
    void setPort(const std::string &port);
    void setPassword(const std::string &password);
    int run();
    const std::string &getPort() const;
    const std::string &getPassword() const;
};