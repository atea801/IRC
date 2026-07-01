/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PtrVec.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 16:04:02 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/23 18:03:36 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <algorithm>
#include <vector>

/* Cette classe template est un wrapper autour d'un vecteur de pointeurs.
Elle permet de stocker des pointeurs vers des objets de type T et fournit
des méthodes pour ajouter, supprimer et vérifier la présence d'éléments
 dans le vecteur. Elle est utilisée dans la classe Channel pour gérer
 les membres, les opérateurs et les invités du canal.
 Pas besoin d'implémenter des constructeurs ni de destructeur car tout est
 déjà pris en charge par `std::vector`
*/
template <typename T> class PtrVec
{
  private:
    std::vector<T *> _elements;

  public:
    /**
     * @brief Ajoute un élément au vecteur.
     * @param element L'élément à ajouter.
     */
    void add(T &element)
    {
        this->_elements.push_back(&element);
    }

    /**
     * @brief Supprime un élément du vecteur. Une vérification est effectuée
     * pour s'assurer que l'élément existe avant de le supprimer, bien que
     * cette vérification soit déjà effectuée auparavant.
     * @param element L'élément à supprimer.
     */
    void remove(T &element)
    {
        typename std::vector<T *>::iterator it;
        it = std::find(this->_elements.begin(), this->_elements.end(), &element);
        if (it != this->_elements.end())
            this->_elements.erase(it);
    }

    /**
     * @brief Vérifie si un élément est présent dans le vecteur.
     * @param element L'élément à vérifier.
     * @return true si l'élément est présent, false sinon.
     */
    bool has(T &element)
    {
        typename std::vector<T *>::const_iterator it;
        it = std::find(this->_elements.begin(), this->_elements.end(), &element);
        if (it != this->_elements.end())
            return (true);
        else
            return (false);
    }

    /**
     * @brief Retourne le nombre d'éléments dans le vecteur.
     * @return Le nombre d'éléments.
     */
    size_t size() const
    {
        return this->_elements.size();
    }

    /**
     * @brief Retourne une référence constante au vecteur d'éléments.
     * @return Une référence constante au vecteur d'éléments.
     */
    const std::vector<T *> &get() const
    {
        return this->_elements;
    }
};