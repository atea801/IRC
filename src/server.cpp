#include "../includes/server.hpp"

server::server (std::string port, std::string password)
    : port (port), password (password)
{
}

server::~server () {}

server::server (const server &copy) { *this = copy; }

server &
server::operator= (const server &other)
{
  if (this != &other)
    {
      this->port = other.port;
      this->password = other.password;
    }
  return *this;
}


/**
 * @brief fonction de parsing du format du port
 * 
 * @param port_cstr 
 * @return int 
 */
int
server::check_port (char *port_cstr)
{
  if (!port_cstr)
    return -1;
  std::string port_str = port_cstr;
  if (port_str.empty () || port_str.size () > 5)
    return -1;
  for (long unsigned i = 0; i < port_str.size (); i++)
    {
      unsigned char c = port_str[i];
      if (!std::isdigit (c))
        return -1;
    }
  long p = std::strtol (port_str.c_str (), NULL, 10);
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
int
server::init_server (char **av)
{
  if (!av || !av[1] || !av[2])
    return -1;
  if (check_port (av[1]) != 0)
    return -1;
  setPort (av[1]);
  setPassword (av[2]);
  return 0;
}

/**
 * @brief cree la socket du serveur et initialise les options necessaire
 * comme ipv4, tcp etc
 * puis bind cette socket au port et la met en mode listen pour attendre les actions a venir
 * @return int (-1 erreur)
 */
int
server::create_socket ()
{
  server_fd = socket (AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
    return (-1);
  int opt = 1;
  setsockopt (server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt));
  sockaddr_in addr;
  std::memset (&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons (std::atoi (port.c_str ()));
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind (server_fd, (sockaddr *)&addr, sizeof (addr)) < 0)
    return (-1);
  if (listen (server_fd, SOMAXCONN) < 0)
    return (-1);
  pollfd listen_p;
  listen_p.fd = server_fd;
  listen_p.events = POLLIN;
  listen_p.revents = 0;
  fds.push_back (listen_p);
  return 0;
}

/**
 * @brief main de la classe server qui permet de cree socket 
 * de gerer les actions des clients et de cree de nouveau clients. 
 * 
 * @return int 
 */
int
server::run ()
{
  if (create_socket () < 0)
    return (-1);
  while (true)
    {
      if (!fds.empty ())
        {
          int ready = poll (&fds[0], static_cast<int> (fds.size ()), -1);
          if (fcntl (server_fd, F_SETFL, O_NONBLOCK) < 0)
            return (-1);
          if (ready < 0)
            {
              perror ("poll");
              return -1;
            }
          for (size_t i = 0; i < fds.size () && ready > 0; i++)
            {
              if (fds[i].revents == 0)
                continue;
              if (fds[i].revents & POLLIN)
                {
                  if (fds[i].fd == server_fd)
                    {
                      if (accept_new_client () < 0)
                        return (-1);
                      continue;
                    }
                  int status = client_actions (i);
                  if (status < 0)
                    return (-1);
                  if (status > 0)
                    {
                      close (fds[i].fd);
                      std::cerr << "Client closed: " << fds[i].fd << "\n";
                      fds.erase (fds.begin () + i);
                      --i;
                      continue;
                    }
                }
              if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
                {
                  close (fds[i].fd);
                  fds.erase (fds.begin () + i);
                  --i;
                }
            }
        }
    }
}

/**
 * @brief appel la fonction accept qui s occupe de cree
 * un nouvel utilisateur
 * @return int (-1 erreur)
 */
int
server::accept_new_client ()
{
  sockaddr_in cli_addr;
  socklen_t cli_len = sizeof (cli_addr);
  int client_fd = accept (server_fd, (sockaddr *)&cli_addr, &cli_len);
  if (client_fd < 0)
    perror ("accept");
  else if (fcntl (client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
      perror ("fctnl");
      close (client_fd);
      return (-1);
    }
  else
    {
      std::cerr << "New client: " << client_fd << "\n";
      pollfd client_p;
      client_p.fd = client_fd;
      client_p.events = POLLIN;
      client_p.revents = 0;
      fds.push_back (client_p);
    }
  return (0);
}

/**
 * @brief lance les fonction send ou recv celon le besoin utilisateur
 * send pour envoyer une donne recv pour recevoir
 * 
 * @param i 
 * @return int (-1 erreur)
 */
int
server::client_actions (size_t i)
{
  char buf[4096];
  ssize_t n = recv (fds[i].fd, buf, sizeof (buf), 0);
  if (n < 0)
    {
      perror ("recv");
      return (-1);
    }
  if (n == 0)
    return (1);
  ssize_t sent = 0;
  while (sent < n)
    {
      ssize_t ret = send (fds[i].fd, buf + sent, n - sent, 0);
      if (ret < 0)
        {
          perror ("send");
          return (-1);
        }
      sent += ret;
    }
  return (0);
}

const std::string &
server::getPort () const
{
  return this->port;
}

void
server::setPort (const std::string &port)
{
  this->port = port;
}

void
server::setPassword (const std::string &password)
{
  this->password = password;
}

const std::string &
server::getPassword () const
{
  return this->password;
}