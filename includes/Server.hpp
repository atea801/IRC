#pragma once

#include "Client.hpp"
#include "Message.hpp"
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

class Server
{
  private:
    std::vector<pollfd> fds;
    std::vector<Client> vec_clients;
    std::string port;
    std::string password;
    int server_fd;

  public:
    Server(std::string port, std::string password);
    ~Server();
    Server(const Server &copy);
    Server &operator=(const Server &other);
    int check_port(char *port);
    int init_server(char **av);
    void setPort(const std::string &port);
    void setPassword(const std::string &password);
    int run();
    int accept_new_client();
    int create_socket();
    int client_actions(size_t i);
    void execute(Message &msg, Client &c);
    void handle_nick(Message &msg, Client &c);
    void handle_user(Message &msg, Client &c);
    void handle_realname(Message &msg, Client &c);
    void handle_pass(Message &msg, Client &c);
    Client *find_client(std::vector<pollfd> fds, size_t i);
    const std::string &getPort() const;
    const std::string &getPassword() const;
};
