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
*/

static void sendWelcome(Client &client)
{
	const std::string &nick = client.getNick();
	client.sendMsg(":irc.server 001 " + nick + " :Welcome to the IRC server " + nick + "!" + client.getUser() + "@" + client.getHost() + "\r\n");
	client.sendMsg(":irc.server 002 " + nick + " :Your host is irc.server, running version 1.0\r\n");
	client.sendMsg(":irc.server 003 " + nick + " :This server was created today\r\n");
	client.sendMsg(":irc.server 004 " + nick + " irc.server 1.0 o itkol\r\n");
}
// ── PASS ─────────────────────────────────────────────────────────────────────

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

// ── NICK ─────────────────────────────────────────────────────────────────────

void  nickCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	char	first;
	
	if (args.size() < 2 || args[1].empty())
	{
		caller.sendMsg(":irc.server 431 " + caller.getNick() + " :No nickname given\r\n");
		return ;
	}
	const std::string &newNick = args[1];

	first = newNick[0];
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
// ── USER ─────────────────────────────────────────────────────────────────────

void  userCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	(void)server;
	if (caller.isUserSet())
	{
		caller.sendMsg(":irc.server 462 " + caller.getNick() + " :You may not reregister\r\n");
		return ;
	}
	if (args.size() < 5)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " USER :Not enough parameters\r\n");
		return ;
	}
	caller.setUser(args[1]);
	std::string rname = args[4];
	if (!rname.empty() && rname[0] == ':')
		rname.erase(0, 1);
	caller.setRealname(rname);
	if (caller.isRegistered())
		sendWelcome(caller);
}

// ── QUIT ─────────────────────────────────────────────────────────────────────

void  quitCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	std::string reason = (args.size() >= 2) ? args[1] : "Client quit";
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	std::string quitMsg = ":" + caller.getNick() + "!" + caller.getUser() + "@" + caller.getHost() + " QUIT :" + reason + "\r\n";
	server.removeFromAllChannels(caller.getSocketFd(), quitMsg);
	caller.sendMsg("ERROR :Closing connection\r\n");
	caller.setWantsQuit(true);
}
