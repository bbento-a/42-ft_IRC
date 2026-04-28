#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include <map>
#include <set>
#include <string>

class Channel
{
	// ── DATA ──────────────────────────────────────────────────────────────────
	private:

	std::string             _name;
	std::string             _topic;
	std::map<int, Client *> _members;   // fd -> Client*
	std::set<int>           _operators; // fds with operator privilege

	// MODE flags
	bool        _inviteOnly;  // +i
	bool        _topicLocked; // +t
	bool        _hasKey;      // +k
	std::string _key;
	bool        _hasLimit;    // +l
	int         _limit;

	std::set<int> _invited; // fds allowed to bypass +i

	// ── INTERFACE ─────────────────────────────────────────────────────────────
	public:

	explicit Channel(const std::string &name);

	// Getters
	std::string                    getName(void) const;
	std::string                    getTopic(void) const;
	const std::map<int, Client *> &getMembers(void) const;
	std::string                    getKey(void) const;
	int                            getLimit(void) const;

	// Queries
	bool  hasMember(int fd) const;
	bool  isOperator(int fd) const;
	bool  isInvited(int fd) const;
	bool  isInviteOnly(void) const;
	bool  isTopicLocked(void) const;
	bool  hasKey(void) const;
	bool  checkKey(const std::string &key) const;
	bool  hasLimit(void) const;
	bool  isFull(void) const;

	// Membership
	void  addMember(Client *client);
	void  removeMember(int fd);
	void  addOperator(int fd);
	void  removeOperator(int fd);
	void  addInvite(int fd);

	// Mode setters
	void  setTopic(const std::string &topic);
	void  setInviteOnly(bool val);
	void  setTopicLocked(bool val);
	void  setKey(const std::string &key);
	void  clearKey(void);
	void  setLimit(int limit);
	void  clearLimit(void);

	// Actions
	void  broadcast(const std::string &msg, int excludeFd = -1) const;
};

#endif