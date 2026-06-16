#pragma once

#include <arpa/inet.h>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "Client.hpp"
#include "message.hpp"

class server
{
  private:
    std::vector<pollfd> fds;
    std::vector<Client> vec_clients;
    std::string port;
    std::string password;
    int server_fd;

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
    int accept_new_client();
    int create_socket();
    int client_actions(size_t i);
    Client *find_client(std::vector<pollfd> fds, size_t i);
    const std::string &getPort() const;
    const std::string &getPassword() const;
};
