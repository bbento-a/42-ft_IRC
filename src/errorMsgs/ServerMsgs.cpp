#include "../../inc/Server.hpp"

const char *Server::InvalidPortNumber::what() const throw()
{ return ("Invalid port number for server"); }

const char *Server::PassEmpty::what() const throw()
{ return ("Password for server is empty"); }

const char *Server::PassTooBig::what() const throw()
{ return ("Password for server has exceeded 100 characters"); }

const char *Server::InvalidPass::what() const throw()
{ return ("Password for server contains invalid characters"); }

const char *Server::FailedtoCreateServerSocket::what() const throw()
{ return ("Failed to create listening socket for server"); }

const char *Server::FailedtoSetSockSettings::what() const throw()
{ return ("Failed to set socket settings"); }

const char *Server::FailedtoTurnSocketNonBlocking::what() const throw()
{ return ("Failed to turn socket non blocking"); }

const char *Server::FailedtoBindServerSock::what() const throw()
{ return ("Failed to assign address to the listening socket"); }

const char *Server::FailedtoTurnListenSock::what() const throw()
{ return ("Failed to turn the socket passive"); }

const char *Server::PollFailedtoRetrieveInfo::what() const throw()
{ return ("Poll failed to retrive information"); }

