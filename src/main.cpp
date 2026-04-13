#include "../inc/Server.hpp"
#include <exception>
#include <iostream>

// ./ircserv <port> <password>

int main (int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Number of arguments for usage incorrect" << '\n';
        std::cout << "Try ./IRC <port> <password>" << '\n';
        return (1);
    }

    Server server;

    try
    {
        server.setup(argv[1], argv[2]);
        server.runtime();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
}