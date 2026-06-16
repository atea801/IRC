/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 15:02:38 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/16 15:06:12 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <iostream>

//a implementer en prenant en compte les variables membres
Channel::Channel(void)
{
    std::cout << "Channel default constructor called" << std::endl;
}

//a implementer en prenant en compte les variables membres
Channel::Channel(const Channel& copy)
{
    std::cout << "Channel copy constructor called" << std::endl;
    *this = copy;
}

//a implementer en prenant en compte les variables membres
Channel::~Channel(void)
{
    std::cout << "Channel destructor called" << std::endl;
}

//a implementer en prenant en compte les variables membres
Channel& Channel::operator=(const Channel& copy)
{
    std::cout << "Channel assignment operator called" << std::endl;
    if (this != &copy) {
        // copy data members here
    }
    return (*this);
}