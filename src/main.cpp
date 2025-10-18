#include <iostream>
#include <cstdlib>
#include "Server.hpp"
#include <csignal>

/* Global variable to control the shutdown */
volatile sig_atomic_t g_shutdown = 0;

/* Signal handler */
void signalHandler(int signal)
{
    (void)signal;
    g_shutdown = 1;
    std::cout << std::endl;
    std::cout << "\033[1;31m🔴 Received shutdown signal. Closing server...\033[0m" << std::endl;
}

/**
 * @brief Main entry point for IRC server application
 * @param argv Argument vector [program, port, password]
 * @details Initializes and runs IRC server on specified port with password authentication
 */
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
        return 1;
    }

    std::signal(SIGINT, signalHandler);  /* Ctrl+C */
    std::signal(SIGTERM, signalHandler); /* kill command */

    unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));
    std::string password = argv[2];
    try
    {
        introServer();
        Server srv(port, password);
        srv.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
