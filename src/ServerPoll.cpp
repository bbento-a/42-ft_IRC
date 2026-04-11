#include "../inc/Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <iostream>

// Logic for having new wawawas:
// Create Client instance
// Create Client struct pollfd
// Accept connection
// Change settings of new client socket's to poll
// Add this new Client to the poll of clients
// Add this new Client to the Clients[]
void    Server::registerClient(void)
{
    int             fd;
    struct pollfd	newClientPoll;
    sockaddr_in		sockSettings;
    socklen_t       settingsLen = sizeof(sockSettings);

    fd = accept(this->_svEndpoint, reinterpret_cast<sockaddr *>(&sockSettings), &settingsLen);
    if (fd == -1)
    {   
		std::cerr << std::strerror(errno) << '\n';
            throw 'g'; //FailedtoCreateClientSocket
    }
    if (fcntl(fd, F_SETFL, O_NONBLOCK) <= -1)
        throw 'h'; // FailedtoTurnSocketNonBlocking
    newClientPoll.fd = fd;
    newClientPoll.events = POLLIN;
	newClientPoll.revents = 0;

    Client newClient(fd);
    this->_clients.insert(std::pair(fd, newClient));
    this->_clientsPoll.push_back(newClientPoll);
	this->_nbConnected++;
}

// Logic for departing wawawas:
// Clear Client instance from Clients[]
// Clear pollfd data associated to that Client
// Close Client's fd
void    Server::unregisterClient(pollfdIter clientInfo)
{
    //erase client from individual channels
    this->_clients.erase(clientInfo->fd);
    this->_clientsPoll.erase(clientInfo);
	this->_nbConnected--;
}

// Logic for receiving data from wawawas:
// Make buffer to read and store info
// Send received data for corresponded place
void    Server::handleClientData(Client clientInfo)
{
    // Make a buffer to store information from a client
    char buf[100];
    int  retCode = -1;
    retCode = recv(clientInfo.getSocketFd(), &buf, 100, 0);
    if (retCode == 0)
    {
        //unregister client
    }

    //  Store that information in a container
    //  Send that info to "corresponded place" in sv (send to commands)

    if (retCode <= -1)
        throw ;//FailedtoReceiveMsg
    else
    {
       //  (Testing server setup)
        for (pollfdIter it = _clientsPoll.begin(); it != _clientsPoll.end(); it++)
        {
            if (it->fd == this->_svEndpoint) // if I send it to the listening socket it will SIGPIPE
                continue ;
            retCode = send((*it).fd, buf, retCode, 0);
            if (retCode <= -1)
                throw 'i'; //FailedtoSendMsg
        }
    }
}

/* s clientIn
void    Server::handleClientData(pollfdIter clientInfo)
{
    char buf[100];
    int  retCode = -1;
    retCode = recv(clientInfo->fd, &buf, 100, 0);
    if (retCode <= -1)
        throw ;//FailedtoReceiveMsg
    else
    {
        for (pollfdIter it = _clientsPoll.begin(); it != _clientsPoll.end(); it++)
        {
            retCode = send(it->fd, buf, 100, 0);
            if (retCode <= -1)
                throw 'i';//FailedtoSendMsg
        }
    }
    //  Get info from a client, and store it
    //  Send that info to everyone in sv
    //  (Testing server setup)
}
*/