/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel_member_funcs.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:17:53 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/18 18:10:28 by bkaras-g         ###   ########.fr       */
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


void    Channel::removeMember(Client &c)
{
    std::vector<Client *>::const_iterator it;
    it = std::find(this->getMembers().begin(), this->getMembers().end(), &c);
    this->_members.erase(it);
}
//std::find
//swap et pop_back

