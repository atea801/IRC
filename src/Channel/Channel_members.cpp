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
    this->_members.add(c);
}

size_t  Channel::NumberOfMembers()
{
    return (this->_members.size());
}

/*
Supprime le client `c` du channel. Le parsing a déjà été effectué donc pas de
check à faire ici (si le client existe par exemple)
*/
void    Channel::removeMember(Client &c)
{
    this->_members.remove(c);
}

/*
Vérifie si le client `c` est membre du channel.
*/
bool    Channel::isMember(Client &c)
{
    return (this->_members.has(c));
}

