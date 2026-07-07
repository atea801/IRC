#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int connect_to_server(const std::string &ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(fd);
        return -1;
    }

    std::cout << "Connecté au serveur " << ip << ":" << port << std::endl;
    return fd;
}

void send_line(int fd, const std::string &line)
{
    std::string msg = line + "\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
}

void bot_register(int fd, std::string pass)
{
    send_line(fd, "NICK bot");
    send_line(fd, "USER bot * 0 bot");
    send_line(fd, "PASS " + pass);
}

void join_channel(int fd)
{
    send_line(fd, "JOIN #bot");
}

bool parse_privmsg(const std::string &line, std::string &target, std::string &text)
{
    size_t privmsg_pos = line.find("PRIVMSG");
    if (privmsg_pos == std::string::npos)
        return false;

    size_t after_cmd = privmsg_pos + 7; // "PRIVMSG" = 7 caracteres
    size_t space = line.find(' ', after_cmd + 1);
    if (space == std::string::npos)
        return false;
    target = line.substr(after_cmd + 1, space - after_cmd - 1);

    size_t colon = line.find(':', space);
    if (colon == std::string::npos)
        return false;
    text = line.substr(colon + 1);
    return true;
}

std::string get_current_time()
{
    std::time_t now = std::time(NULL);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <ip> <port> <password>" << std::endl;
        return 1;
    }

    int fd = connect_to_server(argv[1], std::atoi(argv[2]));
    if (fd < 0)
        return 1;
    bot_register(fd, argv[3]);
    join_channel(fd);
    while (true)
    {
        char buf[4096];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0)
        {
            if (n < 0)
                perror("recv");
            return (1);
        }
        std::string target, text;
        if (parse_privmsg(std::string(buf, n), target, text))
        {
            if (target == "#bot" && text == "!time\r\n")
            {
                std::string reply = "PRIVMSG #bot :Il est " + get_current_time();
                send_line(fd, reply);
            }
            else if (target == "#bot" && text == "!help\r\n")
            {
                std::string reply = "PRIVMSG #bot :Commandes disponibles: !time !help";
                send_line(fd, reply);
            }
        }
    }
    return 0;
}