#include "Server.hpp"

/**
 * @brief Handles PING command for connection keep-alive
 * @param c Client sending the command
 * @param a Command arguments [token]
 */
void Server::cmdPING(Client &c, const std::vector<std::string> &a)
{
    std::string token = a.empty() ? "" : a.back();
    sendTo(c, ":server PONG :" + token + "\r\n");
}

/**
 * @brief Handles QUIT command for client disconnection
 * @param c Client sending the command
 * @param a Command arguments [reason(optional)]
 */
void Server::cmdQUIT(Client &c, const std::vector<std::string> &a)
{
    const std::string reason = a.empty() ? "Client Quit" : a.back();
    quitCleanup(c, reason);
    /* Mark client for graceful disconnect after sending pending data */
    c.closing = true;
}

/**
 * @brief Routes IRC commands to appropriate handler functions
 * @param c Client sending the command
 * @param cmd Parsed command structure containing verb and arguments
 */
void Server::handleCommand(Client &c, const Cmd &cmd)
{
    /* Handle connection maintenance commands for all clients */
    if (cmd.verb == "PING")
        return cmdPING(c, cmd.args);
    if (cmd.verb == "QUIT")
        return cmdQUIT(c, cmd.args);

    /* Require registration for most commands */
    if (!c.registered)
    {
        if (cmd.verb == "PASS")
            return cmdPASS(c, cmd.args);
        if (cmd.verb == "NICK")
            return cmdNICK(c, cmd.args);
        if (cmd.verb == "USER")
            return cmdUSER(c, cmd.args);
        return sendTo(c, ":server NOTICE * :Register first (PASS/NICK/USER)\r\n");
    }

    /* Route registered client commands to appropriate handlers */
    if (cmd.verb == "JOIN")
        return cmdJOIN(c, cmd.args);
    if (cmd.verb == "PART")
        return cmdPART(c, cmd.args);
    if (cmd.verb == "PRIVMSG")
        return cmdPRIVMSG(c, cmd.args);
    if (cmd.verb == "MODE")
        return cmdMODE(c, cmd.args);
    if (cmd.verb == "TOPIC")
        return cmdTOPIC(c, cmd.args);
    if (cmd.verb == "INVITE")
        return cmdINVITE(c, cmd.args);
    if (cmd.verb == "KICK")
        return cmdKICK(c, cmd.args);

    /* Unknown command response */
    sendTo(c, ":server NOTICE " + (c.nick.empty() ? "*" : c.nick) + " :Unknown command\r\n");
}