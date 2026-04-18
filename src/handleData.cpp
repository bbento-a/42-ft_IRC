#include "../inc/Server.hpp"

#include <sstream>

// Forward declarations - defined in commands.cpp
void	passCmd(Client &caller, Server &server, std::vector<std::string> &args);
void	nickCmd(Client &caller, Server &server, std::vector<std::string> &args);
void	userCmd(Client &caller, Server &server, std::vector<std::string> &args);
void	joinCmd(Client &caller, Server &server, std::vector<std::string> &args);
void	modeCmd(Client &caller, Server &server, std::vector<std::string> &args);

// Maps a raw command string to the cmdsKeyword enum. Returns -1 if unknown.
static int	parseCmd(const std::string &cmd)
{
	if (cmd == "PASS")    return PASS;
	if (cmd == "NICK")    return NICK;
	if (cmd == "USER")    return USER;
	if (cmd == "QUIT")    return QUIT;
	if (cmd == "JOIN")    return JOIN;
	if (cmd == "PART")    return PART;
	if (cmd == "TOPIC")   return TOPIC;
	if (cmd == "INVITE")  return INVITE;
	if (cmd == "KICK")    return KICK;
	if (cmd == "MODE")    return MODE;
	if (cmd == "PRIVMSG") return PRIVMSG;
	return -1;
}

void	Server::handleData(Client &curClient)
{
	// Parse data received from message	
	std::vector<std::string>   processedBuf;
	std::stringstream          procStream(curClient.getBuffer());
	std::string                curToken;
	curClient.setBuffer("");
	while(std::getline(procStream, curToken, ' '))
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
	// Case 1: recv gave us nothing at all - vector is empty from the start
	if (processedBuf.empty())
		return ; // Might need to handle in a different way
	// Strip trailing \r\n from the last token (raw recv data)
	std::string &last = processedBuf.back();
	while(!last.empty() && (last[last.size() - 1] == '\n' || last[last.size() - 1] == '\r'))
		last.erase(last.size() - 1);
	if (last.empty())
		processedBuf.pop_back();
	// Case 2: vector had exactly one token which was only "\r\n" - now empty after pop :))
	if (processedBuf.empty())
		return ;
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

			break;
		case JOIN:
			joinCmd(curClient, *this, processedBuf);
			break;
		case PART:

			break;
		case TOPIC:

			break;
		case INVITE:

			break;
		case KICK:

			break;
		case MODE:
			modeCmd(curClient, *this, processedBuf);
			break;
		case PRIVMSG:

			break;
      
      default:
	  	// invalid token
			break;
   }
   // Send that info to "corresponded place" in sv (send to commands)
	// Send corresponding success/error message according to IRC syntax of logs

}


/* 

	optional parts or parameters are noted with square brackets as such: "[<param>]"
	Curly braces around a part of parameter indicate that it may be repeated zero or more times
	"<key>{,<key>}" indicates that there must be at least one <key>

     Command: PASS
  Parameters: <password>

     Command: NICK
  Parameters: <nickname>

     Command: USER
  Parameters: <username> 0 * : <realname>

     Command: QUIT
 Parameters: : [<reason>]



     Command: JOIN
  Parameters: <channel>{,<channel>} [<key>{,<key>}]
  Alt Params: 0

     Command: PART
  Parameters: <channel>{,<channel>} : [<reason>]

     Command: TOPIC
  Parameters: <channel> : [<topic>]

     Command: INVITE
  Parameters: <nickname> <channel>

      Command: KICK
   Parameters: <channel> <user> *( "," <user> ) : [<comment>]

     Command: MODE
  Parameters: <target> [<modestring> [<mode arguments>...]]


  
       Command: PRIVMSG
  Parameters: <target>{,<target>} : <text to be sent>
*/