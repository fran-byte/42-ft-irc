#include "Server.hpp"

/**
 * @brief Handles INVITE command for inviting users to channels
 * @param c Client sending the command
 * @param a Command arguments [nickname, channel]
 */
void Server::cmdINVITE(Client &c, const std::vector<std::string> &a)
{
    if (a.size() < 2)
        return sendTo(c, ":server NOTICE " + c.nick + " :INVITE <nick> <#chan>\r\n");

    const std::string &nick = a[0];
    const std::string &chan = a[1];

    /* Check if channel exists */
    std::map<std::string, Channel>::iterator it = channels.find(chan);
    if (it == channels.end())
        return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");

    Channel &ch = it->second;

    /* Verify inviter is channel member */
    if (!ch.isMember(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not on that channel\r\n");

    /* Check invite permissions for invite-only channels */
    if (ch.inviteOnly && !ch.isOp(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not channel operator\r\n");

    /* Find target user by nickname */
    int tfd = findFdByNick(nick);
    if (tfd < 0)
        return sendTo(c, ":server NOTICE " + c.nick + " :No such nick\r\n");

    /* Add user to invite list */
    ch.invite(tfd);

    /* Notify invited user */
    std::map<int, Client>::iterator itc = clients.find(tfd);
    if (itc != clients.end())
        sendTo(itc->second, ":" + c.nick + " INVITE " + nick + " :" + chan + "\r\n");

    /* Confirm invitation to inviter */
    sendTo(c, ":server NOTICE " + c.nick + " :Invited " + nick + " to " + chan + "\r\n");
}

/**
 * @brief Handles KICK command for removing users from channels
 * @param c Client sending the command
 * @param a Command arguments [channel, nickname, reason(optional)]
 */
void Server::cmdKICK(Client &c, const std::vector<std::string> &a)
{
    if (a.size() < 2)
        return sendTo(c, ":server NOTICE " + c.nick + " :KICK <#chan> <nick> [:reason]\r\n");

    const std::string &chan = a[0];
    const std::string &nick = a[1];
    std::string reason;
    if (a.size() >= 3)
        reason = a[2];
    else
        reason = "Kicked";

    /* Check if channel exists */
    std::map<std::string, Channel>::iterator it = channels.find(chan);
    if (it == channels.end())
        return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");

    Channel &ch = it->second;

    /* Verify kicker has operator privileges */
    if (!ch.isOp(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not channel operator\r\n");

    /* Find and verify target user is in channel */
    int tfd = findFdByNick(nick);
    if (tfd < 0 || !ch.isMember(tfd))
        return sendTo(c, ":server NOTICE " + c.nick + " :User not in channel\r\n");

    /* Broadcast kick message to all channel members */
    std::string msg = ":" + c.nick + " KICK " + chan + " " + nick + " :" + reason + "\r\n";
    broadcastToChannel(ch, -1, msg);

    /* Remove user from channel */
    ch.removeMember(tfd);
    std::map<int, Client>::iterator itc = clients.find(tfd);
    if (itc != clients.end())
        itc->second.channels.erase(chan);

    /* Clean up empty channels */
    if (ch.members.empty())
        channels.erase(it);
}