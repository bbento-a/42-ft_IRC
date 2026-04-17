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

void  passCmd(Client, std::vector<std::string>)
{
   
}
void  nickCmd(Client, std::vector<std::string>)
{

}
void  userCmd(Client, std::vector<std::string>)
{

}
void  quitCmd(Client, std::vector<std::string>)
{

}
void  joinCmd(Client, std::vector<std::string>)
{

}
void  partCmd(Client, std::vector<std::string>)
{

}
void  topicCmd(Client, std::vector<std::string>)
{

}
void  inviteCmd(Client, std::vector<std::string>)
{

}
void  kickCmd(Client, std::vector<std::string>)
{

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
void  privmsgCmd(Client, std::vector<std::string>)
{

}