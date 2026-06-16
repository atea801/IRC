/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 14:57:26 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/16 15:49:43 by bkaras-g         ###   ########.fr       */
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
    std::string             _name;
    std::vector<Client *>   _members;
    std::vector<Client *>   _chanOps;
    std::vector<Client *>   _invited;
    std::string             _topic;
    bool                    _topic_restricted;  // mode +t : only ops can set topic
    bool                    _is_invite_only;    // mode +i
    bool                    _has_a_password;    // mode +k
    std::string             _password;
    bool                    _has_a_user_limit;  // mode +l
    size_t                  _user_limit;

public:
    // ---------- Constructors / Destructor ----------

    Channel(void);
    Channel(const Channel& copy);
    ~Channel(void);

    // ---------- Overloading Operators Methods -------

    Channel& operator=(const Channel& copy);

    // ---------- Getter and Setter Methods ------------

    const std::string&              getName() const;
    const std::vector<Client *>&    getMembers() const;
    const std::vector<Client *>&    getChanOps() const;
    const std::vector<Client *>&    getInvited() const;
    const std::string&              getTopic() const;
    bool                            isTopicRestricted() const;
    bool                            isInviteOnly() const;
    bool                            hasPassword() const;
    const std::string&              getPassword() const;
    bool                            hasUserLimit() const;
    size_t                          getUserLimit() const;

    void    setName(const std::string& name);
    void    setTopic(const std::string& topic);
    void    setTopicRestricted(bool restricted);
    void    setInviteOnly(bool inviteOnly);
    void    setPassword(const std::string& password);
    void    removePassword();
    void    setUserLimit(size_t limit);
    void    removeUserLimit();

    // ---------- Member Methods -----------------------
    // addMember / removeMember / addOperator / isMember / invite ...

};

#endif // CHANNEL_HPP