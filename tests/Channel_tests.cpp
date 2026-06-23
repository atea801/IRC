/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel_tests.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 17:43:50 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/23 18:11:06 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
** Tests unitaires pour la classe Channel (includes/Channel.hpp).
**
** Compilation (depuis la racine du projet) :
**   c++ -Wall -Wextra -g -std=c++98 -I./includes \
       tests/Channel_tests.cpp \
       src/Channel/Channel.cpp \
       src/Channel/Channel_members.cpp \
       src/Channel/Channel_ChanOps.cpp \
       src/Channel/Channel_get_set.cpp \
       src/Client/Client.cpp \
       src/Client/Client_get_set.cpp \
       -o channel_tests
   ./channel_tests
**
** N.B. : le constructeur par defaut de Channel n'initialise PAS les membres
** booleens / size_t (_topic_restricted, _is_invite_only, _has_a_password,
** _has_a_user_limit, _user_limit). Lire ces valeurs sur un Channel fraichement
** construit est un comportement indefini : les tests ci-dessous fixent donc
** toujours une valeur via un setter avant de la relire.
*/

#include "Channel.hpp"
#include "Client.hpp"
#include <iostream>
#include <string>

// ----------------------------------------------------------------------------
// Mini framework de test
// ----------------------------------------------------------------------------

static int g_checks_run = 0;
static int g_checks_failed = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        ++g_checks_run;                                                        \
        if (!(cond)) {                                                         \
            ++g_checks_failed;                                                 \
            std::cout << "  [FAIL] " << __FILE__ << ":" << __LINE__            \
                      << "  CHECK(" << #cond << ")" << std::endl;              \
        }                                                                      \
        else {                                                                 \
            std::cout << "  [SUCCESS] " << __FILE__ << ":" << __LINE__         \
                      << "  CHECK(" << #cond << ")" << std::endl;              \
        }                                                                      \
    } while (0)

#define CHECK_EQ(a, b)                                                         \
    do {                                                                       \
        ++g_checks_run;                                                        \
        if (!((a) == (b))) {                                                   \
            ++g_checks_failed;                                                 \
            std::cout << "  [FAIL] " << __FILE__ << ":" << __LINE__            \
                      << "  CHECK_EQ(" << #a << ", " << #b << ")"              \
                      << "  got '" << (a) << "' expected '" << (b) << "'"      \
                      << std::endl;                                            \
        }                                                                      \
        else {                                                                 \
            std::cout << "  [SUCCESS] " << __FILE__ << ":" << __LINE__         \
                      << "  CHECK_EQ(" << #a << ", " << #b << ")"              \
                      << "  got '" << (a) << "' expected '" << (b) << "'"      \
                      << std::endl;                                            \
        }                                                                      \
    } while (0)

static void section(const std::string& title)
{
    std::cout << "\n=== " << title << " ===" << std::endl;
}

// Petite fabrique de clients : chaque client a un fd unique (l'egalite de
// Client repose sur le fd, et PtrVec identifie par adresse).
static Client makeClient(int fd)
{
    return Client(fd);
}

// ----------------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------------

static void test_name()
{
    section("getName / setName");

    Channel chan;
    chan.setName("#general");
    CHECK_EQ(chan.getName(), std::string("#general"));

    chan.setName("#dev");
    CHECK_EQ(chan.getName(), std::string("#dev"));

    chan.setName("");
    CHECK_EQ(chan.getName(), std::string(""));
}

static void test_topic()
{
    section("getTopic / setTopic");

    Channel chan;
    chan.setTopic("Bienvenue !");
    CHECK_EQ(chan.getTopic(), std::string("Bienvenue !"));

    chan.setTopic("Nouveau sujet");
    CHECK_EQ(chan.getTopic(), std::string("Nouveau sujet"));

    chan.setTopic("");
    CHECK_EQ(chan.getTopic(), std::string(""));
}

static void test_topic_restricted()
{
    section("isTopicRestricted / setTopicRestricted (mode +t)");

    Channel chan;
    chan.setTopicRestricted(true);
    CHECK(chan.isTopicRestricted() == true);

    chan.setTopicRestricted(false);
    CHECK(chan.isTopicRestricted() == false);
}

static void test_invite_only()
{
    section("isInviteOnly / setInviteOnly (mode +i)");

    Channel chan;
    chan.setInviteOnly(true);
    CHECK(chan.isInviteOnly() == true);

    chan.setInviteOnly(false);
    CHECK(chan.isInviteOnly() == false);
}

static void test_password()
{
    section("hasPassword / getPassword / setPassword / removePassword (mode +k)");

    Channel chan;

    // setPassword active le flag et stocke le mot de passe.
    chan.setPassword("secret42");
    CHECK(chan.hasPassword() == true);
    CHECK_EQ(chan.getPassword(), std::string("secret42"));

    // Ecrasement par un autre mot de passe.
    chan.setPassword("autre");
    CHECK(chan.hasPassword() == true);
    CHECK_EQ(chan.getPassword(), std::string("autre"));

    // removePassword desactive le flag et vide le mot de passe.
    chan.removePassword();
    CHECK(chan.hasPassword() == false);
    CHECK_EQ(chan.getPassword(), std::string(""));
}

static void test_user_limit()
{
    section("hasUserLimit / getUserLimit / setUserLimit / removeUserLimit (mode +l)");

    Channel chan;

    chan.setUserLimit(10);
    CHECK(chan.hasUserLimit() == true);
    CHECK_EQ(chan.getUserLimit(), (size_t)10);

    chan.setUserLimit(1);
    CHECK(chan.hasUserLimit() == true);
    CHECK_EQ(chan.getUserLimit(), (size_t)1);

    chan.removeUserLimit();
    CHECK(chan.hasUserLimit() == false);
    CHECK_EQ(chan.getUserLimit(), (size_t)0);
}

static void test_members()
{
    section("addMember / NumberOfMembers / isMember / removeMember / getMembers");

    Channel chan;
    Client a = makeClient(3);
    Client b = makeClient(4);
    Client c = makeClient(5);

    CHECK_EQ(chan.NumberOfMembers(), (size_t)0);
    CHECK(chan.isMember(a) == false);

    chan.addMember(a);
    chan.addMember(b);
    CHECK_EQ(chan.NumberOfMembers(), (size_t)2);
    CHECK(chan.isMember(a) == true);
    CHECK(chan.isMember(b) == true);
    CHECK(chan.isMember(c) == false);

    // getMembers : le vecteur expose les memes adresses que celles ajoutees.
    const std::vector<Client *>& members = chan.getMembers();
    CHECK_EQ(members.size(), (size_t)2);
    CHECK(members[0] == &a);
    CHECK(members[1] == &b);

    // removeMember retire le bon membre, laisse les autres en place.
    chan.removeMember(a);
    CHECK_EQ(chan.NumberOfMembers(), (size_t)1);
    CHECK(chan.isMember(a) == false);
    CHECK(chan.isMember(b) == true);

    // Retirer un client absent ne change rien (PtrVec::remove est sur).
    chan.removeMember(c);
    CHECK_EQ(chan.NumberOfMembers(), (size_t)1);

    chan.removeMember(b);
    CHECK_EQ(chan.NumberOfMembers(), (size_t)0);
}

static void test_operators()
{
    section("addOperator / isOperator / removeOperator / getChanOps");

    Channel chan;
    Client op = makeClient(7);
    Client other = makeClient(8);

    CHECK(chan.isOperator(op) == false);

    chan.addOperator(op);
    CHECK(chan.isOperator(op) == true);
    CHECK(chan.isOperator(other) == false);
    CHECK_EQ(chan.getChanOps().size(), (size_t)1);
    CHECK(chan.getChanOps()[0] == &op);

    chan.removeOperator(op);
    CHECK(chan.isOperator(op) == false);
    CHECK_EQ(chan.getChanOps().size(), (size_t)0);

    // Les operateurs sont independants des membres.
    CHECK_EQ(chan.NumberOfMembers(), (size_t)0);
}

static void test_invited()
{
    section("invite / isInvited / getInvited");

    Channel chan;
    Client guest = makeClient(11);
    Client other = makeClient(12);

    CHECK(chan.isInvited(guest) == false);

    chan.invite(guest);
    CHECK(chan.isInvited(guest) == true);
    CHECK(chan.isInvited(other) == false);
    CHECK_EQ(chan.getInvited().size(), (size_t)1);
    CHECK(chan.getInvited()[0] == &guest);
}

static void test_lists_are_independent()
{
    section("Independance des listes (membres / operateurs / invites)");

    Channel chan;
    Client x = makeClient(20);

    chan.addMember(x);
    chan.addOperator(x);
    chan.invite(x);

    CHECK(chan.isMember(x) == true);
    CHECK(chan.isOperator(x) == true);
    CHECK(chan.isInvited(x) == true);

    // Retirer d'une liste n'affecte pas les autres.
    chan.removeMember(x);
    CHECK(chan.isMember(x) == false);
    CHECK(chan.isOperator(x) == true);
    CHECK(chan.isInvited(x) == true);

    chan.removeOperator(x);
    CHECK(chan.isMember(x) == false);
    CHECK(chan.isOperator(x) == false);
    CHECK(chan.isInvited(x) == true);
}

static void test_copy_and_assign()
{
    section("Constructeur de copie / operateur d'affectation");

    // Note : dans l'implementation actuelle, le constructeur de copie et
    // l'operateur= ne recopient AUCUNE donnee membre. Ce test verifie
    // simplement qu'ils s'executent sans crash (et documente ce comportement).
    Channel original;
    original.setName("#src");

    Channel copy(original);   // constructeur de copie
    Channel assigned;
    assigned = original;      // operateur d'affectation

    // Auto-affectation : ne doit pas crasher.
    assigned = assigned;

    CHECK(true); // si on arrive ici, aucun crash n'a eu lieu
}

// ----------------------------------------------------------------------------
// main
// ----------------------------------------------------------------------------

int main()
{
    std::cout << "########## Tests Channel ##########" << std::endl;

    test_name();
    test_topic();
    test_topic_restricted();
    test_invite_only();
    test_password();
    test_user_limit();
    test_members();
    test_operators();
    test_invited();
    test_lists_are_independent();
    test_copy_and_assign();

    std::cout << "\n###################################" << std::endl;
    std::cout << "Checks executes : " << g_checks_run << std::endl;
    std::cout << "Checks rates    : " << g_checks_failed << std::endl;
    if (g_checks_failed == 0)
        std::cout << "RESULTAT : TOUS LES TESTS PASSENT" << std::endl;
    else
        std::cout << "RESULTAT : " << g_checks_failed << " CHECK(S) EN ECHEC" << std::endl;

    return (g_checks_failed == 0 ? 0 : 1);
}