/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel_ChanOps.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 00:00:00 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/19 14:05:52 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

void Channel::addOperator(Client &c)
{
    this->_chanOps.add(c);
}

void Channel::removeOperator(Client &c)
{
    this->_chanOps.remove(c);
}

bool Channel::isOperator(Client &c)
{
    return (this->_chanOps.has(c));
}

void Channel::invite(Client &c)
{
    this->_invited.add(c);
}

bool Channel::isInvited(Client &c)
{
    return (this->_invited.has(c));
}
