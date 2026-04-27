#include "../inc/Server.hpp"

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
        return ;
    if (fcntl(fd, F_SETFL, O_NONBLOCK) <= -1)
    {
        close(fd);
        return ;
    }
    newClientPoll.fd = fd;
    newClientPoll.events = POLLIN;
	newClientPoll.revents = 0;

    Client newClient(fd);
    newClient.setHost(inet_ntoa(sockSettings.sin_addr));
    this->_clients.insert(std::make_pair(fd, newClient));
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
	std::cout << "Disconnected user"<< '\n';
    close(clientInfo->fd);
    this->_clients.erase(clientInfo->fd);
    this->_clientsPoll.erase(clientInfo);
	this->_nbConnected--;
}

// Logic for receiving data from wawawas:
// Make buffer to read and store info
// Send received data for corresponded place

void    Server::handleClientData(pollfdIter it)
{
    Client *clientInfo = &_clients.at(it->fd);
    // Make a buffer to store information from a client
    // Store that information
    char    buf[513];
    int     retCode = 513;

    std::memset(buf, '\0', retCode);
    retCode = recv(clientInfo->getSocketFd(), &buf, 512, 0);

    if (retCode <= -1)
        return ;
    else if (retCode == 0) // client closed the connection gracefully
    {
        unregisterClient(it);
        return ;
    }

    std::string tmp = clientInfo->getBuffer();
    tmp += buf;
    clientInfo->setBuffer(tmp);

    if (clientInfo->getBuffer().find('\n') == std::string::npos)
    {
        return ;
        // because if you receive and EOF without newline, we don't want
        // the server to process that data right in this moment,
        // we just want it to store for the next time it receives
        // more information from the client that sent the later data mentioned
    }
    else // handle data
    {
        handleData(*clientInfo);
    }
}
