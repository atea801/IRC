/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PtrVec.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 16:04:02 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/19 16:17:20 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>

template <typename T>
class PtrVec {
private:
    std::vector<T *> _elements;
public:
    void add(T &element){
        this->_elements.push_back(&element);
    }

    void remove(T &element){
        typename std::vector<T*>::iterator it;
        it = std::find(_items.begin(), _items.end(), &e);
        this->_elements.erase(it);
    }

    void isInVec(T &element){
        typename std::vector<T*>::iterator it;
        it = std::find(_items.begin(), _items.end(), &e);
        if (it != this->_elements.end())
            return (true);
        else
            return (false);
    }
};