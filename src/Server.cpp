#include "../inc/Server.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
// #include <arpa/inet.h>
#include <fcntl.h>

void    Server::parseArguments(char *port, char *password)
{
    // check if port is within the correct ports to use
        // if not, throw
    // store infos in sv
}


// In setup() we're creating a listening socket for the server - the endpoint
// for clients to connect, with the defined settings asked for the project.
    
void	Server::setup(char *port, char *password)
{
    // parse arguments
		// port nb
		// password


    // creates socket for server's endpoint
    // Creating a listening (passive) socket to receive new connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket <= -1)
        throw ; // FailedtoCreate listening socket


	// ↓ Settings that are required to set before turn the socket passive ↓


    // This is going to be the server's binding info
    // sockaddr_in is used for IPv4 
    sockaddr_in sockSettings;
    sockSettings.sin_family = AF_INET;
    sockSettings.sin_port = htons(5050); // add port that was given in args
    sockSettings.sin_addr.s_addr = INADDR_ANY;
    // htons() is used to change the bits order to big endian, which is the order used in networking
    // htons -> "host to network short"

	// Setting the socket to be able to reuse the address/port,
	// without waiting for TIME_WAIT (default time in TCP for when a server stops running)
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, NULL, sizeof(sockSettings)) <= -1)
        throw ; // FailedtoSetSockSettings
	
	// Setting the socket to be non-blocking - as asked in the subject
	if (fcntl(listenSocket, F_SETFL, O_NONBLOCK) <= -1)
        throw ; // FailedtoTurnSocketNonBlocking


	// ↓ Here is the part we link and connect the socket to become a listening asset ↓

    // Doing the binding (assigning address to socket)
    if (bind(listenSocket, reinterpret_cast<sockaddr *>(&sockSettings), sizeof(sockSettings)) <= -1)
        throw ; // FailedtoBind listening socket

    // Listen will "turn the socket passive"
    if (listen(listenSocket, SOMAXCONN) <= -1)
        throw ; // FailedtoListen listening socket

	this->svEndpoint = listenSocket;
}

void	Server::runtime()
{
    // Loop
	while (1)
	{

	}
}