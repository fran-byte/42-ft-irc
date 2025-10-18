#include "Server.hpp"
#include <iostream>

void introServer()
{
    const std::string green = "\033[1;32m";
    const std::string yellow = "\033[1;33m";
    const std::string red = "\033[1;31m";
    const std::string cyan = "\033[1;36m";
    const std::string reset = "\033[0m";
    const std::string bold = "\033[1m";

    std::cout << green << std::endl;
    std::cout << "               ██╗██████╗  ██████╗" << std::endl;
    std::cout << "               ██║██╔══██╗██╔════╝" << std::endl;
    std::cout << "               ██║██████╔╝██║     " << std::endl;
    std::cout << "               ██║██╔══██╗██║     " << std::endl;
    std::cout << "               ██║██║  ██║╚██████╗" << std::endl;
    std::cout << "               ╚═╝╚═╝  ╚═╝ ╚═════╝" << std::endl;
    std::cout << std::endl;
    std::cout << "  ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗ " << std::endl;
    std::cout << "  ██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗" << std::endl;
    std::cout << "  ███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝" << std::endl;
    std::cout << "  ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗" << std::endl;
    std::cout << "  ███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║" << std::endl;
    std::cout << "  ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝" << std::endl;
    std::cout << std::endl;
    std::cout << "      ╔═════════════════════════════════════╗" << std::endl;
    std::cout << "      ║    IRC Commands Used in HexChat     ║" << std::endl;
    std::cout << "      ╠═════════════════════════════════════╣" << std::endl;
    std::cout << reset;
    std::cout << "      ║                                     ║" << std::endl;
    std::cout << "      ║   Ctrl+C → Disconnect server        ║" << std::endl;
    std::cout << "      ║                                     ║" << std::endl;
    std::cout << "      ║ " << green << "AUTHENTICATION" << reset << "                      ║" << std::endl;
    std::cout << "      ║   /PASS <password>                  ║" << std::endl;
    std::cout << "      ║   /NICK <nickname>" << green << " (Auto)" << reset << "           ║" << std::endl;
    std::cout << "      ║   /USER <user> <0> <0> <name>" << green << " (Auto)" << reset << "║" << std::endl;
    std::cout << "      ║ " << green << "CHANNELS" << reset << "                            ║" << std::endl;
    std::cout << "      ║   /JOIN <channel> [:key]            ║" << std::endl;
    std::cout << "      ║   /PART <channel>                   ║" << std::endl;
    std::cout << "      ║   /TOPIC <channel> [:topic]         ║" << std::endl;
    std::cout << "      ║ " << green << "OPERATORS" << reset << "                           ║" << std::endl;
    std::cout << "      ║   /KICK <channel> <user>            ║" << std::endl;
    std::cout << "      ║   /INVITE <user> <channel>          ║" << std::endl;
    std::cout << "      ║   /MODE <channel> <mode>            ║" << std::endl;
    std::cout << "      ║ " << yellow << "CHANNEL MODES" << reset << "                       ║" << std::endl;
    std::cout << "      ║   +i/-i → Invite-only               ║" << std::endl;
    std::cout << "      ║   +t/-t → Topic restriction         ║" << std::endl;
    std::cout << "      ║   +k/-k <key> → Password            ║" << std::endl;
    std::cout << "      ║   +o/-o <user> → Operator           ║" << std::endl;
    std::cout << "      ║   +l/-l <limit> → User limit        ║" << std::endl;
    std::cout << "      ║ " << green << "MESSAGING" << reset << "                           ║" << std::endl;
    std::cout << "      ║   /PRIVMSG <target> <msg>           ║" << std::endl;
    std::cout << "      ║   /QUIT [:message]                  ║" << std::endl;
    std::cout << "      ║ " << cyan << "NETCAT" << reset << "                              ║" << std::endl;
    std::cout << "      ║   nc -C 127.0.0.1 <PORT>            ║" << std::endl;
    std::cout << "      ║   For 'nc' do not use the '/' prefix║" << std::endl;
    std::cout << "      ╚═════════════════════════════════════╝" << std::endl;
    std::cout << reset;
}
