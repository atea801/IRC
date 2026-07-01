#include "Message.hpp"

Message::Message()
{
}

Message::~Message()
{
}

Message::Message(const Message &copy)
{
    *this = copy;
}

Message &Message::operator=(const Message &other)
{
    if (this != &other)
    {
        command = other.command;
        args = other.args;
    }
    return *this;
}

void Message::clear()
{
    command.clear();
    args.clear();
}