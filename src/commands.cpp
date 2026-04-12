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