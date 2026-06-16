/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 14:57:26 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/16 15:38:31 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <Client.hpp>

class Channel {
private:
    // ---------- Private Data members -------------------------
    std::vector<Client *> _clientsList;

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