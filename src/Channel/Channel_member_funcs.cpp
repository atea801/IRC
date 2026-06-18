/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel_member_funcs.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:17:53 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/18 17:03:11 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

void    Channel::addMember(Client &c)
{
    this->_members.push_back(&c);
}

// void    Channel::removeMember(Client &c)
// {
//     for (size_t i = 0; i < this->)
// }

