#ifndef SERVER_HPP
#define SERVER_HPP

#include <exception>
#include <poll.h>
#include <vector>

// Notes from metro thinking time aahh (it means this needs reviewing)
class Client{};

class Server
{
	private:
	int		serverPort;
	int		password;
	int		svEndpoint;

	std::vector <struct pollfd>	clientsPoll;
	Client						Clients; //connected clients to the sv
	Channel						Channels[]; //existing channels at sv
	
	public:
	//OCF
	void    parseArguments(char *port, char *password);
	void	setup(char *port, char *password); // check everything for start up
	void	runtime(); //loop of connections
	void	registerClient(); //check client infos to link them to the sv
};

class Channel
{
	private:
	Client Clients; // these are the ones connected to the channel
	// should the ops be a derived from client?
	//sm more important cmds (like ban, kick)
	//most cmds (like join, privmsg)

	public:
	//OCF
};


#endif