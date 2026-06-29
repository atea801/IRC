#include "Server.hpp"

void Server::exec_flow(Message &msg, Client &c)
{
    std::string cmd = msg.get_command();
    for (size_t i = 0; i < cmd.size(); i++)
        cmd[i] = toupper(cmd[i]);
    if (cmd.empty())
        return;

    if (c.getStatus() != REGISTERED)
    {
        if (cmd != "PASS" && cmd != "NICK" && cmd != "USER")
        {
            send_reply_error(c, ERR_NOTREGISTERED, "You have not registered");
            return;
        }
    }

    if (cmd == "PASS")
        handle_pass(msg, c);
    else if (cmd == "NICK")
        handle_nick(msg, c);
    else if (cmd == "USER")
        handle_user(msg, c);
    else if (cmd == "QUIT")
        c.setStatus(QUIT);
    else if (cmd == "PRIVMSG")
        handle_privmsg(msg, c);
    else
        send_reply_error(c, ERR_UNKNOWNCOMMAND, cmd, "Unknown command");

    if (c.getBoolPass() && c.getBoolUser() && c.getBoolNick())
        c.setStatus(REGISTERED);
    std::cout << "STATUS :" << c.getStatus() << '\n';
}

void Server::handle_nick(Message &msg, Client &c)
{
    IrcError error = msg.parsing_nick();
    if (error != IRC_OK)
    {
        if (error == ERR_NONICKNAMEGIVEN)
            send_reply_error(c, error, "No nickname given");
        else if (error == ERR_ERRONEUSNICKNAME)
            send_reply_error(c, error, msg.get_args()[0], "Erroneus nickname");
        return;
    }
    // check prealable
    std::vector<std::string> args = msg.get_args();
    for (size_t i = 0; i < vec_clients.size(); i++)
    {
        if (&vec_clients[i] != &c && vec_clients[i].getNickname() == args[0])
        {
            error = ERR_NICKNAMEINUSE;
            send_reply_error(c, error, msg.get_args()[0], "Nickname is already in use");
            return;
        }
    }
    // execution finale
    c.setNickname(args[0]);
    c.setBoolNick(true);
}

void Server::handle_user(Message &msg, Client &c)
{
    // --- DEBUG : voir comment la ligne a été découpée ---
    std::vector<std::string> args = msg.get_args();
    // std::cerr << "args.size() = " << args.size() << std::endl;
    // for (size_t i = 0; i < args.size(); ++i)
    //     std::cerr << "  args[" << i << "] = [" << args[i] << "]" << std::endl;
    // ----------------------------------------------------

	if (c.getStatus() == REGISTERED)
    {
        send_reply_error(c, ERR_ALREADYREGISTERED, "You may not reregister");
        return;
    }

    IrcError error = msg.parsing_user();
    if (error != IRC_OK)
    {
        if (error == ERR_NEEDMOREPARAMS)
            send_reply_error(c, error, msg.get_command(), "Not enough parameters");
        return;
    }
    c.setUsername(args[0]);
    c.setRealname(args[3]);
    c.setBoolUser(true);
}

void Server::handle_pass(Message &msg, Client &c)
{
	if (c.getStatus() == REGISTERED)
    {
        send_reply_error(c, ERR_ALREADYREGISTERED, "You may not reregister");
        return;
    }

    IrcError error = msg.parsing_pass();
    if (error != IRC_OK)
    {
        if (error == ERR_NEEDMOREPARAMS)
            send_reply_error(c, error, msg.get_command(), "Not enough parameters");
        return;
    }
    const std::vector<std::string> args = msg.get_args();
    if (args[0] != this->password)
    {
        error = ERR_PASSWDMISMATCH;
        send_reply_error(c, error, "Password incorrect");
        return;
    }
    c.setBoolPass(true);
}

void Server::handle_privmsg(Message &msg, Client &c)
{
    // IrcError error = msg.parsing_privmsg();
    // if (error != IRC_OK)
    // {

    // }
    const std::vector<std::string> args = msg.get_args();
    int dest = find_dest(args[0]);
    std::cout << "ARGS SIZE = " << args.size() << std::endl;
    for (size_t i = 0; i < args.size(); i++)
        std::cout << "args[" << i << "] = [" << args[i] << "]" << std::endl;
    std::string msg_to_send = ":" + c.getNickname() + " PRIVMSG " + args[0] + " :" + args[1] + "\r\n";
    send(dest, msg_to_send.c_str(), msg_to_send.size(), 0);
}

int Server::find_dest(std::string dest)
{
    for (size_t i = 0; i < vec_clients.size(); i++)
    {
        if (vec_clients[i].getNickname() == dest)
            return vec_clients[i].getFdClient();
    }
    return -1;
}
