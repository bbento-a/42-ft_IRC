#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <exception>
#include <poll.h>
#include <string>
#include <vector>

typedef typename std::vector<struct pollfd>::iterator pollfdIter;

// Notes from metro thinking time aahh (it means this needs reviewing)


class Server
{
	private:
	std::string		_password;
	int				_serverPort;
	int				_svEndpoint;
	unsigned int	_nbConnected;

	std::vector <struct pollfd>	_clientsPoll;
	std::vector <Client>		_clients; //connected clients to the sv
	//Channel						_channels[]; //existing channels at sv
	
	public:
	//OCF
	void    parseArguments(char *port, char *password);
	void	setup(char *port, char *password); // check everything for start up
	void	runtime(void); //loop of connections
	void	registerClient(void); //check client infos to link them to the sv
	void    unregisterClient(pollfdIter clientInfo);
	void    handleClientData(struct pollfd *clientInfo);

	class	InvalidPortNumber : public std::exception
	{ public: const char *what() const throw();	};

	class	PassEmpty : public std::exception
	{
		public:
		const char *what() const throw();
	};
	class	PassTooBig : public std::exception
	{
		public:
		const char *what() const throw();
	};
	class	InvalidPass : public std::exception
	{
		public:
		const char *what() const throw();
	};
	class	FailedtoCreateServerSocket : public std::exception
	{
		public:
		const char *what() const throw();
	};
	class	FailedtoSetSockSettings : public std::exception
	{
		public:
		const char *what() const throw();
	};
	class	FailedtoTurnSocketNonBlocking : public std::exception
	{
		public:
		const char *what() const throw();
	};
	class	FailedtoBindServerSock : public std::exception
	{
		public:
		const char *what() const throw();
	};
	class	FailedtoTurnListenSock : public std::exception
	{
		public:
		const char *what() const throw();
	};
	class	PollFailedtoRetrieveInfo : public std::exception
	{
		public:
		const char *what() const throw();
	};
};


#endif