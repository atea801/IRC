/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 14:57:26 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/16 15:49:03 by bkaras-g         ###   ########.fr       */
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
    std::vector<Client *>   _members;
    std::vector<Client *>   _chanOps;
    std::string             _topic;
    bool                    _is_invite_only;
    bool                    _has_a_password;
    std::string             _password;
    bool                    _has_a_user_limit;
    size_t                  _user_limit;

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