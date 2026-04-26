#include "../inc/Client.hpp"
#include <sys/socket.h>

Client::Client(int fd)
	: socketFd(fd), buffer(""), nick(""), user(""), realname(""),
	  passVerified(false), nickSet(false), userSet(false), _wantsQuit(false)
{}

int	Client::getSocketFd(void) const
{
	return socketFd;
}

void	Client::setBuffer(const std::string &str)
{
	buffer = str;
}

std::string	Client::getBuffer(void) const
{
	return buffer;
}

void	Client::setNick(const std::string &n)
{
	nick = n;
	nickSet = true;
}

std::string	Client::getNick(void) const
{
	return nick;
}

void	Client::setUser(const std::string &u)
{
	user = u;
	userSet = true;
}

std::string	Client::getUser(void) const
{
	return user;
}

void	Client::setRealname(const std::string &r)
{
	realname = r;
}

std::string	Client::getRealname(void) const
{
	return realname;
}

void	Client::setPassVerified(bool val)
{
	passVerified = val;
}

bool	Client::isPassVerified(void) const
{
	return passVerified;
}

void	Client::setWantsQuit(bool val)
{
	_wantsQuit = val;
}

bool	Client::wantsQuit(void) const
{
	return _wantsQuit;
}

// A client is fully registered once PASS was validated, NICK and USER were received.
bool	Client::isRegistered(void) const
{
	return (passVerified && nickSet && userSet);
}

// Sends a complete IRC message (caller is responsible for appending "\r\n").
// Convenience wrapper so nothing outside Client needs to know about socketFd.
// Enforces encapsulation: socketFd is private, so the only way to write to a
// client is through this method. If logic needs to be added (e.g. checking if
// the socket is still valid, logging outgoing messages, handling EAGAIN),
// there is one single place to change it.
void	Client::sendMsg(const std::string &msg) const
{
	send(socketFd, msg.c_str(), msg.size(), 0);
}
