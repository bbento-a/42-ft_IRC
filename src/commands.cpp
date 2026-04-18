#include "../inc/Server.hpp"
#include <sstream>
#include <cstdlib>
/* 
my source for cmds: 
https://modern.ircdocs.horse/#connection-messages


     Command: PASS
  Parameters: <password>
  Errors:
	ERR_NEEDMOREPARAMS (461)
	ERR_ALREADYREGISTERED (462)
	ERR_PASSWDMISMATCH (464) 

     Command: NICK
  Parameters: <nickname>
  Errors:
	ERR_NONICKNAMEGIVEN (431)
	ERR_ERRONEUSNICKNAME (432)
	ERR_NICKNAMEINUSE (433)
	ERR_NICKCOLLISION (436)


     Command: USER
  Parameters: <username> 0 * <realname>
  Errors:
   ERR_NEEDMOREPARAMS (461)
   ERR_ALREADYREGISTERED (462) 


    Command: QUIT
   Parameters: [<reason>]
  Errors:
   None


      Command: JOIN
  Parameters: <channel>{,<channel>} [<key>{,<key>}]
  Alt Params: 0

       Command: PART
  Parameters: <channel>{,<channel>} [<reason>]

     Command: TOPIC
  Parameters: <channel> [<topic>]

     Command: INVITE
  Parameters: <nickname> <channel>

      Command: KICK
   Parameters: <channel> <user> *( "," <user> ) [<comment>]

     Command: MODE
  Parameters: <target> [<modestring> [<mode arguments>...]]



       Command: PRIVMSG
  Parameters: <target>{,<target>} <text to be sent>
*/

static void sendWelcome(Client &client)
{
	const std::string &nick = client.getNick();
	client.sendMsg(":irc.server 001 " + nick + " :Welcome to the IRC server " + nick + "!" + client.getUser() + "@localhost\r\n");
	client.sendMsg(":irc.server 002 " + nick + " :Your host is irc.server, running version 1.0\r\n");
	client.sendMsg(":irc.server 003 " + nick + " :This server was created today\r\n");
	client.sendMsg(":irc.server 004 " + nick + " irc.server 1.0 o itkol\r\n");
}

void  passCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	if (caller.isRegistered())
	{
		caller.sendMsg(":irc.server 462 " + caller.getNick() + " :You may not reregister\r\n");
		return ;
	}
	if (args.size() < 2)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " PASS :Not enough parameters\r\n");
		return ;
	}
	if (!server.checkPassword(args[1]))
	{
		caller.sendMsg(":irc.server 464 " + caller.getNick() + " :Password incorrect\r\n");
		return ;
	}
	caller.setPassVerified(true);
}

void  nickCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	if (args.size() < 2 || args[1].empty())
	{
		caller.sendMsg(":irc.server 431 " + caller.getNick() + " :No nickname given\r\n");
		return ;
	}
	const std::string &newNick = args[1];
	char first = newNick[0];
	if (!std::isalpha(first) && first != '_' && first != '-')
	{
		caller.sendMsg(":irc.server 432 " + caller.getNick() + " " + newNick + " :Erroneous nickname\r\n");
		return ;
	}
	if (server.isNickInUse(newNick) && newNick != caller.getNick())
	{
		caller.sendMsg(":irc.server 433 " + caller.getNick() + " " + newNick + " :Nickname is already in use\r\n");
		return ;
	}
	caller.setNick(newNick);
	if (caller.isRegistered())
		sendWelcome(caller);
}

void  userCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	(void)server;
	if (caller.isRegistered())
	{
		caller.sendMsg(":irc.server 462 " + caller.getNick() + " :You may not reregister\r\n");
		return ;
	}
	if (args.size() < 2)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " USER :Not enough parameters\r\n");
		return ;
	}
	caller.setUser(args[1]);
	if (caller.isRegistered())
		sendWelcome(caller);
}
void  quitCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	std::string reason = (args.size() >= 2) ? args[1] : "Client quit";
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	std::string quitMsg = ":" + caller.getNick() + "!" + caller.getUser() + "@localhost QUIT :" + reason + "\r\n";
	server.removeFromAllChannels(caller.getSocketFd(), quitMsg);
	caller.sendMsg("ERROR :Closing connection\r\n");
	caller.setWantsQuit(true);
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
	// Parse comma-separated channel names and optional keys
	std::vector<std::string> chanNames;
	std::vector<std::string> keys;
	{
		std::istringstream ss(args[1]);
		std::string tok;
		while (std::getline(ss, tok, ','))
			chanNames.push_back(tok);
	}
	if (args.size() >= 3)
	{
		std::istringstream ss(args[2]);
		std::string tok;
		while (std::getline(ss, tok, ','))
			keys.push_back(tok);
	}
	for (size_t i = 0; i < chanNames.size(); i++)
	{
		const std::string &name = chanNames[i];
		std::string key = (i < keys.size()) ? keys[i] : "";
		if (name.empty() || name[0] != '#')
		{
			caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + name + " :No such channel\r\n");
			continue ;
		}
		Channel &chan = server.getOrCreateChannel(name);
		if (chan.hasMember(caller.getSocketFd()))
			continue ;
		if (chan.isInviteOnly() && !chan.isInvited(caller.getSocketFd()))
		{
			caller.sendMsg(":irc.server 473 " + caller.getNick() + " " + name + " :Cannot join channel (+i)\r\n");
			continue ;
		}
		if (chan.hasKey() && !chan.checkKey(key))
		{
			caller.sendMsg(":irc.server 475 " + caller.getNick() + " " + name + " :Cannot join channel (+k)\r\n");
			continue ;
		}
		if (chan.isFull())
		{
			caller.sendMsg(":irc.server 471 " + caller.getNick() + " " + name + " :Cannot join channel (+l)\r\n");
			continue ;
		}
		chan.addMember(&caller);
		// First member in the channel becomes operator
		if (chan.getMembers().size() == 1)
			chan.addOperator(caller.getSocketFd());
		// Notify everyone in the channel
		std::string joinMsg = ":" + caller.getNick() + "!" + caller.getUser() + "@localhost JOIN " + name + "\r\n";
		chan.broadcast(joinMsg, -1);
		// Topic
		if (chan.getTopic().empty())
			caller.sendMsg(":irc.server 331 " + caller.getNick() + " " + name + " :No topic is set\r\n");
		else
			caller.sendMsg(":irc.server 332 " + caller.getNick() + " " + name + " :" + chan.getTopic() + "\r\n");
		// Names list
		std::string namesList;
		const std::map<int, Client *> &members = chan.getMembers();
		for (std::map<int, Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
		{
			if (!namesList.empty()) namesList += " ";
			if (chan.isOperator(it->first)) namesList += "@";
			namesList += it->second->getNick();
		}
		caller.sendMsg(":irc.server 353 " + caller.getNick() + " = " + name + " :" + namesList + "\r\n");
		caller.sendMsg(":irc.server 366 " + caller.getNick() + " " + name + " :End of /NAMES list\r\n");
	}
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
	std::string reason = (args.size() >= 3) ? args[2] : caller.getNick();
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	std::vector<std::string> chanNames;
	{
		std::istringstream ss(args[1]);
		std::string tok;
		while (std::getline(ss, tok, ','))
			chanNames.push_back(tok);
	}
	for (size_t i = 0; i < chanNames.size(); i++)
	{
		const std::string &name = chanNames[i];
		Channel *chan = server.getChannel(name);
		if (!chan)
		{
			caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + name + " :No such channel\r\n");
			continue ;
		}
		if (!chan->hasMember(caller.getSocketFd()))
		{
			caller.sendMsg(":irc.server 442 " + caller.getNick() + " " + name + " :You're not on that channel\r\n");
			continue ;
		}
		std::string partMsg = ":" + caller.getNick() + "!" + caller.getUser()
			+ "@localhost PART " + name + " :" + reason + "\r\n";
		chan->broadcast(partMsg, -1);
		chan->removeMember(caller.getSocketFd());
	}
}
void  topicCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
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
	Channel *chan = server.getChannel(chanName);
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
	// No topic argument: query current topic
	if (args.size() == 2)
	{
		if (chan->getTopic().empty())
			caller.sendMsg(":irc.server 331 " + caller.getNick() + " " + chanName + " :No topic is set\r\n");
		else
			caller.sendMsg(":irc.server 332 " + caller.getNick() + " " + chanName + " :" + chan->getTopic() + "\r\n");
		return ;
	}
	// Setting topic: check +t lock
	if (chan->isTopicLocked() && !chan->isOperator(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 482 " + caller.getNick() + " " + chanName + " :You're not channel operator\r\n");
		return ;
	}
	std::string newTopic = args[2];
	if (!newTopic.empty() && newTopic[0] == ':')
		newTopic.erase(0, 1);
	chan->setTopic(newTopic);
	std::string notify = ":" + caller.getNick() + "!" + caller.getUser()
		+ "@localhost TOPIC " + chanName + " :" + newTopic + "\r\n";
	chan->broadcast(notify, -1);
}
void  inviteCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
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
	Channel *chan = server.getChannel(chanName);
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
	if (chan->isInviteOnly() && !chan->isOperator(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 482 " + caller.getNick() + " " + chanName + " :You're not channel operator\r\n");
		return ;
	}
	Client *target = server.getClientByNick(targetNick);
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
	chan->addInvite(target->getSocketFd());
	caller.sendMsg(":irc.server 341 " + caller.getNick() + " " + targetNick + " " + chanName + "\r\n");
	target->sendMsg(":" + caller.getNick() + "!" + caller.getUser()
		+ "@localhost INVITE " + targetNick + " " + chanName + "\r\n");
}
void  kickCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
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
	std::string reason = (args.size() >= 4) ? args[3] : caller.getNick();
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	Channel *chan = server.getChannel(chanName);
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
	Client *target = server.getClientByNick(targetNick);
	if (!target || !chan->hasMember(target->getSocketFd()))
	{
		caller.sendMsg(":irc.server 441 " + caller.getNick() + " " + targetNick + " " + chanName + " :They aren't on that channel\r\n");
		return ;
	}
	std::string kickMsg = ":" + caller.getNick() + "!" + caller.getUser() + "@localhost KICK " + chanName + " " + targetNick + " :" + reason + "\r\n";
	chan->broadcast(kickMsg, -1);
	chan->removeMember(target->getSocketFd());
}
void  modeCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	// Need at least: MODE <channel>
	if (args.size() < 2)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " MODE :Not enough parameters\r\n");
		return ;
	}

	const std::string &target = args[1];

	// Only channel MODE is handled (channels start with '#')
	if (target.empty() || target[0] != '#')
		return ;

	Channel *chan = server.getChannel(target);
	if (!chan)
	{
		caller.sendMsg(":irc.server 403 " + caller.getNick() + " " + target + " :No such channel\r\n");
		return ;
	}

	// No modestring: reply with RPL_CHANNELMODEIS (324)
	if (args.size() == 2)
	{
		std::string modes = "+";
		std::string modeArgs;
		if (chan->isInviteOnly())  modes += "i";
		if (chan->isTopicLocked()) modes += "t";
		if (chan->hasKey())   { modes += "k"; modeArgs += " " + chan->getKey(); }
		if (chan->hasLimit())
		{
			std::ostringstream oss;
			oss << chan->getLimit();
			modes += "l";
			modeArgs += " " + oss.str();
		}
		caller.sendMsg(":irc.server 324 " + caller.getNick() + " " + target + " " + modes + modeArgs + "\r\n");
		return ;
	}

	// Caller must be a member and an operator to change modes
	if (!chan->hasMember(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 442 " + caller.getNick() + " " + target + " :You're not on that channel\r\n");
		return ;
	}
	if (!chan->isOperator(caller.getSocketFd()))
	{
		caller.sendMsg(":irc.server 482 " + caller.getNick() + " " + target + " :You're not channel operator\r\n");
		return ;
	}

	const std::string	&modeStr = args[2];
	size_t				argIdx = 3;
	bool				adding = true;
	std::string			appliedStr;   // full modestring with +/- signs, e.g. "+i-o+kl"
	std::string			appliedArgs;  // space-separated arguments for the broadcast
	char				lastDir = 0;

	for (size_t i = 0; i < modeStr.size(); i++)
	{
		char c = modeStr[i];

		if (c == '+') { adding = true;  continue; }
		if (c == '-') { adding = false; continue; }

		char thisDir = adding ? '+' : '-';

		if (c == 'i')
		{
			chan->setInviteOnly(adding);
		}
		else if (c == 't')
		{
			chan->setTopicLocked(adding);
		}
		else if (c == 'k')
		{
			if (adding)
			{
				if (argIdx >= args.size())
				{
					caller.sendMsg(":irc.server 461 " + caller.getNick() + " MODE :Not enough parameters\r\n");
					continue ;
				}
				chan->setKey(args[argIdx]);
				appliedArgs += " " + args[argIdx++];
			}
			else
			{
				chan->clearKey();
				if (argIdx < args.size()) argIdx++; // consume the '*' some clients send
			}
		}
		else if (c == 'o')
		{
			if (argIdx >= args.size())
			{
				caller.sendMsg(":irc.server 461 " + caller.getNick() + " MODE :Not enough parameters\r\n");
				continue ;
			}
			std::string targetNick = args[argIdx++];
			Client *targetClient = server.getClientByNick(targetNick);
			if (!targetClient || !chan->hasMember(targetClient->getSocketFd()))
			{
				caller.sendMsg(":irc.server 441 " + caller.getNick() + " " + targetNick + " " + chan->getName() + " :They aren't on that channel\r\n");
				continue ;
			}
			if (adding)
				chan->addOperator(targetClient->getSocketFd());
			else
				chan->removeOperator(targetClient->getSocketFd());
			appliedArgs += " " + targetNick;
		}
		else if (c == 'l')
		{
			if (adding)
			{
				if (argIdx >= args.size())
				{
					caller.sendMsg(":irc.server 461 " + caller.getNick() + " MODE :Not enough parameters\r\n");
					continue ;
				}
				int limit = std::atoi(args[argIdx].c_str());
				if (limit <= 0) { argIdx++; continue ; }
				chan->setLimit(limit);
				appliedArgs += " " + args[argIdx++];
			}
			else
			{
				chan->clearLimit();
			}
		}
		else
		{
			caller.sendMsg(":irc.server 472 " + caller.getNick() + " " + std::string(1, c) + " :is unknown mode char to me\r\n");
			continue ;
		}

		// Append direction indicator to the output string only when it changes
		if (thisDir != lastDir) { appliedStr += thisDir; lastDir = thisDir; }
		appliedStr += c;
	}

	// Broadcast the mode change to all channel members
	if (!appliedStr.empty())
	{
		std::string notify = ":" + caller.getNick() + "!" + caller.getUser()
			+ "@irc.server MODE " + target + " " + appliedStr + appliedArgs + "\r\n";
		chan->broadcast(notify, -1);
	}
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
	if (!text.empty() && text[0] == ':')
		text.erase(0, 1);
	const std::string &target = args[1];
	std::string msg = ":" + caller.getNick() + "!" + caller.getUser()
		+ "@localhost PRIVMSG " + target + " :" + text + "\r\n";
	if (!target.empty() && target[0] == '#')
	{
		Channel *chan = server.getChannel(target);
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
		chan->broadcast(msg, caller.getSocketFd());
	}
	else
	{
		Client *dest = server.getClientByNick(target);
		if (!dest)
		{
			caller.sendMsg(":irc.server 401 " + caller.getNick() + " " + target + " :No such nick\r\n");
			return ;
		}
		dest->sendMsg(msg);
	}
}