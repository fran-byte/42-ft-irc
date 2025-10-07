#include "Server.hpp"

/**
 * @brief Handles PRIVMSG command for sending messages to channels or users
 * @param c Client sending the command
 * @param a Command arguments [target, message]
 */
void Server::cmdPRIVMSG(Client &c, const std::vector<std::string> &a)
{
    if (a.size() < 2)
        return sendTo(c, ":server NOTICE " + c.nick + " :PRIVMSG needs <target> :<text>\r\n");

    const std::string &target = a[0];
    const std::string &text = a[1]; /* Parser places trailing message here */

    /* Handle channel message */
    if (!target.empty() && target[0] == '#')
    {
        std::map<std::string, Channel>::iterator it = channels.find(target);
        if (it == channels.end())
            return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");
        Channel &ch = it->second;
        /* Verify sender is channel member */
        if (!ch.isMember(c.fd))
            return sendTo(c, ":server NOTICE " + c.nick + " :Cannot send to channel\r\n");

        /* Broadcast message to all channel members except sender */
        std::string msg = ":" + c.nick + " PRIVMSG " + target + " :" + text + "\r\n";
        broadcastToChannel(ch, c.fd, msg);
        return;
    }
    /* Handle private message to user */
    else
    {
        int dstFd = findFdByNick(target);
        if (dstFd < 0)
            return sendTo(c, ":server NOTICE " + c.nick + " :No such nick\r\n");

        std::map<int, Client>::iterator itc = clients.find(dstFd);
        if (itc == clients.end())
            return; /* User disconnected */
        std::string msg = ":" + c.nick + " PRIVMSG " + target + " :" + text + "\r\n";
        sendTo(itc->second, msg);
        return;
    }
}