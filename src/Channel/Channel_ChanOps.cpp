/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel_ChanOps_funcs.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 00:00:00 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/19 12:23:13 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

void    Channel::addOperator(Client &c)
{
    this->_chanOps.push_back(&c);
}

void    Channel::removeOperator(Client &c)
{
    std::vector<Client *>::const_iterator it;
    it = std::find(this->getChanOps().begin(), this->getChanOps().end(), &c);
    this->_chanOps.erase(it);
}

bool    Channel::isOperator(Client &c)
{
    std::vector<Client *>::const_iterator it;
    it = std::find(this->getChanOps().begin(), this->getChanOps().end(), &c);
    if (it == this->getChanOps().end())
        return (false);
    else
        return (true);
}
