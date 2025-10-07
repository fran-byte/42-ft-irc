#pragma once
#include <vector>
#include <string>
#include <sstream>

/**
 * @brief Represents a parsed IRC command with verb and arguments
 */
struct Cmd
{
    std::string verb;
    std::vector<std::string> args;
};

/**
 * @brief Parses raw IRC line into command structure
 * @param line Raw IRC input line (without \r\n)
 * @return Parsed command with uppercase verb and separated arguments
 */
inline Cmd parseIrcLine(const std::string &line)
{
    Cmd out;
    std::string s = line, trailing;
    /* Extract trailing parameter (after " :") */
    std::string::size_type p = s.find(" :");
    if (p != std::string::npos)
    {
        trailing = s.substr(p + 2);
        s.erase(p);
    }
    /* Split remaining string into tokens */
    std::istringstream iss(s);
    iss >> out.verb;
    for (std::string tok; iss >> tok;)
        out.args.push_back(tok);
    /* Add trailing parameter as last argument */
    if (!trailing.empty())
        out.args.push_back(trailing);
    /* Convert verb to uppercase for case-insensitive comparison */
    for (size_t i = 0; i < out.verb.size(); ++i)
        out.verb[i] = std::toupper(out.verb[i]);
    return out;
}