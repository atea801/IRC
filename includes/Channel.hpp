/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 14:57:26 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/16 14:58:33 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>

class Channel {
private:
    // ---------- Private Data members -------------------------
public:
    // ---------- Constructors / Destructor ----------

    Channel(void);
    Channel(const Channel& copy);
    ~Channel(void);

    // ---------- Overloading Operators Methods -------

    Channel& operator=(const Channel& copy);

    // ---------- Getter and Setter Methods ------------

    // ---------- Member Methods -----------------------

};

#endif // CHANNEL_HPP