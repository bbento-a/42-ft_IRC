#include "../inc/Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

// Logic for having new wawawas:
// Create Client instance
// Create Client struct pollfd
// Accept connection
// Change settings of new client socket's to poll
// Add this new Client to the poll of clients
// Add this new Client to the Clients[]
void    Server::registerClient(void)
{
    struct pollfd	newClientPoll;
    sockaddr_in		sockSettings;
    Client			newClient;
    socklen_t       settingsLen = sizeof(sockSettings);

    newClient.socketFd = accept(this->_svEndpoint, reinterpret_cast<sockaddr *>(&sockSettings), &settingsLen);
    if (newClient.socketFd == -1)
        throw ; //FailedtoCreateClientSocket
    if (fcntl(newClient.socketFd, F_SETFL, O_NONBLOCK) <= -1)
        throw ; // FailedtoTurnSocketNonBlocking
    newClientPoll.fd = newClient.socketFd;
    newClientPoll.events = POLLIN;
	newClientPoll.revents = 0;

    this->_clientsPoll.push_back(newClientPoll);
    this->_clients.push_back(newClient);
}

// Logic for departing wawawas:
// Clear Client instance from Clients[]
// Clear pollfd data associated to that Client
// Close Client's fd
void    Server::unregisterClient(pollfdIter clientInfo)
{
    std::vector<Client>::iterator it = this->_clients.begin();
    while (it != this->_clients.end() && clientInfo->fd != it->socketFd)
        it++;
    if (it == this->_clients.end())
        return ;
    this->_clients.erase(it);
    this->_clientsPoll.erase(clientInfo);
}

// Logic for receiving data from wawawas:
// Make buffer to read and store info
// Send received data for corresponded place
void    Server::handleClientData(pollfdIter clientInfo)
{

}