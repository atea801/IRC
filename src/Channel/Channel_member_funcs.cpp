/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel_member_funcs.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:17:53 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/19 12:18:33 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

void    Channel::addMember(Client &c)
{
    this->_members.push_back(&c);
}

size_t  Channel::NumberOfMembers()
{
    return (this->getMembers().size());
}

/*
Supprime le client `c` du channel. Le parsing a déjà été effectué donc pas de
check à faire ici (si le client existe par exemple)
*/
void    Channel::removeMember(Client &c)
{
    std::vector<Client *>::const_iterator it;
    it = std::find(this->getMembers().begin(), this->getMembers().end(), &c);
    this->_members.erase(it);
}

/*
Vérifie si le client `c` est membre du channel.
*/
bool    Channel::isMember(Client &c)
{
    std::vector<Client *>::const_iterator it;
    it = std::find(this->getMembers().begin(), this->getMembers().end(), &c);
    if (it == this->getMembers().end())
        return (false);
    else
        return (true);
}

