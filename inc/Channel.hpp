#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include <string>
#include <map>
#include <set>

class Channel
{
	private:

	std::string				_name;
	std::string				_topic;
	std::string				_key;       // MODE +k
	std::map<int, Client *>	_members;   // fd -> Client (regular + ops)
	std::set<int>			_operators; // fds with op privilege
	std::set<int>			_invited;   // fds invited (for MODE +i)

	bool	_inviteOnly;  // MODE +i
	bool	_topicLocked; // MODE +t (only ops can change topic)
	bool	_hasKey;      // MODE +k
	int		_limit;       // MODE +l
	bool	_hasLimit;    // MODE +l

	public:

	Channel(const std::string &name);

	std::string	getName(void) const;
	std::string	getTopic(void) const;

	bool	hasMember(int fd) const;
	bool	isOperator(int fd) const;
	bool	isInvited(int fd) const;
	bool		isInviteOnly(void) const;
	bool		isTopicLocked(void) const;
	bool		checkKey(const std::string &key) const;
	std::string	getKey(void) const;
	bool		hasKey(void) const;
	int			getLimit(void) const;
	bool		hasLimit(void) const;
	bool		isFull(void) const;

	void	addMember(Client *client);
	void	removeMember(int fd);
	void	addOperator(int fd);
	void	removeOperator(int fd);
	void	addInvite(int fd);

	void	setTopic(const std::string &topic);
	void	broadcast(const std::string &msg, int excludeFd = -1) const;

	void	setInviteOnly(bool val);
	void	setTopicLocked(bool val);
	void	setKey(const std::string &key);
	void	clearKey(void);
	void	setLimit(int limit);
	void	clearLimit(void);
};

#endif