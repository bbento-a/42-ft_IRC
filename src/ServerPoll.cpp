#include "../inc/Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

// Logic for having new wawawas:
// Create Client instance
// Create Client struct pollfd
// Accept connection
// Change settings of new client's socket
// Add this new Client to the poll of clients
// Add this new Client to the Clients[]
void    Server::registerClient(void)
{
    struct pollfd	newClientPoll;
    sockaddr_in		sockSettings;
    Client			newClient;

	sockSettings.sin_addr.s_addr = AF_INET;
}

// Logic for departing wawawas:
// Clear Client instance from Clients[]
// Clear pollfd data associated to that Client
// Close Client's fd
void    Server::unregisterClient(struct pollfd *clientInfo)
{
    
}

// Logic for receiving data from wawawas:
// Make buffer to read and store info
// Send received data for corresponded place
void    Server::handleClientData(struct pollfd *clientInfo)
{

}