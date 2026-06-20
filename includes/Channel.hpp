/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: komorebi <komorebi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 14:57:26 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/20 15:18:30 by komorebi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <algorithm>
#include "Client.hpp"
#include "PtrVec.hpp"

class Channel {
private:
    // ---------- Private Data members -------------------------
    std::string             _name;
    PtrVec<Client>          _members;
    PtrVec<Client>          _chanOps;
    PtrVec<Client>          _invited;
    std::string             _topic;
    bool                    _topic_restricted;  // mode +t : seuls les chanOps peuvent changer le topic
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
    // certains getter return par valeur et pas par reference car ils 
    // ne pesent pas lourd donc peuvent etre copies. Quand on return par ref
    //imperatif de return un const pour eviter que la variable retournee soit
    //modifiee par des petits hackers en herbe
    const std::string&              getName() const;
    const std::vector<Client *>&         getMembers() const;
    const std::vector<Client *>&         getChanOps() const;
    const std::vector<Client *>&         getInvited() const;
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
    void    addMember(Client &c);
    size_t  NumberOfMembers();
    void    removeMember(Client &c);
    bool    isMember(Client &c);
    void    addOperator(Client &c);
    void    removeOperator(Client &c);
    bool    isOperator(Client &c);
    void    invite(Client &c);
    bool    isInvited(Client &c);
};

#endif // CHANNEL_HPP