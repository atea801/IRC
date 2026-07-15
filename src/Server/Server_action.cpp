#include "Server.hpp"

/*
Note sur l'ajour de `MSG_NOSIGNAL` à `send()` : Sous Linux, écrire dans une socket dont le pair
(l'autre extrémité) s'est déconnecté déclenche le signal `SIGPIPE`, dont l'action par défaut
est de terminer votre processus. Un client qui se déconnecte au mauvais moment tuerait 
ainsi le serveur tout entier. `MSG_NOSIGNAL` permet d'ignorer `SIGPIPE`
*/
void Server::flush_client(int fd)
{
    Client *c = find_client(fd);
    if (!c || c->getOut().empty())
        return;
    const std::string &out = c->getOut();
    ssize_t n = send(fd, out.c_str(), out.size(), MSG_NOSIGNAL);
    if (n > 0)
        c->consumeOut(static_cast<size_t>(n));
    // si n <= 0 : on ne touche pas au buffer, on réessaiera au prochain POLLOUT
}

/**
 * @brief fonction de parsing du format du port
 *
 * @param port_cstr
 * @return int
 */
int Server::check_port(char *port_cstr)
{
    if (!port_cstr)
        return -1;
    std::string port_str = port_cstr;
    if (port_str.empty() || port_str.size() > 5)
        return -1;
    for (long unsigned i = 0; i < port_str.size(); i++)
    {
        unsigned char c = port_str[i];
        if (!std::isdigit(c))
            return -1;
    }
    long p = std::strtol(port_str.c_str(), NULL, 10);
    if (p < 1 || p > 65535)
        return -1;
    return 0;
}

/**
 * @brief initialise la varible priver pot et password et parse
 * port pour que tout soit au bon format et dans la classe
 *
 * @param av
 * @return int (-1 erreur)
 */
int Server::init_server(char **av)
{
    if (!av || !av[1] || !av[2])
        return -1;
    if (check_port(av[1]) != 0)
        return -1;
    setPort(av[1]);
    setPassword(av[2]);
    return 0;
}

/**
 * @brief cree la socket du serveur et initialise les options necessaire
 * comme ipv4, tcp etc
 * puis bind cette socket au port et la met en mode listen pour attendre les
 * actions a venir
 * @return int (-1 erreur)
 */
int Server::create_socket()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        return (-1);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::atoi(port.c_str()));
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
        return (-1);
    if (listen(server_fd, SOMAXCONN) < 0)
        return (-1);
    pollfd listen_p;
    listen_p.fd = server_fd;
    listen_p.events = POLLIN;
    listen_p.revents = 0;
    fds.push_back(listen_p);

    pollfd stdin_p;
    stdin_p.fd = STDIN_FILENO;
    stdin_p.events = POLLIN;
    stdin_p.revents = 0;
    fds.push_back(stdin_p);

    return 0;
}

/**
 * @brief appel la fonction accept qui s occupe de cree
 * un nouvel utilisateur
 * @return int (-1 erreur)
 */
int Server::accept_new_client()
{
    sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int client_fd = accept(server_fd, (sockaddr *)&cli_addr, &cli_len);
    // si value reurn accept < 0 => erreur
    if (client_fd < 0)
        perror("accept");
    // si le retour du flux continu est < 0 => ERREUR
    else if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fctnl");
        close(client_fd);
        return (-1);
    }
    else
    {
        std::cerr << "New client: " << client_fd << "\n";
        pollfd client_p;
        client_p.fd = client_fd;
        client_p.events = POLLIN;
        client_p.revents = 0;
        fds.push_back(client_p);
        Client c(client_p.fd);
        vec_clients.erase(client_fd);
        vec_clients.insert(std::make_pair(client_fd, c));
        Client *newc = find_client(client_fd);
        if (newc)
            sendConnectBanner(*newc);
    }
    return (0);
}

/**
 * @brief localise le client auquel appartient le buffer qui s apprete
 * a etre parser/executer.
 *
 * @param fds
 * @param i
 * @return Client*
 */
Client *Server::find_client(int fd)
{
    std::map<int, Client>::iterator it = vec_clients.find(fd);
    if (it == vec_clients.end())
        return NULL;
    return &it->second;
}

/**
 * @brief lance les fonction send ou recv celon le besoin utilisateur
 * send pour envoyer une donne recv pour recevoir
 *
 * @param i
 * @return int (-1 erreur)
 */
int Server::client_actions(size_t i)
{
    // 1. recv dépose dans buf temporaire
    char buf[4096];
    ssize_t n = recv(fds[i].fd, buf, sizeof(buf), 0);
    if (n <= 0)
    {
        if (n < 0)
            perror("recv");
        return (1);
    }
    // 2. trouve le bon client par fd
    Client *c = find_client(fds[i].fd);
    if (!c)
    {
        perror("find client");
        return (-1);
    }
    // 3. ajoute les donnees recues au buffer du client
    c->setBuffer(c->getBuffer() + std::string(buf, n));

    // 4. boucle clean du buffer
    while (c->getBuffer().find("\r\n") != std::string::npos)
    {
        Message message;
        // a. extraire la premiere commande + nettoyer
        message.extract_and_clean(*c);
        // b. on parse la lign extraire en focniton de commande parisng 2
        // c. execute
        const std::vector<std::string> args = message.get_args();
        this->exec_flow(message, *c);
        if (c->getStatus() == QUIT)
            return (2);
    }
    return (0);
}

/**
 * @brief main de la classe server qui permet de cree socket
 * de gerer les actions des clients et de cree de nouveau clients.
 *
 * @return int
 */
int Server::run()
{
    if (create_socket() < 0)
        return (-1);
    while (!g_stop) //check si un signal demande l'arrêt du serveur
    {
        if (fds.empty())
            continue;

        // (A) AVANT poll : on demande POLLOUT seulement si le client a
        //     quelque chose à envoyer. Sinon poll nous réveillerait en boucle.
        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].fd == server_fd || fds[i].fd == STDIN_FILENO)
                continue;
            Client *c = find_client(fds[i].fd);
			// on dit a poll surveille cd fd a la fois pour la lecture ET pour l'ecriture
            if (c && !c->getOut().empty())
                fds[i].events = POLLIN | POLLOUT;
            else
                fds[i].events = POLLIN;
        }

        int ready = poll(&fds[0], static_cast<int>(fds.size()), -1);
        if (ready < 0)
        {
            if (errno == EINTR) //check si un signal a été détecté.
                continue; // on revient au début du while
            perror("poll");
            return -1;
        }

        for (size_t i = 0; i < fds.size() && ready > 0; i++)
        {
            if (fds[i].revents == 0)
                continue;

            if (fds[i].fd == STDIN_FILENO && (fds[i].revents & POLLIN))
            {
                char buf[64];
                ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
                if (n == 0)
                {
                    g_stop = 1;
                    break;
                }
                continue;
            }
            
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == server_fd)
                {
                    if (accept_new_client() < 0)
                        return (-1);
                    continue;
                }
                int status = client_actions(i);
                if (status < 0)
                    return (-1);
                if (status == 1) //déconnection non prévue (ex kill PID du client ) --> besoin de générer un QUIT
                {
                    Message msg;
                    msg.fill_cmd_and_args(4, "QUIT\r\n");
                    Client *c = find_client(fds[i].fd);
                    handle_quit(msg, *c);
                }
                if (status > 0)
                {
                    remove_client(fds[i].fd);
                    close(fds[i].fd);
                    std::cerr << "Client closed: " << fds[i].fd << "\n";
                    fds.erase(fds.begin() + i);
                    --i;
                    continue;
                }
            }

            // (B) APRÈS lecture : si le socket est prêt en écriture, on vide le buffer
            if (fds[i].revents & POLLOUT)
                flush_client(fds[i].fd);

            if (fds[i].fd != server_fd && fds[i].fd != STDIN_FILENO && 
                (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)))
            {
                Message msg;
                msg.fill_cmd_and_args(4, "QUIT\r\n");
                Client *c = find_client(fds[i].fd);
                if (c)
                    handle_quit(msg, *c);
                remove_client(fds[i].fd);
                close(fds[i].fd);
                fds.erase(fds.begin() + i);
                --i;
                continue;
            }
        }
    }
    //si un signal a été détecté, fermeture des sockets. Les vecteurs et maps s'auto-nettoient.
    for (size_t i = 0; i < fds.size(); i++)
    {
        if (fds[i].fd != STDIN_FILENO)
            close(fds[i].fd);
    }
    fds.clear();
    vec_clients.clear();
    channels.clear();
    return (0);
}