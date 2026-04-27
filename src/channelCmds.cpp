#include "../inc/Server.hpp"
#include <sstream>
#include <cstdlib>

/*
  Command: JOIN     | Parameters: <channel>{,<channel>} [<key>{,<key>}]
  Command: PART     | Parameters: <channel>{,<channel>} [<reason>]
  Command: TOPIC    | Parameters: <channel> [<topic>]
  Command: INVITE   | Parameters: <nickname> <channel>
  Command: KICK     | Parameters: <channel> <user> [<comment>]
  Command: MODE     | Parameters: <target> [<modestring> [<mode arguments>...]]
  Command: PRIVMSG  | Parameters: <target>{,<target>} <text to be sent>
*/

// ── SHARED UTILITIES ──────────────────────────────────────────────────────────

// Splits a comma-separated string into 'out'.
static void parseCommaSplit(const std::string &str, std::vector<std::string> &out)
{
	std::istringstream ss(str);
	std::string tok;
	while (std::getline(ss, tok, ','))
		out.push_back(tok);
}

// ── JOIN ───────────────────────────────────────────────────────────────────────

// Returns false (and sends error) if caller is not allowed to join the channel.
static bool checkJoinAllowed(Channel &chan, Client &caller, const std::string &name, const std::string &key)
{
	// +i: channel is invite-only and caller was not invited
	if (chan.isInviteOnly() && !chan.isInvited(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 473 " + caller.getNick() + " " + name + " :Cannot join channel (+i)\r\n");
		return (false);
	}
	// +k: channel has a key and the provided key does not match
	if (chan.hasKey() && !chan.checkKey(key))
	{
		caller.sendMsg(":irc.server 475 " + caller.getNick() + " " + name + " :Cannot join channel (+k)\r\n");
		return (false);
	}
	// +l: channel has reached its member limit
	if (chan.isFull())
	{
		caller.sendMsg(":irc.server 471 " + caller.getNick() + " " + name + " :Cannot join channel (+l)\r\n");
		return (false);
	}
	return (true);
}

// Sends topic (331/332) and names list (353/366) to the joining client.
static void sendJoinReplies(Channel &chan, Client &caller, const std::string &name)
{
	// 331: no topic set  |  332: current topic
	if (chan.getTopic().empty())
		caller.sendMsg(":irc.server 331 " + caller.getNick() + " " + name + " :No topic is set\r\n");
	else
		caller.sendMsg(":irc.server 332 " + caller.getNick() + " " + name + " :" + chan.getTopic() + "\r\n");

	// Build the names list, prefixing operators with '@'
	std::string namesList;
	const std::map<int, Client *> &members = chan.getMembers();
	for (std::map<int, Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (!namesList.empty()) namesList += " ";
		if (chan.isOperator(it->first)) namesList += "@";
		namesList += it->second->getNick();
	}
	// 353: names list  |  366: end of names list
	caller.sendMsg(":irc.server 353 " + caller.getNick() + " = " + name + " :" + namesList + "\r\n");
	caller.sendMsg(":irc.server 366 " + caller.getNick() + " " + name + " :End of /NAMES list\r\n");
}

// Processes joining a single channel: validates name, checks guards, adds member.
static void joinOne(Client &caller, Server &server, const std::string &name, const std::string &key)
{
	if (name.empty() || name[0] != '#')
	{
		caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + name + " :No such channel\r\n");
		return ;
	}
	Channel &chan = server.getOrCreateChannel(name);
	if (chan.hasMember(caller.getSocketFd()))  // already in channel, skip silently
		return ;
	if (!checkJoinAllowed(chan, caller, name, key))  // +i / +k / +l guards
		return ;
	chan.addMember(&caller);
	if (chan.getMembers().size() == 1)  // first member becomes operator
		chan.addOperator(caller.getSocketFd());
	// Notify everyone in the channel, then send topic + names to the joiner
	chan.broadcast(":" + caller.getNick() + "!" + caller.getUser() + "@" + caller.getHost() + " JOIN " + name + "\r\n", -1);
	sendJoinReplies(chan, caller, name);
}

void  joinCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	if (!caller.isRegistered())
	{
		caller.sendMsg(":irc.server 451 " + caller.getNick() + " :You have not registered\r\n");
		return ;
	}
	if (args.size() < 2)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " JOIN :Not enough parameters\r\n");
		return ;
	}
	// Parse comma-separated channel names (e.g. #a,#b) and optional keys (e.g. pass1,pass2)
	std::vector<std::string> chanNames;
	std::vector<std::string> keys;
	parseCommaSplit(args[1], chanNames);
	if (args.size() >= 3)
		parseCommaSplit(args[2], keys);
	for (size_t i = 0; i < chanNames.size(); i++)
	{
		std::string key;
		if (i < keys.size())
			key = keys[i];
		joinOne(caller, server, chanNames[i], key);
	}
}

// ── PART ───────────────────────────────────────────────────────────────────────

// Processes a single channel PART: validates membership and broadcasts before removal.
static void partOne(Client &caller, Server &server, const std::string &name, const std::string &reason)
{
	Channel	*chan;

	chan = server.getChannel(name);
	if (!chan)
	{
		caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + name + " :No such channel\r\n");
		return ;
	}
	if (!chan->hasMember(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 442 " + caller.getNick() + " " + name + " :You're not on that channel\r\n");
		return ;
	}
	// Broadcast before removing so the departing client also receives the message
	std::string partMsg = ":" + caller.getNick() + "!" + caller.getUser() + "@" + caller.getHost() + " PART " + name + " :" + reason + "\r\n";
	chan->broadcast(partMsg, -1);
	chan->removeMember(caller.getSocketFd());
	server.pruneChannel(name);
}

void  partCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	if (!caller.isRegistered())
	{
		caller.sendMsg(":irc.server 451 " + caller.getNick() + " :You have not registered\r\n");
		return ;
	}
	if (args.size() < 2)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " PART :Not enough parameters\r\n");
		return ;
	}
	// Default reason is the caller's nick when none is given
	std::string reason;
	if (args.size() >= 3)
		reason = args[2];
	else
		reason = caller.getNick();
	if (!reason.empty() && reason[0] == ':')  // strip leading IRC colon
		reason.erase(0, 1);
	std::vector<std::string> chanNames;
	parseCommaSplit(args[1], chanNames);
	for (size_t i = 0; i < chanNames.size(); i++)
		partOne(caller, server, chanNames[i], reason);
}

// ── TOPIC ──────────────────────────────────────────────────────────────────────

// Handles the SET path of TOPIC: checks +t lock, strips colon, then broadcasts.
static void topicSet(Channel *chan, Client &caller, const std::string &chanName, const std::string &rawTopic)
{
	// Setting topic: check +t lock
	if (chan->isTopicLocked() && !chan->isOperator(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 482 " + caller.getNick() + " " + chanName + " :You're not channel operator\r\n");
		return ;
	}
	std::string newTopic = rawTopic;
	if (!newTopic.empty() && newTopic[0] == ':')  // strip leading IRC colon
		newTopic.erase(0, 1);
	chan->setTopic(newTopic);
	std::string notify = ":" + caller.getNick() + "!" + caller.getUser() + "@" + caller.getHost() + " TOPIC " + chanName + " :" + newTopic + "\r\n";
	chan->broadcast(notify, -1);
}

void  topicCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	Channel	*chan;

	if (!caller.isRegistered())
	{
		caller.sendMsg(":irc.server 451 " + caller.getNick() + " :You have not registered\r\n");
		return ;
	}
	if (args.size() < 2)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " TOPIC :Not enough parameters\r\n");
		return ;
	}
	const std::string &chanName = args[1];
	chan = server.getChannel(chanName);
	if (!chan)
	{
		caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + chanName + " :No such channel\r\n");
		return ;
	}
	if (!chan->hasMember(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 442 " + caller.getNick() + " " + chanName + " :You're not on that channel\r\n");
		return ;
	}
	// No topic argument: query current topic (331/332)
	if (args.size() == 2)
	{
		if (chan->getTopic().empty())
			caller.sendMsg(":irc.server 331 " + caller.getNick() + " " + chanName + " :No topic is set\r\n");
		else
			caller.sendMsg(":irc.server 332 " + caller.getNick() + " " + chanName + " :" + chan->getTopic() + "\r\n");
		return ;
	}
	topicSet(chan, caller, chanName, args[2]);
}

// ── INVITE ─────────────────────────────────────────────────────────────────────

// Validates the target nick and delivers the INVITE message.
static void sendInviteToTarget(Channel *chan, Client &caller, Server &server,
	const std::string &targetNick, const std::string &chanName)
{
	Client	*target;

	target = server.getClientByNick(targetNick);
	if (!target)
	{
		caller.sendMsg(":irc.server 401 " + caller.getNick() + " " + targetNick + " :No such nick\r\n");
		return ;
	}
	if (chan->hasMember(target->getSocketFd()))
	{
		caller.sendMsg(":irc.server 443 " + caller.getNick() + " " + targetNick + " " + chanName + " :is already on channel\r\n");
		return ;
	}
	// Record the invite, send 341 to the inviter, and notify the target
	chan->addInvite(target->getSocketFd());
	caller.sendMsg(":irc.server 341 " + caller.getNick() + " " + targetNick + " " + chanName + "\r\n");
	target->sendMsg(":" + caller.getNick() + "!" + caller.getUser() + "@" + caller.getHost() + " INVITE " + targetNick + " " + chanName + "\r\n");
}

void  inviteCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	Channel	*chan;

	if (!caller.isRegistered())
	{
		caller.sendMsg(":irc.server 451 " + caller.getNick() + " :You have not registered\r\n");
		return ;
	}
	if (args.size() < 3)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " INVITE :Not enough parameters\r\n");
		return ;
	}
	const std::string &targetNick = args[1];
	const std::string &chanName = args[2];
	chan = server.getChannel(chanName);
	if (!chan)
	{
		caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + chanName + " :No such channel\r\n");
		return ;
	}
	if (!chan->hasMember(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 442 " + caller.getNick() + " " + chanName + " :You're not on that channel\r\n");
		return ;
	}
	// Invite-only channels require the caller to be an operator
	if (chan->isInviteOnly() && !chan->isOperator(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 482 " + caller.getNick() + " " + chanName + " :You're not channel operator\r\n");
		return ;
	}
	sendInviteToTarget(chan, caller, server, targetNick, chanName);
}

// ── KICK ───────────────────────────────────────────────────────────────────────

// Validates the target and executes the kick: broadcasts before removal.
static void kickTarget(Channel *chan, Client &caller, Server &server,
	const std::string &chanName, const std::string &targetNick, const std::string &reason)
{
	Client	*target;

	target = server.getClientByNick(targetNick);
	if (!target || !chan->hasMember(target->getSocketFd()))
	{
		caller.sendMsg(":irc.server 441 " + caller.getNick() + " " + targetNick + " " + chanName + " :They aren't on that channel\r\n");
		return ;
	}
	// Broadcast before removing so the kicked client also receives the message
	std::string kickMsg = ":" + caller.getNick() + "!" + caller.getUser() + "@" + caller.getHost() + " KICK " + chanName + " " + targetNick + " :" + reason + "\r\n";
	chan->broadcast(kickMsg, -1);
	chan->removeMember(target->getSocketFd());
	server.pruneChannel(chanName);
}

void  kickCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	Channel	*chan;

	if (!caller.isRegistered())
	{
		caller.sendMsg(":irc.server 451 " + caller.getNick() + " :You have not registered\r\n");
		return ;
	}
	if (args.size() < 3)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " KICK :Not enough parameters\r\n");
		return ;
	}
	const std::string &chanName = args[1];
	const std::string &targetNick = args[2];
	std::string reason;
	if (args.size() >= 4)
		reason = args[3];
	else
		reason = caller.getNick();
	if (!reason.empty() && reason[0] == ':')  // strip leading IRC colon
		reason.erase(0, 1);
	chan = server.getChannel(chanName);
	if (!chan)
	{
		caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + chanName + " :No such channel\r\n");
		return ;
	}
	if (!chan->hasMember(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 442 " + caller.getNick() + " " + chanName + " :You're not on that channel\r\n");
		return ;
	}
	if (!chan->isOperator(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 482 " + caller.getNick() + " " + chanName + " :You're not channel operator\r\n");
		return ;
	}
	kickTarget(chan, caller, server, chanName, targetNick, reason);
}

// ── MODE ───────────────────────────────────────────────────────────────────────

static void modeQuery(Channel *chan, Client &caller, const std::string &target)
{
	// No modestring: reply with RPL_CHANNELMODEIS (324)
	std::string modes = "+";
	std::string modeArgs;
	if (chan->isInviteOnly())  modes += "i";
	if (chan->isTopicLocked()) modes += "t";
	if (chan->hasKey())
	{
		modes += "k";
		modeArgs += " " + chan->getKey();
	}
	if (chan->hasLimit())
	{
		std::ostringstream oss;
		oss << chan->getLimit();
		modes += "l";
		modeArgs += " " + oss.str();
	}
	caller.sendMsg(":irc.server 324 " + caller.getNick() + " " + target + " " + modes + modeArgs + "\r\n");
}

// +k: set or clear the channel password key
static bool handleModeK(Channel *chan, bool adding, size_t &argIdx,
	const std::vector<std::string> &args, std::string &appliedArgs, Client &caller)
{
	if (adding)
	{
		if (argIdx >= args.size())
		{
			caller.sendMsg(":irc.server 461 " + caller.getNick() + " MODE :Not enough parameters\r\n");
			return (false);
		}
		chan->setKey(args[argIdx]);
		appliedArgs += " " + args[argIdx++];
	}
	else
	{
		chan->clearKey();
		if (argIdx < args.size()) argIdx++; // consume the '*' some clients send
	}
	return (true);
}

// +o: grant or revoke operator status for a channel member
static bool handleModeO(Channel *chan, bool adding, size_t &argIdx,
	const std::vector<std::string> &args, std::string &appliedArgs, Client &caller, Server &server)
{
	Client	*targetClient;

	if (argIdx >= args.size())
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " MODE :Not enough parameters\r\n");
		return (false);
	}
	std::string targetNick = args[argIdx++];
	targetClient = server.getClientByNick(targetNick);
	if (!targetClient || !chan->hasMember(targetClient->getSocketFd()))
	{
		caller.sendMsg(":irc.server 441 " + caller.getNick() + " " + targetNick + " " + chan->getName() + " :They aren't on that channel\r\n");
		return (false);
	}
	if (adding)
		chan->addOperator(targetClient->getSocketFd());
	else
		chan->removeOperator(targetClient->getSocketFd());
	appliedArgs += " " + targetNick;
	return (true);
}

// +l: set or clear the maximum member limit
static bool handleModeL(Channel *chan, bool adding, size_t &argIdx,
	const std::vector<std::string> &args, std::string &appliedArgs, Client &caller)
{
	int limit;

	if (adding)
	{
		if (argIdx >= args.size())
		{
			caller.sendMsg(":irc.server 461 " + caller.getNick() + " MODE :Not enough parameters\r\n");
			return (false);
		}
		limit = std::atoi(args[argIdx].c_str());
		if (limit <= 0)
		{
			argIdx++;
			return (false);
		}
		chan->setLimit(limit);
		appliedArgs += " " + args[argIdx++];
	}
	else
		chan->clearLimit();
	return (true);
}

// Returns false and sends the appropriate error if caller is not a member/op.
static bool checkChanOp(Channel *chan, Client &caller, const std::string &target)
{
	if (!chan->hasMember(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 442 " + caller.getNick() + " " + target + " :You're not on that channel\r\n");
		return (false);
	}
	if (!chan->isOperator(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 482 " + caller.getNick() + " " + target + " :You're not channel operator\r\n");
		return (false);
	}
	return (true);
}

// Dispatches a single mode character to its handler. Returns true if applied.
static bool applyOneMode(Channel *chan, char c, bool adding, size_t &argIdx,
	const std::vector<std::string> &args, std::string &appliedArgs, Client &caller, Server &server)
{
	if (c == 'i')
	{
		chan->setInviteOnly(adding);
		return (true);
	}
	if (c == 't')
	{
		chan->setTopicLocked(adding);
		return (true);
	}
	if (c == 'k')
		return (handleModeK(chan, adding, argIdx, args, appliedArgs, caller));
	if (c == 'o')
		return (handleModeO(chan, adding, argIdx, args, appliedArgs, caller, server));
	if (c == 'l')
		return (handleModeL(chan, adding, argIdx, args, appliedArgs, caller));
	caller.sendMsg(":irc.server 472 " + caller.getNick() + " " + std::string(1, c) + " :is unknown mode char to me\r\n");
	return (false);
}

// Iterates modeStr, applies each flag, fills appliedStr/appliedArgs.
static void applyModes(Channel *chan, Client &caller, Server &server,
	const std::vector<std::string> &args, std::string &appliedStr, std::string &appliedArgs)
{
	const std::string	&modeStr = args[2];
	size_t				argIdx = 3;
	bool				adding = true;
	char				lastDir = 0;

	for (size_t i = 0; i < modeStr.size(); i++)
	{
		char	c;
		char	thisDir;
		bool	applied;

		c = modeStr[i];
		if (c == '+')
		{
			adding = true;
			continue;
		}
		if (c == '-')
		{
			adding = false;
			continue;
		}
		if (adding)
			thisDir = '+';
		else
			thisDir = '-';
		applied = applyOneMode(chan, c, adding, argIdx, args, appliedArgs, caller, server);
		if (applied)
		{
			// Only emit +/- when the direction changes (avoids "++ik", produces "+ik")
			if (thisDir != lastDir)
			{
				appliedStr += thisDir;
				lastDir = thisDir;
			}
			appliedStr += c;
		}
	}
}

void  modeCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	Channel	*chan;
	
	if (args.size() < 2)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " MODE :Not enough parameters\r\n");
		return ;
	}
	const std::string &target = args[1];
	if (target.empty() || target[0] != '#')
		return ;
	chan = server.getChannel(target);
	if (!chan)
	{
		caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + target + " :No such channel\r\n");
		return ;
	}
	if (args.size() == 2)  // no modestring: query current modes
		return (modeQuery(chan, caller, target));
	if (!checkChanOp(chan, caller, target))  // must be a member and an operator
		return ;
	// Collect the applied changes, then broadcast once
	std::string appliedStr;
	std::string appliedArgs;
	applyModes(chan, caller, server, args, appliedStr, appliedArgs);
	if (!appliedStr.empty())
	{
		std::string notify = ":" + caller.getNick() + "!" + caller.getUser() + "@irc.server MODE " + target + " " + appliedStr + appliedArgs + "\r\n";
		chan->broadcast(notify, -1);
	}
}

// ── PRIVMSG ─────────────────────────────────────────────────────────────────────

// Sends a PRIVMSG to a channel, excluding the sender.
static void privmsgToChannel(Client &caller, Server &server, const std::string &target, const std::string &msg)
{
	Channel	*chan;

	chan = server.getChannel(target);
	if (!chan)
	{
		caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + target + " :No such channel\r\n");
		return ;
	}
	if (!chan->hasMember(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 404 " + caller.getNick() + " " + target + " :Cannot send to channel\r\n");
		return ;
	}
	chan->broadcast(msg, caller.getSocketFd());  // exclude sender from broadcast
}

// Sends a PRIVMSG directly to a nick.
static void privmsgToNick(Client &caller, Server &server, const std::string &target, const std::string &msg)
{
	Client	*dest;

	dest = server.getClientByNick(target);
	if (!dest)
	{
		caller.sendMsg(":irc.server 401 " + caller.getNick() + " " + target + " :No such nick\r\n");
		return ;
	}
	dest->sendMsg(msg);
}

void  privmsgCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	if (!caller.isRegistered())
	{
		caller.sendMsg(":irc.server 451 " + caller.getNick() + " :You have not registered\r\n");
		return ;
	}
	if (args.size() < 2)
	{
		caller.sendMsg(":irc.server 411 " + caller.getNick() + " :No recipient given (PRIVMSG)\r\n");
		return ;
	}
	if (args.size() < 3)
	{
		caller.sendMsg(":irc.server 412 " + caller.getNick() + " :No text to send\r\n");
		return ;
	}
	std::string text = args[2];
	if (!text.empty() && text[0] == ':')  // strip leading IRC colon from trailing parameter
		text.erase(0, 1);
	const std::string &target = args[1];
	std::string msg = ":" + caller.getNick() + "!" + caller.getUser() + "@" + caller.getHost() + " PRIVMSG " + target + " :" + text + "\r\n";
	if (!target.empty() && target[0] == '#')  // channel message
		privmsgToChannel(caller, server, target, msg);
	else  // direct nick message
		privmsgToNick(caller, server, target, msg);
}
