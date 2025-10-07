#include "Server.hpp"
#include <stdexcept>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * @brief Constructs server instance and initializes listening socket
 * @param port Network port to listen on
 * @param pwd Server password for client authentication
 */
Server::Server(unsigned short port, const std::string &pwd)
    : listen_fd(-1), password(pwd)
{
    initListen(port);
    struct pollfd p; /* fd, events, revents */
    p.fd = listen_fd;
    p.events = POLLIN; /* listening socket for incoming connections */
    p.revents = 0;     /* NO events*/
    pfds.push_back(p);
}

/**
 * @brief Destructor closes listening socket
 */
Server::~Server()
{
    if (listen_fd >= 0)
        ::close(listen_fd);
}

/**
 * @brief Sets file descriptor to non-blocking mode
 * @param fd File descriptor to modify
 */
void Server::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        flags = 0;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/**
 * @brief Initializes listening socket on specified port
 * @param port Network port to bind socket to
 */
void Server::initListen(unsigned short port)
{
    listen_fd = ::socket(AF_INET, SOCK_STREAM, 0); /* Protocol: IPv4, Socket type: TCP, Protocol: default (0) */
    throw std::runtime_error("socket failed");
    int yes = 1;
    ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)); /*SO_REUSEADDR: Prevents Error: Address already in use*/
    sockaddr_in addr;                                                     /*COMMS structure family, addr, port*/
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); /*INADDR_ANY: Listens on all interfaces*/
    addr.sin_port = htons(port);
    if (::bind(listen_fd, (sockaddr *)&addr, sizeof(addr)) < 0) /*Associate the socket with the address*/
        throw std::runtime_error("bind failed");
    if (::listen(listen_fd, 128) < 0) /*Listen for connections (max 128 pending)*/
        throw std::runtime_error("listen failed");
    setNonBlocking(listen_fd);
}

/**
 * @brief Main server event loop handling client connections and I/O
 */
void Server::run()
{
    for (;;)
    {
        /* Set poll events: always POLLIN, +POLLOUT if data pending to send */
        for (size_t i = 1; i < pfds.size(); ++i)
        {
            int fd = pfds[i].fd;
            pfds[i].events = clients[fd].sendBuf.empty() ? POLLIN : (POLLIN | POLLOUT);
        }

        /* Wait for events on all file descriptors (infinite timeout) */
        int n = ::poll(&pfds[0], pfds.size(), -1);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("poll failed");
        }

        /* Handle new incoming connections on listening socket */
        if (pfds[0].revents & POLLIN)
            acceptNew();

        /* Process events for all client connections */
        for (size_t i = 1; i < pfds.size(); ++i)
        {
            short ev = pfds[i].revents;
            if (ev & (POLLERR | POLLHUP | POLLNVAL))
            {
                disconnect(i--);
                continue;
            }
            if (ev & POLLIN)
                handleRead(i);
            if (i < pfds.size() && (pfds[i].revents & POLLOUT))
                handleWrite(i);
        }

        /* Close marked connections after sendBuf is emptied */
        for (size_t i = 1; i < pfds.size(); /* ++i inside */)
        {
            int fd = pfds[i].fd;
            std::map<int, Client>::iterator it = clients.find(fd);
            if (it != clients.end())
            {
                Client &cl = it->second;
                /* Graceful disconnect: wait until all pending data sent */
                if (cl.closing && cl.sendBuf.empty())
                {
                    disconnect(i);
                    continue;
                }
            }
            ++i;
        }
    }
}

/**
 * @brief Accepts new client connections and adds to poll set
 */
void Server::acceptNew()
{
    for (;;)
    {
        int cfd = ::accept(listen_fd, NULL, NULL); /*extracts the first pending connection from the listening socket's queue*/
        if (cfd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            std::perror("accept");
            break;
        }
        setNonBlocking(cfd);
        clients.insert(std::make_pair(cfd, Client(cfd))); /*Maps (fd → Client) and inserts into clients structure*/
        struct pollfd p;
        p.fd = cfd;
        p.events = POLLIN;
        p.revents = 0;
        pfds.push_back(p);
        std::fprintf(stderr, "Nuevo cliente (fd=%d)\n", cfd);
    }
}

/**
 * @brief Handles incoming data from client and processes complete lines
 * @param idx Index of client in pollfd vector
 */
void Server::handleRead(size_t idx)
{
    int fd = pfds[idx].fd;
    Client &c = clients[fd]; /* Extract client structure from (map clients)*/
    char buf[4096];
    for (;;)
    {
        ssize_t r = ::recv(fd, buf, sizeof(buf), 0); /* byte received from socket*/
        if (r > 0)
        {
            c.recvBuf.append(buf, r);
            std::string::size_type pos;
            while ((pos = c.recvBuf.find("\r\n")) != std::string::npos)
            {
                std::string line = c.recvBuf.substr(0, pos);
                c.recvBuf.erase(0, pos + 2);
                onLine(c, line); /* Process complete IRC line */
            }
        }
        else if (r == 0) /*client disconnected*/
        {
            disconnect(idx);
            return;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            disconnect(idx);
            return;
        }
    }
}

/**
 * @brief Sends pending data from client's send buffer
 * @param idx Index of client in pollfd vector
 */
void Server::handleWrite(size_t idx)
{
    int fd = pfds[idx].fd;
    Client &c = clients[fd]; /* Extract client structure from (map clients)*/
    while (!c.sendBuf.empty())
    {
        ssize_t n = ::send(fd, c.sendBuf.data(), c.sendBuf.size(), 0); /* byte sent */
        if (n > 0)
            c.sendBuf.erase(0, n);
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            disconnect(idx);
            return;
        }
    }
    pfds[idx].events &= ~POLLOUT; // Clears the POLLOUT bit to stop monitoring write readiness
}

/**
 * @brief Disconnects client and cleans up resources
 * @param idx Index of client in pollfd vector
 */
void Server::disconnect(size_t idx)
{
    int fd = pfds[idx].fd;

    /* Clean up channels if client was still member */
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it != clients.end() && !it->second.channels.empty()) /*NOT is the .end & NOT is .empty*/
        quitCleanup(it->second, "Connection closed");
    ::close(fd);
    clients.erase(fd);
    pfds[idx] = pfds.back(); /* Replace target with last element (swap) */
    pfds.pop_back();         /* Remove last element (now duplicated) */
}

/**
 * @brief Queues message for sending to client
 * @param c Client reference to send to
 * @param msg Message string to send
 */
void Server::sendTo(Client &c, const std::string &msg)
{
    c.sendBuf += msg;
    markWrite(c.fd);
}

/**
 * @brief Marks client fd for write monitoring in poll
 * @param fd File descriptor to monitor for write readiness
 */
void Server::markWrite(int fd)
{
    for (size_t i = 1; i < pfds.size(); ++i)
        if (pfds[i].fd == fd)
        {
            pfds[i].events |= POLLOUT; /*pfds[i].events =  pfds[i].events | POLLOUT*/
            break;
        }
}

/**
 * @brief Processes complete IRC line from client
 * @param c Client that sent the line
 * @param line Complete IRC line to process
 */
void Server::onLine(Client &c, const std::string &line)
{
    Cmd cmd = parseIrcLine(line);
    handleCommand(c, cmd);
}

/**
 * @brief Finds client file descriptor by nickname
 * @param nick Nickname to search for
 * @return File descriptor or -1 if not found
 */
int Server::findFdByNick(const std::string &nick) const
{
    for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
        if (it->second.nick == nick)
            return it->first;
    return -1;
}

/**
 * @brief Broadcasts message to all channel members except sender
 * @param ch Channel to broadcast to
 * @param fromFd Sender's file descriptor to exclude
 * @param msg Message to broadcast
 */
void Server::broadcastToChannel(const Channel &ch, int fromFd, const std::string &msg)
{
    for (std::set<int>::const_iterator it = ch.members.begin(); it != ch.members.end(); ++it)
    {
        if (*it == fromFd) /* Except fromFd*/
            continue;
        std::map<int, Client>::iterator ci = clients.find(*it);
        if (ci != clients.end())
            sendTo(ci->second, msg);
    }
}

/**
 * @brief Cleans up client from all channels and notifies members
 * @param c Client to clean up
 * @param reason Reason for quitting
 */
void Server::quitCleanup(Client &c, const std::string &reason)
{
    const std::string quitMsg = ":" + (c.nick.empty() ? "*" : c.nick) + " QUIT :" + reason + "\r\n";
    std::vector<std::string> chans(c.channels.begin(), c.channels.end()); /*Copy channels*/
    for (size_t i = 0; i < chans.size(); ++i)
    {
        std::map<std::string, Channel>::iterator it = channels.find(chans[i]); /* Find channel in server */
        if (it == channels.end())
            continue;
        Channel &ch = it->second;              /* Get channel reference*/
        broadcastToChannel(ch, c.fd, quitMsg); /* Notify all channel members */
        ch.removeMember(c.fd);
        c.channels.erase(chans[i]);
        if (ch.members.empty())
            channels.erase(it);
        else if (ch.ops.empty())
        {
            int promote = *ch.members.begin(); /* Get first member */
            ch.addOp(promote);                 /* Promote to operator */
        }
    }
}