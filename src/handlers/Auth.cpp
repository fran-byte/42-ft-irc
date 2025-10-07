#include "Server.hpp"

/**
 * @brief Attempts to complete client registration when all conditions met
 * @param c Client to check and potentially register
 */
void Server::tryRegister(Client &c)
{
    if (!c.registered && c.passOk && !c.nick.empty() && !c.user.empty())
    {
        c.registered = true;
        sendTo(c, ":server 001 " + c.nick + " :Welcome to ft_irc\r\n");
    }
}

/**
 * @brief Handles PASS command for client password authentication
 * @param c Client sending the command
 * @param a Command arguments containing password
 */
void Server::cmdPASS(Client &c, const std::vector<std::string> &a)
{
    if (a.empty())
        return sendTo(c, ":server NOTICE * :PASS needs parameter\r\n");
    if (c.registered)
        return;
    c.passOk = (a[0] == password);
    if (!c.passOk)
        sendTo(c, ":server NOTICE * :Bad password\r\n");
    tryRegister(c);
}

/**
 * @brief Handles NICK command for setting client nickname
 * @param c Client sending the command
 * @param a Command arguments containing nickname
 */
void Server::cmdNICK(Client &c, const std::vector<std::string> &a)
{
    if (a.empty())
        return sendTo(c, ":server NOTICE * :NICK needs parameter\r\n");
    /* TODO: Validate duplicates and format */
    c.nick = a[0];
    tryRegister(c);
}

/**
 * @brief Handles USER command for setting client username and realname
 * @param c Client sending the command
 * @param a Command arguments containing user info
 */
void Server::cmdUSER(Client &c, const std::vector<std::string> &a)
{
    if (a.size() < 4)
        return sendTo(c, ":server NOTICE * :USER needs 4 params\r\n");
    c.user = a[0];
    c.realname = a.back();
    tryRegister(c);
}