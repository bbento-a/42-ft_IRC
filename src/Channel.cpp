#include "../inc/Channel.hpp"

// Initialises a channel with its name. All MODE flags start off.
Channel::Channel(const std::string &name)
	: _name(name), _topic(""), _key(""),
	  _inviteOnly(false), _topicLocked(false), _hasKey(false),
	  _limit(0), _hasLimit(false)
{}

// Returns the channel name (e.g. "#general").
std::string	Channel::getName(void) const
{
	return _name;
}

// Returns the current topic. Empty string means no topic is set.
std::string	Channel::getTopic(void) const
{
	return _topic;
}

// Returns true if the client with this fd is in the channel.
bool	Channel::hasMember(int fd) const
{
	return _members.count(fd) > 0;
}

// Returns true if the client with this fd has operator privileges.
// Used to gate KICK, INVITE, TOPIC (when +t), and MODE.
bool	Channel::isOperator(int fd) const
{
	return _operators.count(fd) > 0;
}

// Returns true if the client with this fd has been invited.
// Checked on JOIN when the channel is invite-only (+i).
bool	Channel::isInvited(int fd) const
{
	return _invited.count(fd) > 0;
}

// Returns true if the channel is invite-only (MODE +i).
// JOIN must reject clients that are not in _invited when this is true.
bool	Channel::isInviteOnly(void) const
{
	return _inviteOnly;
}

// Returns true if only operators can change the topic (MODE +t).
// TOPIC must check this before allowing a regular user to set a new topic.
bool	Channel::isTopicLocked(void) const
{
	return _topicLocked;
}

// Returns true if the channel has a key set and the provided key matches.
// JOIN must call this when MODE +k is active and reject on mismatch.
bool	Channel::checkKey(const std::string &key) const
{
	return (_hasKey && _key == key);
}

// Returns the channel key (empty string if none). Used in RPL_CHANNELMODEIS.
std::string	Channel::getKey(void) const
{
	return _key;
}

// Returns true if a key is currently set (+k active).
bool	Channel::hasKey(void) const
{
	return _hasKey;
}

// Adds a client to the channel's member list, keyed by their socket fd.
// Called by JOIN after all checks (invite, key, etc.) pass.
void	Channel::addMember(Client *client)
{
	_members[client->getSocketFd()] = client;
}

// Removes a client from the channel entirely (members, operators, invited).
// Called by PART, KICK, and QUIT so no stale entries remain.
void	Channel::removeMember(int fd)
{
	_members.erase(fd);
	_operators.erase(fd);
	_invited.erase(fd);
}

// Grants operator status to a client already in the channel.
// The first client to JOIN a channel should be made an operator.
void	Channel::addOperator(int fd)
{
	_operators.insert(fd);
}

// Revokes operator status from a client.
void	Channel::removeOperator(int fd)
{
	_operators.erase(fd);
}

// Records that a client has been invited to this channel.
// Called by INVITE; allows the client to bypass the +i gate on JOIN.
void	Channel::addInvite(int fd)
{
	_invited.insert(fd);
}

// Sets the channel topic. Called by TOPIC after permission checks.
void	Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}

// Sends msg to every member, skipping excludeFd (pass -1 to send to all).
// Used by PRIVMSG, JOIN, PART, KICK, QUIT to notify the whole channel.
void	Channel::broadcast(const std::string &msg, int excludeFd) const
{
	for (std::map<int, Client *>::const_iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (it->first != excludeFd)
			it->second->sendMsg(msg);
	}
}

// Enables or disables invite-only mode (MODE +i / -i).
void	Channel::setInviteOnly(bool val)
{
	_inviteOnly = val;
}

// Enables or disables the topic lock (MODE +t / -t).
void	Channel::setTopicLocked(bool val)
{
	_topicLocked = val;
}

// Sets the channel key and activates the +k flag (MODE +k <key>).
void	Channel::setKey(const std::string &key)
{
	_key = key;
	_hasKey = true;
}

// Removes the channel key and deactivates the +k flag (MODE -k).
void	Channel::clearKey(void)
{
	_key = "";
	_hasKey = false;
}

// Sets the user limit and activates the +l flag (MODE +l <limit>).
void	Channel::setLimit(int limit)
{
	_limit = limit;
	_hasLimit = true;
}

// Removes the user limit and deactivates the +l flag (MODE -l).
void	Channel::clearLimit(void)
{
	_limit = 0;
	_hasLimit = false;
}

// Returns the current user limit. Meaningful only when hasLimit() is true.
int	Channel::getLimit(void) const
{
	return _limit;
}

// Returns true if a user limit is currently set (+l active).
bool	Channel::hasLimit(void) const
{
	return _hasLimit;
}

// Returns true if the channel is full (member count >= limit).
// JOIN should call this when hasLimit() is true.
bool	Channel::isFull(void) const
{
	return _hasLimit && (int)_members.size() >= _limit;
}

// Returns a const reference to the map of members (fd -> Client*).
// Used by JOIN to build PRL_NAMREPLY
const std::map<int, Client *> &Channel::getMembers(void) const
{
	return _members;
}
