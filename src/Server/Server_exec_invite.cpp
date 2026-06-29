/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_exec_invite.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 15:56:29 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/29 16:07:43 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handle_invite(Message &msg, Client &c)
{
    /*IrcError error = msg.parsing_invite();
    if (error != IRC_OK)
    {
        if(error == ERR_NEEDMOREPARAMS)
            send_reply_error(c, error, " MODE :Not enough parameters");
        return;
    }
    */

    Channel *chan = findChannelByName(msg.get_args()[1]);
    if (!chan)
    {
        // ERR_NOSUCHCHANNEL (403)
        // send_reply_error "<client> <channel> :No such channel"
        send_reply_error(c, ERR_NOSUCHCHANNEL, msg.get_args()[1], "No such channel");
        return;
    }
    if (!chan->isMember(c))
    {
        // ERR_NOTONCHANNEL (442) "<client> <channel> :You're not on that channel"
        send_reply_error(c, ERR_NOTONCHANNEL, chan->getName(), "You're not on that channel");
        return;
    }
    Client *target = findClientByNickname(msg.get_args()[0]);
    if (target == NULL || chan->isMember(*target))
    {
        // ERR_USERONCHANNEL (443) 
        // "<client> <nick> <channel> :is already on channel"
        send_reply_error(c, ERR_USERONCHANNEL, target->getNickname(), chan->getName(), "is already on channel");
    }
}