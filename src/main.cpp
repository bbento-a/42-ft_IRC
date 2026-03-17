#include "../inc/Server.hpp"
#include <exception>
#include <iostream>

// ./ircserv <port> <password>

int main (int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Wrong usage..." << '\n';
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
        std::cerr << e.what() << '\n';
    }
}