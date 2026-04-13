#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>


//:)

class Client
{
	private:

	int			socketFd;
	std::string	buffer;
	std::string	nick;
	std::string	user;
	//	realname;
	//	isAuthenticated;
	

	public:

	Client(int fd);
	int			getSocketFd(void) const;
	void		setBuffer(std::string str);
	std::string	getBuffer(void) const;
};

#endif