#ifndef SERVER_HPP
#define SERVER_HPP

#include <exception>
#include <poll.h>
#include <vector>

typedef typename std::vector<struct pollfd>::iterator pollfdIter;

// Notes from metro thinking time aahh (it means this needs reviewing)
class Client
{
	public:
	int socketFd;
};

class Server
{
	private:
	int		_serverPort;
	int		_password;
	int		_svEndpoint;

	std::vector <struct pollfd>	_clientsPoll;
	std::vector <Client>		_clients; //connected clients to the sv
	Channel						_channels[]; //existing channels at sv
	
	public:
	//OCF
	void    parseArguments(char *port, char *password);
	void	setup(char *port, char *password); // check everything for start up
	void	runtime(void); //loop of connections
	void	registerClient(void); //check client infos to link them to the sv
	void    unregisterClient(pollfdIter clientInfo);
	void    handleClientData(pollfdIter clientInfo);
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