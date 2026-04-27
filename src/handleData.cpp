#include "../inc/Server.hpp"

// Maps a raw command string to the cmdsKeyword enum. Returns -1 if unknown.
static int	parseCmd(const std::string &cmd)
{
	if (cmd == "PASS")
		return PASS;
	if (cmd == "NICK")
		return NICK;
	if (cmd == "USER")
		return USER;
	if (cmd == "QUIT")
		return QUIT;
	if (cmd == "JOIN")
		return JOIN;
	if (cmd == "PART")
		return PART;
	if (cmd == "TOPIC")
		return TOPIC;
	if (cmd == "INVITE")
		return INVITE;
	if (cmd == "KICK")
		return KICK;
	if (cmd == "MODE")
		return MODE;
	if (cmd == "PRIVMSG")
		return PRIVMSG;
	return -1;
}

// Dispatches a parsed command to the appropriate handler.
static void	dispatchCmd(Client &curClient, Server &server, std::vector<std::string> &args)
{
	switch (parseCmd(args[0]))
	{
		case PASS:   passCmd(curClient, server, args);    break;
		case NICK:   nickCmd(curClient, server, args);    break;
		case USER:   userCmd(curClient, server, args);    break;
		case QUIT:   quitCmd(curClient, server, args);    break;
		case JOIN:   joinCmd(curClient, server, args);    break;
		case PART:   partCmd(curClient, server, args);    break;
		case TOPIC:  topicCmd(curClient, server, args);   break;
		case INVITE: inviteCmd(curClient, server, args);  break;
		case KICK:   kickCmd(curClient, server, args);    break;
		case MODE:   modeCmd(curClient, server, args);    break;
		case PRIVMSG: privmsgCmd(curClient, server, args); break;
		default:     break;
	}
}

void	Server::handleData(Client &curClient)
{
	std::string	buffer = curClient.getBuffer();
	curClient.setBuffer("");

	std::stringstream	bufStream(buffer);
	std::string			line;

	while (std::getline(bufStream, line))
	{
		// Tokenise the line into command + arguments.
		std::vector<std::string>	processedBuf;
		std::stringstream			procStream(line);
		std::string					curToken;

		while (std::getline(procStream, curToken, ' '))
		{
			// A token starting with ':' consumes the rest of the line (trailing param).
			if (!curToken.empty() && curToken[0] == ':')
			{
				std::string tmp;
				std::getline(procStream, tmp);
				curToken += tmp;
			}
			if (!curToken.empty())
				processedBuf.push_back(curToken);
		}
		if (processedBuf.empty())
			continue;
		// Strip trailing \r\n from the last token.
		std::string &last = processedBuf.back();
		while (!last.empty() && (last[last.size() - 1] == '\n' || last[last.size() - 1] == '\r'))
			last.erase(last.size() - 1);
		if (last.empty())
			processedBuf.pop_back();
		if (processedBuf.empty())
			continue;

		dispatchCmd(curClient, *this, processedBuf);
	}
}
