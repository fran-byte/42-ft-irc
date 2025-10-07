#include "Server.hpp"
#include <cstdlib>

/**
 * @brief Checks if a mode character requires an argument when being set
 * @param f Mode character to check
 * @return True if mode requires argument
 */
static bool needsArg(char f) { return f == 'k' || f == 'o' || f == 'l'; }

/**
 * @brief Handles MODE command for channel mode management
 * @param c Client sending the command
 * @param a Command arguments [target, modes, args...]
 */
void Server::cmdMODE(Client &c, const std::vector<std::string> &a)
{
    if (a.empty())
        return sendTo(c, ":server NOTICE " + c.nick + " :MODE needs a target\r\n");
    const std::string &target = a[0];
    /* Only channel modes are supported */
    if (target.empty() || target[0] != '#')
        return sendTo(c, ":server NOTICE " + c.nick + " :Only channel modes supported\r\n");

    std::map<std::string, Channel>::iterator it = channels.find(target);
    if (it == channels.end())
        return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");
    Channel &ch = it->second;

    /* Display current modes if no mode changes requested */
    if (a.size() == 1)
    {
        std::string modes = "+";
        if (ch.inviteOnly)
            modes += "i";
        if (ch.topicOpOnly)
            modes += "t";
        if (!ch.key.empty())
            modes += "k";
        if (ch.limit >= 0)
            modes += "l";
        return sendTo(c, ":server NOTICE " + c.nick + " :" + target + " " + modes + "\r\n");
    }

    /* Require channel operator privileges for mode changes */
    if (!ch.isOp(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not channel operator\r\n");

    std::string modeStr = a[1];
    std::vector<std::string> args;
    /* Extract mode arguments */
    for (size_t i = 2; i < a.size(); ++i)
        args.push_back(a[i]);

    bool set = true;
    /* Process each mode character in sequence */
    for (size_t i = 0; i < modeStr.size(); ++i)
    {
        char m = modeStr[i];
        if (m == '+')
        {
            set = true;
            continue;
        }
        if (m == '-')
        {
            set = false;
            continue;
        }

        std::string arg;
        /* Extract argument for modes that require one */
        if (needsArg(m) && set)
        {
            if (args.empty())
                continue; /* Skip if missing argument */
            arg = args.front();
            args.erase(args.begin());
        }

        /* Apply mode changes */
        switch (m)
        {
        case 'i':
            ch.setInviteOnly(set);
            break;
        case 't':
            ch.setTopicRestricted(set);
            break;
        case 'k':
            if (set)
                ch.setKey(arg);
            else
                ch.clearKey();
            break;
        case 'l':
            if (set)
                ch.setLimit(std::atoi(arg.c_str()));
            else
                ch.clearLimit();
            break;
        case 'o':
        {
            /* Operator privilege management */
            int fdTarget = findFdByNick(arg);
            if (fdTarget < 0 || !ch.isMember(fdTarget))
                break;
            if (set)
                ch.ops.insert(fdTarget);
            else
                ch.ops.erase(fdTarget);
        }
        break;
        default: /* Ignore unknown modes */
            break;
        }
    }

    /* Broadcast mode changes to all channel members */
    std::string notif = ":" + c.nick + " MODE " + target + " " + modeStr;
    for (size_t i = 0; i < args.size(); ++i)
        notif += " " + args[i];
    notif += "\r\n";
    broadcastToChannel(ch, -1, notif);
}