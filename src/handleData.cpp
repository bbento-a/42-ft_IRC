#include "../inc/Server.hpp"

#include <sstream>
void	Server::handleData(Client curClient)
{
	// Parse data received from message	
	std::vector<std::string>   processedBuf;
	std::stringstream          procStream(curClient.getBuffer());
	std::string                curToken;
	while(std::getline(procStream, curToken, ' '))
	{
		processedBuf.push_back(curToken);
	}

	switch (curToken[0])
	{
		case PASS:

        	break;
		case NICK:

			break;
		case USER:

			break;
		case QUIT:

			break;
		case JOIN:

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
  Parameters: <username> 0 * <realname>

     Command: QUIT
 Parameters: [<reason>]



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