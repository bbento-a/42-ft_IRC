#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client
{
	private:

	int			socketFd;
	std::string	buffer;
	std::string	nick;
	std::string	user;

	public:

	Client(int fd);
	int	getSocketFd(void);
};

#endif