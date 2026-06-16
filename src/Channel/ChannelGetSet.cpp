/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelGetSet.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 16:11:40 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/16 18:00:49 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

const std::string&  Channel::getName() const
{
    return (this->_name);
}

void Channel::setName(const std::string& name)
{
    this->_name = name;
}

const std::vector<Client *>&    Channel::getMembers() const
{
    return (this->_members);
}

const std::vector<Client *>&    Channel::getChanOps() const
{
    return (this->_chanOps);
}

const std::vector<Client *>&    Channel::getInvited() const
{
    return (this->_invited);
}

const std::string&  Channel::getTopic() const
{
    return (this->_topic);
}

bool    Channel::isTopicRestricted() const
{
    return (this->_topic_restricted);
}

bool    Channel::isInviteOnly() const
{
    return (this->_is_invite_only);
}

bool    Channel::hasPassword() const
{
    return (this->_has_a_password);
}

const std::string&  Channel::getPassword() const
{
    return (this->_password);
}

bool    Channel::hasUserLimit() const
{
    return (this->_has_a_user_limit);
}

size_t  Channel::getUserLimit() const
{
    return (this->_user_limit);
}

void    Channel::setTopic(const std::string& topic)
{
    this->_topic = topic;
}

void    Channel::setTopicRestricted(bool restricted)
{
    this->_topic_restricted = restricted;
}

void    Channel::setInviteOnly(bool inviteOnly)
{
    this->_is_invite_only = inviteOnly;
}

/*
@param password: le password a mettre en place. Il a deja ete 
parse donc il est conforme aux exigences du protocole IRC
*/
void    Channel::setPassword(const std::string& password)
{
    this->_has_a_password = true;
    this->_password = password;
}

void    Channel::removePassword()
{
    this->_has_a_password = false;
    this->_password = "";
}
