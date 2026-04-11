/* 
my source for cmds: 
https://modern.ircdocs.horse/#connection-messages


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