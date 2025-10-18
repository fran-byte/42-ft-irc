#pragma once
#include <string>
#include <set>

/**
 * @brief Represents a connected IRC client with connection state and data
 */
struct Client
{
    int fd;                           /* File descriptor for client socket */
    std::string recvBuf, sendBuf;     /* Input and output buffers */
    std::string nick, user, realname; /* Client identification */
    bool passOk, registered;          /* Authentication and registration status */
    bool closing;                     /* Marked for disconnection */
    std::set<std::string> channels;   /* Channels client has joined */

    Client() : fd(-1), passOk(false), registered(false), closing(false) {}
    /*Constructor*/
    explicit Client(int f) : fd(f), passOk(false), registered(false), closing(false) {}
};
