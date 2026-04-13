#include "../inc/Server.hpp"
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
// #include <arpa/inet.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <iostream>

void    Server::parseArguments(char *port, char *password)
{
	long parsePort = std::atol(port);
	if (parsePort < 1024 || parsePort > 65535) // because ports are stored in 16-bit unsigned int
		throw InvalidPortNumber();
	this->_serverPort = static_cast<int>(parsePort);

	std::string passParsed(password);
	if (passParsed.empty())
		throw PassEmpty();
	else if (passParsed.size() > 100)
		throw PassTooBig();
	for (size_t i = 0; i < passParsed.length(); i++)
		if (!std::isprint(passParsed[i]))
			throw InvalidPass();
	this->_password = passParsed;
}


// In setup() we're creating a listening socket for the server - the endpoint
// for clients to connect, with the defined settings asked for the project.
    
void	Server::setup(char *port, char *password)
{
	this->_nbConnected = 0;
	parseArguments(port, password);

    // creates socket for server's endpoint
    // Creating a listening (passive) socket to receive new connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket <= -1)
        throw FailedtoCreateServerSocket();


	// ↓ Settings that are required to set before turn the socket passive ↓


    // This is going to be the server's binding info
    // sockaddr_in is used for IPv4 
    sockaddr_in sockSettings;
    sockSettings.sin_family = AF_INET;
    sockSettings.sin_port = htons(this->_serverPort); // add port that was given in args
    sockSettings.sin_addr.s_addr = INADDR_ANY;
    // htons() is used to change the bits order to big endian, which is the order used in networking
    // htons -> "host to network short"
	int optVal = 1; // to turn optname on
	// Setting the socket to be able to reuse the address/port,
	// without waiting for TIME_WAIT (default time in TCP for when a server stops running)
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) <= -1)
	    throw FailedtoSetSockSettings();
	
	// Setting the socket to be non-blocking - as asked in the subject
	if (fcntl(listenSocket, F_SETFL, O_NONBLOCK) <= -1)
        throw FailedtoTurnSocketNonBlocking();


	// ↓ Here is the part we link and connect the socket to become a listening asset ↓

    // Doing the binding (assigning address to socket)
    if (bind(listenSocket, reinterpret_cast<sockaddr *>(&sockSettings), sizeof(sockSettings)) <= -1)
        throw FailedtoBindServerSock(); // FailedtoBind listening socket

    // Listen will "turn the socket passive"
    if (listen(listenSocket, SOMAXCONN) <= -1)
		throw FailedtoTurnListenSock();

	this->_svEndpoint = listenSocket;
	struct pollfd addToPoll;
	addToPoll.fd = listenSocket;
	addToPoll.events = POLLIN;
	addToPoll.revents = 0;
	this->_clientsPoll.push_back(addToPoll);
	this->_nbConnected++;
}

void	Server::runtime()
{
	while (1)
	{
		// using poll to identify if there is new data from the client connections
		// and if there is poll will edit the given struct pollfd * variable, so when
		// we iter it, we can act according to the new data that was found
		if (poll(&_clientsPoll[0], _clientsPoll.size(), -1) <= -1)
			throw PollFailedtoRetrieveInfo();

		try
		{
			for (unsigned int i = 0; i < _nbConnected; i++)
			{
				// for legibility
				short	cEvent = _clientsPoll[i].revents;
				int		cFd = _clientsPoll[i].fd;
		
				if ((cEvent & POLLERR) || (cEvent & POLLNVAL) || (cEvent & POLLHUP)) // disconnect error, invalid or hung up connections
				{
					unregisterClient(_clientsPoll.begin() + i);
					i--;
				}
				else if (cFd == this->_svEndpoint && (cEvent & POLLIN)) // accept new connections and add to the poll
				{
					std::cout << "Connected user"<< '\n';
					registerClient();
				}
				else if (cEvent & POLLIN) // new read input data
				{
					handleClientData(_clients.at(cFd));
				}
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
		

	}
}