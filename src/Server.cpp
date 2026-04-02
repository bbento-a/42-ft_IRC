#include "../inc/Server.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void    Server::parseArguments(char *port, char *password)
{
    // check if port is within the correct ports to use
        // if not, throw
    // store infos in sv
}
    
void	Server::setup(char *port, char *password)
{
    // parse arguments
    // creates socket for server's endpoint
}

void	Server::runtime()
{
    // Creating a listening (passive) socket to receive new connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket <= -1)
        throw ; // FailedtoCreate listening socket
    
    // This is going to be the server's binding info
    // sockaddr_in is used for IPv4 
    sockaddr_in sockSettings;
    sockSettings.sin_family = AF_INET;
    sockSettings.sin_port = htons(5050);
    sockSettings.sin_addr.s_addr = INADDR_ANY;
    // htons() is used to change the bits order to big endian, which is the order used in networking
    // htons -> "host to network short"

    // Doing the binding (assigning address to socket)
    if (bind(listenSocket, reinterpret_cast<sockaddr *>(&sockSettings), sizeof(sockSettings)) <= -1)
        throw ; // FailedtoBind listening socket

    // Listen will "turn the socket passive"
    if (listen(listenSocket, SOMAXCONN) <= -1)
        throw ; // FailedtoListen listening socket
    // Loop
}