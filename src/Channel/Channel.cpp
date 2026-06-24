/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 15:02:38 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/23 17:53:25 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <iostream>

// a implementer en prenant en compte les variables membres
Channel::Channel(void)
    : _name(""), _members(), _chanOps(), _invited(), _topic(""), _topic_restricted(false), _is_invite_only(false),
      _has_a_password(false), _password(""), _has_a_user_limit(false), _user_limit(0)
{
    std::cout << "Channel default constructor called" << std::endl;
}

// a implementer en prenant en compte les variables membres
Channel::Channel(const Channel &copy)
{
    std::cout << "Channel copy constructor called" << std::endl;
    *this = copy;
}

// a implementer en prenant en compte les variables membres
Channel::~Channel(void)
{
    std::cout << "Channel destructor called" << std::endl;
}

// a implementer en prenant en compte les variables membres
Channel &Channel::operator=(const Channel &copy)
{
    std::cout << "Channel assignment operator called" << std::endl;
    if (this != &copy)
    {
        this->_name = copy._name;
        this->_members = copy._members;
        this->_chanOps = copy._chanOps;
        this->_invited = copy._invited;
        this->_topic = copy._topic;
        this->_topic_restricted = copy._topic_restricted;
        this->_is_invite_only = copy._is_invite_only;
        this->_has_a_password = copy._has_a_password;
        this->_password = copy._password;
        this->_has_a_user_limit = copy._has_a_user_limit;
        this->_user_limit = copy._user_limit;
    }
    return (*this);
}