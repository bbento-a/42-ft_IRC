#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Client
{
	private:

	int			socketFd;
	std::string	buffer;
	std::string	nick;
	std::string	user;
	std::string	realname;
	std::string	host;
	bool		passVerified;
	bool		nickSet;
	bool		userSet;
	bool		_wantsQuit;

	public:

	Client(int fd);
	~Client();

	int			getSocketFd(void) const;

	void		setBuffer(const std::string &str);
	std::string	getBuffer(void) const;

	void		setNick(const std::string &n);
	std::string	getNick(void) const;

	void		setUser(const std::string &u);
	std::string	getUser(void) const;

	void		setRealname(const std::string &r);
	std::string	getRealname(void) const;

	void		setHost(const std::string &h);
	std::string	getHost(void) const;

	void		setPassVerified(bool val);
	bool		isPassVerified(void) const;

	void		setWantsQuit(bool val);
	bool		wantsQuit(void) const;

	bool		isUserSet(void) const;
	bool		isRegistered(void) const;

	void		sendMsg(const std::string &msg) const;
};

#endif