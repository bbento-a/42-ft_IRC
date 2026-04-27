#include "../inc/Client.hpp"
#include <sys/socket.h>
#include <unistd.h>

// Initialises all fields to safe defaults; flags start false until each IRC command is received.
Client::Client(int fd)
	: socketFd(fd), buffer(""), nick(""), user(""), realname(""), host(""),
	  passVerified(false), nickSet(false), userSet(false), _wantsQuit(false)
{}

// Closes the socket so the OS releases the file descriptor.
Client::~Client()
{
	close(this->socketFd);
}

int	Client::getSocketFd(void) const
{
	return socketFd;
}

// The buffer accumulates raw bytes from recv() until a full IRC line is ready.
void	Client::setBuffer(const std::string &str)
{
	buffer = str;
}

std::string	Client::getBuffer(void) const
{
	return buffer;
}

// Sets the nick and raises the flag used by isRegistered().
void	Client::setNick(const std::string &n)
{
	nick = n;
	nickSet = true;
}

std::string	Client::getNick(void) const
{
	return nick;
}

// Sets the username and raises the flag used by isRegistered().
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

void	Client::setHost(const std::string &h)
{
	host = h;
}

std::string	Client::getHost(void) const
{
	return host;
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

bool	Client::isUserSet(void) const
{
	return userSet;
}

// A client is fully registered once PASS was validated, NICK and USER were received.
bool	Client::isRegistered(void) const
{
	return (passVerified && nickSet && userSet);
}

// Sends a raw IRC message to the client's socket.
void	Client::sendMsg(const std::string &msg) const
{
	send(socketFd, msg.c_str(), msg.size(), 0);
}
