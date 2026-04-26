#include "../inc/Server.hpp"

#include <sstream>
#include <iostream>

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

void	Server::handleData(Client &curClient)
{
	std::string	buffer = curClient.getBuffer();
	curClient.setBuffer("");

	std::stringstream	bufStream(buffer);
	std::string			line;

	while (std::getline(bufStream, line))
	{
		// Parse data received from message
		std::vector<std::string>	processedBuf;
		std::stringstream			procStream(line);
		std::string					curToken;

		while (std::getline(procStream, curToken, ' '))
		{
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
		// Strip trailing \r from the last token
		std::string &last = processedBuf.back();
		while (!last.empty() && (last[last.size() - 1] == '\n' || last[last.size() - 1] == '\r'))
			last.erase(last.size() - 1);
		if (last.empty())
			processedBuf.pop_back();
		if (processedBuf.empty())
			continue;

		for (size_t i = 0; i < processedBuf.size(); i++)
			std::cout << processedBuf[i];
		std::cout << '\n';

		switch (parseCmd(processedBuf[0]))
		{
			case PASS:
				passCmd(curClient, *this, processedBuf);
				break;
			case NICK:
				nickCmd(curClient, *this, processedBuf);
				break;
			case USER:
				userCmd(curClient, *this, processedBuf);
				break;
			case QUIT:
				quitCmd(curClient, *this, processedBuf);
				break;
			case JOIN:
				joinCmd(curClient, *this, processedBuf);
				break;
			case PART:
				partCmd(curClient, *this, processedBuf);
				break;
			case TOPIC:
				topicCmd(curClient, *this, processedBuf);
				break;
			case INVITE:
				inviteCmd(curClient, *this, processedBuf);
				break;
			case KICK:
				kickCmd(curClient, *this, processedBuf);
				break;
			case MODE:
				modeCmd(curClient, *this, processedBuf);
				break;
			case PRIVMSG:
				privmsgCmd(curClient, *this, processedBuf);
				break;
			default:
				break;
		}
	}
}
