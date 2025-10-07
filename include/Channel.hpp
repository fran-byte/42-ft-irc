#pragma once
#include <string>
#include <set>

/**
 * @brief Represents an IRC channel with members, modes, and permissions
 */
struct Channel
{
    std::string name, topic, key;
    bool inviteOnly;  /* +i mode: invite-only channel */
    bool topicOpOnly; /* +t mode: only operators can change topic */
    int limit;        /* +l mode: user limit (-1 = unlimited) */
    std::set<int> members, ops, invited;

    Channel() : inviteOnly(false), topicOpOnly(false), limit(-1) {}
    explicit Channel(const std::string &n)
        : name(n), inviteOnly(false), topicOpOnly(false), limit(-1) {}

    /* === Getters / checks === */
    bool isMember(int fd) const { return members.count(fd) != 0; }
    bool isOp(int fd) const { return ops.count(fd) != 0; }
    bool isInvited(int fd) const { return invited.count(fd) != 0; }

    /* === Mode setters === */
    void setInviteOnly(bool on) { inviteOnly = on; }
    void setTopicRestricted(bool on) { topicOpOnly = on; }
    void setKey(const std::string &k) { key = k; }
    void clearKey() { key.clear(); }
    void setLimit(int n) { limit = n; }
    void clearLimit() { limit = -1; }

    /* === Member management === */
    void addMember(int fd)
    {
        members.insert(fd);
        invited.erase(fd);
    }
    void removeMember(int fd)
    {
        members.erase(fd);
        ops.erase(fd);
        invited.erase(fd);
    }
    void addOp(int fd) { ops.insert(fd); }
    void invite(int fd) { invited.insert(fd); }

    /* === JOIN validation === */
    bool canJoin(int fd, const std::string *providedKey) const
    {
        if (limit >= 0 && (int)members.size() >= limit)
            return false; /* +l limit reached */
        if (!key.empty())
        {
            if (!providedKey || *providedKey != key)
                return false; /* +k wrong key */
        }
        if (inviteOnly && invited.count(fd) == 0 && members.count(fd) == 0)
            return false; /* +i not invited */
        return true;
    }
};