#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	private:

	int			socketFd;
	std::string	buffer;
	std::string	nick;
	std::string	user;
	bool		passVerified;
	bool		nickSet;
	bool		userSet;

	public:

	Client(int fd);

	int			getSocketFd(void) const;

	void		setBuffer(const std::string &str);
	std::string	getBuffer(void) const;

	void		setNick(const std::string &n);
	std::string	getNick(void) const;

	void		setUser(const std::string &u);
	std::string	getUser(void) const;

	void		setPassVerified(bool val);
	bool		isPassVerified(void) const;

	bool		isRegistered(void) const;

	void		sendMsg(const std::string &msg) const;
};

#endif