#include "Server.hpp"

/**
 * @brief Handles JOIN command for client joining channels
 * @param c Client sending the command
 * @param a Command arguments [channel, key(optional)]
 */
void Server::cmdJOIN(Client &c, const std::vector<std::string> &a)
{
    if (a.empty())
    {
        std::string nickname;
        if (c.nick.empty())
            nickname = "*";
        else
            nickname = c.nick;
        return sendTo(c, ":server NOTICE " + nickname + " :JOIN needs channel\r\n");
    }
    std::string chan = a[0];
    /* Validate channel name starts with # */
    if (chan.empty() || chan[0] != '#')
        return sendTo(c, ":server NOTICE " + c.nick + " :Bad channel name\r\n");

    /* Extract channel key if provided */
    std::string *keyOpt = 0;
    std::string keyTmp;
    if (a.size() >= 2)
    {
        keyTmp = a[1];
        keyOpt = &keyTmp;
    }

    /* Get or create channel */
    Channel &ch = channels[chan];
    if (ch.name.empty())
        ch.name = chan;
    bool newlyCreated = ch.members.empty();

    /* Check join permissions */
    if (!ch.canJoin(c.fd, keyOpt))
        return sendTo(c, ":server NOTICE " + c.nick + " :Cannot join channel\r\n");

    /* Add client to channel */
    ch.addMember(c.fd);
    c.channels.insert(chan);
    /* Make first joiner channel operator */
    if (newlyCreated)
        ch.addOp(c.fd);

    /* Notify all channel members */
    std::string nickname;
    if (c.nick.empty())
        nickname = "*";
    else
        nickname = c.nick;
    std::string joinMsg = ":" + nickname + " JOIN " + chan + "\r\n";
    broadcastToChannel(ch, -1, joinMsg); /* Send to everyone including joiner */
}

/**
 * @brief Handles PART command for client leaving channels
 * @param c Client sending the command
 * @param a Command arguments [channel]
 */
void Server::cmdPART(Client &c, const std::vector<std::string> &a)
{
    if (a.empty())
    {
        std::string nickname;
        if (c.nick.empty())
            nickname = "*";
        else
            nickname = c.nick;
        return sendTo(c, ":server NOTICE " + nickname + " :PART needs channel\r\n");
    }
    std::string chan = a[0];
    /* Check if channel exists */
    std::map<std::string, Channel>::iterator it = channels.find(chan);
    if (it == channels.end())
        return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");

    Channel &ch = it->second;
    /* Verify client is channel member */
    if (!ch.isMember(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not on that channel\r\n");

    /* Notify channel and remove client */
    std::string partMsg = ":" + c.nick + " PART " + chan + "\r\n";
    broadcastToChannel(ch, c.fd, partMsg); /* Send to others */
    sendTo(c, partMsg);                    /* Send to self */

    /* Clean up channel membership */
    ch.removeMember(c.fd);
    c.channels.erase(chan);
    /* Delete empty channels */
    if (ch.members.empty())
        channels.erase(it);
}