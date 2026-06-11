#include "../includes/server.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
        return 1;
    }

    server srv("", "");
    if (srv.init_server(argv) != 0) {
        std::cerr << "Invalid arguments: port/password\n";
        return 1;
    }

    std::cout << "Server initialized. Port: " << srv.getPort()
              << ", Password: " << srv.getPassword() << "\n";
    srv.run();
    return 0;
}
