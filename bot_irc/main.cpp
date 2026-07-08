#include "bot_irc.hpp"

// =========================================================================================
// =========================================================================================
//
// MISE EN PLACE DE BOT
//
// =========================================================================================
// =========================================================================================

/**
 * @brief Ouvre une socket TCP et se connecte au serveur IRC
 * @param ip 127.0.0.1
 * @param port
 * @return int
 */
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

/**
 * @brief Envoi une ligne IRC complete sur la socket, rajout automatiquement "\r\n"
 * @param fd
 * @param line
 */
void send_line(int fd, const std::string &line)
{
    std::string msg = line + "\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
}

/**
 * @brief Enregistre le bot aupres du serveur
 * @param fd
 * @param pass
 */
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

/**
 * @brief Extrait la cible et le texte d'une ligne privmsg, pour recuperer (channel ou nick) +
 * le message
 * @param line ligne IRC complete sans le "\r\n"
 * @param target cible du message (ex: #bot ou nickname)
 * @param text contenu du message apres le ":"
 * @return true
 * @return false
 */
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

// =========================================================================================
// =========================================================================================
//
// FONCTIONNALITE DU BOT
//
// =========================================================================================
// =========================================================================================
std::vector<std::string> forbidden_words()
{
    std::vector<std::string> w;
    w.push_back("idiot");
    w.push_back("stupid");
    w.push_back("noob");
    w.push_back("connard");
    return w;
}

/**
 * @brief Fonctionalite !TIME qui renvoie l'heure
 *
 * @return std::string
 */
std::string get_current_time()
{
    std::time_t now = std::time(NULL);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

/**
 * @brief Convertit une chaine en minuscule
 * @param s
 * @return std::string
 */
std::string to_lower(const std::string &s)
{
    std::string r = s;
    for (size_t i = 0; i < r.size(); ++i)
        r[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(r[i])));
    return r;
}

/**
 * @brief Indiaue si un texte contient au moins un mot interdit
 * @param text
 * @param words
 * @return true
 * @return false
 */
bool has_forbidden_word(const std::string &text, const std::vector<std::string> &words)
{
    std::string low = to_lower(text);
    for (size_t i = 0; i < words.size(); ++i)
    {
        if (!words[i].empty() && low.find(to_lower(words[i])) != std::string::npos)
            return true;
    }
    return false;
}

/**
 * @brief Extrait le nick de l'emetteur
 *
 * Le prefixe a la forme ":nick!user@host ...". Le nick se situe entre le ':'
 * initial et le '!'.
 * @param line
 * @param nick
 * @return true
 * @return false
 */
bool parse_sender_nick(const std::string &line, std::string &nick)
{
    if (line.empty() || line[0] != ':')
        return false;
    size_t excl = line.find('!');
    size_t space = line.find(' ');
    if (excl == std::string::npos || space == std::string::npos || excl > space)
        return false;
    nick = line.substr(1, excl - 1);
    return true;
}
/**
 * @brief Envoi la commande KICK au serveur
 * @param fd
 * @param channel
 * @param nick
 */
void kick_user(int fd, const std::string &channel, const std::string &nick, const std::string &reason)
{
    send_line(fd, "KICK " + channel + " " + nick + " :" + reason);
}

// =========================================================================================
// =========================================================================================
//
// POINT D'ENTREE DU BOT
//
// =========================================================================================
// =========================================================================================

/**
 * @brief Decide a chaque message recu si un user spamme
 *
 * - combien de message on ete send recement => msg_count
 * - a quel moment ca a commence => window_start
 * - check le nombre d'avertissement => warning
 *
 * @param nick
 * @param states map qui permet d'associer le nick a son etat
 * @return SpamAction
 */
SpamAction register_message(const std::string &nick, std::map<std::string, UserState> &states)
{
    std::time_t now = std::time(NULL);
    UserState &st = states[nick];

    // check si la fenetre de comptage est expirer -> reset
    //  tout premier message ou plus de SPAM_WINDOW s ce sont ecoule
    if (st.window_start == 0 || now - st.window_start >= SPAM_WINDOW)
    {
        st.window_start = now;
        st.msg_count = 0;
    }
    st.msg_count++;
    // check si trop de message
    if (st.msg_count > SPAM_THRESHOLD)
    {
        // on repart pour la prochaine rafale
        st.msg_count = 0;
        st.window_start = now;
        st.warnings++;
        if (st.warnings >= 2)
            return SPAM_KICK;
        return SPAM_WARN;
    }
    return SPAM_NONE;
}

/**
 * @brief Traite une ligne complete recue du serveur
 *
 * Parse la ligne comme un PRIVMSG. Si l'emetteur (autre que le bot) ecrit un
 * mot interdit dans un channel, il est kicke. Sinon, les commandes !time et
 * !help sont traitees.
 * @param fd
 * @param line
 * @param words
 */
void handle_line(int fd, const std::string &line, const std::vector<std::string> &words,
                 std::map<std::string, UserState> &states)
{
    std::string target, text;
    if (!parse_privmsg(line, target, text))
        return;

    std::string sender;
    parse_sender_nick(line, sender);

    if (sender == "bot" || sender.empty())
        return;

    if (!target.empty() && (target[0] == '#' || target[0] == '&'))
    {
        // 1) Gros mot -> kick immediat
        if (has_forbidden_word(text, words))
        {
            kick_user(fd, target, sender, "Mot interdit detecte");
            states.erase(sender);
            return;
        }
        // 2) Anti-spam
        SpamAction act = register_message(sender, states);
        if (act == SPAM_WARN)
        {
            send_line(fd, "PRIVMSG " + target + " :" + sender + " arrete de spam ! (avertissement 1/2)");
            return;
        }
        if (act == SPAM_KICK)
        {
            kick_user(fd, target, sender, "Spam repete");
            states.erase(sender);
            return;
        }
    }

    if (target == "#bot" && text == "!time")
        send_line(fd, "PRIVMSG #bot :Il est " + get_current_time());
    else if (target == "#bot" && text == "!help")
        send_line(fd, "PRIVMSG #bot :Commandes disponibles: !time !help");
}

/**
 * @brief Se connecte au serveur, s'enregistre, rejoint #bot, puis boucle : lit la
 * socket, reconstitue les lignes completes terminees par "\r\n" (gestion des
 * paquets partiels ou groupes) et delegue chacune a handle_line().
 *
 * @param argc
 * @param argv
 * @return int
 */
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

    std::vector<std::string> words = forbidden_words();
    std::map<std::string, UserState> states;
    std::string buffer;

    while (true)
    {
        char buf[4096];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0)
        {
            if (n < 0)
                perror("recv");
            return 1;
        }

        buffer += std::string(buf, n);
        size_t pos;
        while ((pos = buffer.find("\r\n")) != std::string::npos)
        {
            std::string line = buffer.substr(0, pos);
            buffer.erase(0, pos + 2);
            handle_line(fd, line, words, states);
        }
    }
    return 0;
}