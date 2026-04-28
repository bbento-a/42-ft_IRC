#include "../inc/Server.hpp"

// Sends the 4 welcome numerics (001-004) after a client fully registers.
static void sendWelcome(Client &client)
{
	const std::string &nick = client.getNick();
	client.sendMsg(":irc.server 001 " + nick + " :Welcome to the IRC server " + nick + "!" + client.getUser() + "@" + client.getHost() + "\r\n");
	client.sendMsg(":irc.server 002 " + nick + " :Your host is irc.server, running version 1.0\r\n");
	client.sendMsg(":irc.server 003 " + nick + " :This server was created today\r\n");
	client.sendMsg(":irc.server 004 " + nick + " irc.server 1.0 itkol\r\n");
}
// ── PASS ─────────────────────────────────────────────────────────────────────

void  passCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	// Reject if the client is already registered.
	if (caller.isRegistered())
	{
		caller.sendMsg(":irc.server 462 " + caller.getNick() + " :You may not reregister\r\n");
		return ;
	}
	// PASS requires exactly one argument.
	if (args.size() < 2)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " PASS :Not enough parameters\r\n");
		return ;
	}
	// Validate the password against the server's configured password.
	if (!server.checkPassword(args[1]))
	{
		caller.sendMsg(":irc.server 464 " + caller.getNick() + " :Password incorrect\r\n");
		return ;
	}
	caller.setPassVerified(true);
	// If NICK and USER were already received before PASS, complete registration now.
	if (caller.isRegistered())
		sendWelcome(caller);
}

// ── NICK ─────────────────────────────────────────────────────────────────────

void  nickCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	char	first;

	// PASS must be verified before NICK can be accepted.
	if (!caller.isPassVerified())
	{
		caller.sendMsg(":irc.server 451 " + caller.getNick() + " :Send PASS first\r\n");
		return ;
	}
	// NICK requires a non-empty argument.
	if (args.size() < 2 || args[1].empty())
	{
		caller.sendMsg(":irc.server 431 " + caller.getNick() + " :No nickname given\r\n");
		return ;
	}
	const std::string &newNick = args[1];

	// Nick must not exceed 9 characters (IRC RFC 1459).
	if (newNick.size() > 9)
	{
		caller.sendMsg(":irc.server 432 " + caller.getNick() + " " + newNick + " :Erroneous nickname\r\n");
		return ;
	}
	// First character must be a letter, underscore, or dash.
	first = newNick[0];
	if (!std::isalpha(first) && first != '_' && first != '-')
	{
		caller.sendMsg(":irc.server 432 " + caller.getNick() + " " + newNick + " :Erroneous nickname\r\n");
		return ;
	}
	// Reject if another client already holds this nick.
	if (server.isNickInUse(newNick) && newNick != caller.getNick())
	{
		caller.sendMsg(":irc.server 433 " + caller.getNick() + " " + newNick + " :Nickname is already in use\r\n");
		return ;
	}
	caller.setNick(newNick);
	// If PASS and USER were already done, the client is now fully registered.
	if (caller.isRegistered())
		sendWelcome(caller);
}

// ── USER ─────────────────────────────────────────────────────────────────────

void  userCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	(void)server;
	// PASS must be verified before USER can be accepted.
	if (!caller.isPassVerified())
	{
		caller.sendMsg(":irc.server 451 " + caller.getNick() + " :Send PASS first\r\n");
		return ;
	}
	// USER can only be sent once per connection.
	if (caller.isUserSet())
	{
		caller.sendMsg(":irc.server 462 " + caller.getNick() + " :You may not reregister\r\n");
		return ;
	}
	// USER requires: username, mode, unused, realname (5 tokens total).
	if (args.size() < 5)
	{
		caller.sendMsg(":irc.server 461 " + caller.getNick() + " USER :Not enough parameters\r\n");
		return ;
	}
	caller.setUser(args[1]);
	// Strip the leading ':' from the realname field if present.
	std::string rname = args[4];
	if (!rname.empty() && rname[0] == ':')
		rname.erase(0, 1);
	caller.setRealname(rname);
	// If PASS and NICK were already done, the client is now fully registered.
	if (caller.isRegistered())
		sendWelcome(caller);
}

// ── QUIT ─────────────────────────────────────────────────────────────────────

void  quitCmd(Client &caller, Server &server, std::vector<std::string> &args)
{
	// Use the provided reason or fall back to a default message.
	std::string reason = (args.size() >= 2) ? args[1] : "Client quit";
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	// Broadcast the QUIT message to every channel the client is in.
	std::string quitMsg = ":" + caller.getNick() + "!" + caller.getUser() + "@" + caller.getHost() + " QUIT :" + reason + "\r\n";
	server.removeFromAllChannels(caller.getSocketFd(), quitMsg);
	// Notify the client that the connection is closing, then flag for cleanup.
	caller.sendMsg("ERROR :Closing connection\r\n");
	caller.setWantsQuit(true);
}
