/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PtrVec.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: komorebi <komorebi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 16:04:02 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/20 15:06:45 by komorebi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>

template <typename T>
class PtrVec {
private:
    std::vector<T> _elements;
public:
    void add(T &element){
        this->_elements.push_back(&element);
    }

    void remove(T &element){
        typename std::vector<T>::iterator it;
        it = std::find(this->_elements.begin(), this->_elements.end(), &e);
        this->_elements.erase(it);
    }

    void has(T &element){
        typename std::vector<T>::iterator it;
        it = std::find(this->_elements.begin(), this->_elements.end(), &e);
        if (it != this->_elements.end())
            return (true);
        else
            return (false);
    }
    
    size_t size() const {
        return this->_elements.size(); }

    const std::vector<T>& get() const {
        return this->_elements; }
};