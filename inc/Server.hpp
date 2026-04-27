#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include <exception>
#include <map>
#include <poll.h>
#include <string>
#include <vector>


// ── TYPES ─────────────────────────────────────────────────────────────────────

typedef std::vector<struct pollfd>::iterator pollfdIter;

typedef	enum cmdsKeyword
{
	PASS,
	NICK,
	USER,
	QUIT,
	JOIN,
	PART,
	TOPIC,
	INVITE,
	KICK,
	MODE,
	PRIVMSG
} cmdsKeyword;

// ── SERVER ────────────────────────────────────────────────────────────────────

class Server
{
	// ── DATA ──────────────────────────────────────────────────────────────────
	private:

	std::string						_password;
	int								_serverPort;
	int								_svEndpoint;
	unsigned int					_nbConnected;

	std::vector<struct pollfd>      _clientsPoll;
	std::map<int, Client>           _clients;
	std::map<std::string, Channel>  _channels;

	// ── INTERFACE ─────────────────────────────────────────────────────────────
	public:

	// Lifecycle
	void		parseArguments(char *port, char *password);
	void		setup(char *port, char *password);
	void		runtime(void);

	// Client management
	void		registerClient(void);
	void		unregisterClient(pollfdIter clientInfo);
	void		handleClientData(pollfdIter it);
	void		handleData(Client &curClient);

	// Queries
	bool		checkPassword(const std::string &pass) const;
	bool		isNickInUse(const std::string &nick) const;
	Client		*getClientByNick(const std::string &nick);
	Channel		*getChannel(const std::string &name);
	Channel		&getOrCreateChannel(const std::string &name);

	// Channel management
	void  removeFromAllChannels(int fd, const std::string &quitMsg);
	void  pruneChannel(const std::string &name);

	// ── EXCEPTIONS ────────────────────────────────────────────────────────────
	class	InvalidPortNumber : public std::exception
	{ public: const char *what() const throw(); };

	class	PassEmpty : public std::exception
	{ public: const char *what() const throw(); };

	class	PassTooBig : public std::exception
	{ public: const char *what() const throw(); };

	class	InvalidPass : public std::exception
	{ public: const char *what() const throw(); };

	class	FailedtoCreateServerSocket : public std::exception
	{ public: const char *what() const throw(); };

	class	FailedtoSetSockSettings : public std::exception
	{ public: const char *what() const throw(); };

	class	FailedtoTurnSocketNonBlocking : public std::exception
	{ public: const char *what() const throw(); };

	class	FailedtoBindServerSock : public std::exception
	{ public: const char *what() const throw(); };

	class	FailedtoTurnListenSock : public std::exception
	{ public: const char *what() const throw(); };

	// class	PollFailedtoRetriveInfo : public std::exception
	// { public: const char *what() const throw(); };
	
};

// ── COMMAND HANDLERS ──────────────────────────────────────────────────────────
// Defined in commands.cpp and channelCmds.cpp

void  passCmd  (Client &caller, Server &server, std::vector<std::string> &args);
void  nickCmd  (Client &caller, Server &server, std::vector<std::string> &args);
void  userCmd  (Client &caller, Server &server, std::vector<std::string> &args);
void  quitCmd  (Client &caller, Server &server, std::vector<std::string> &args);
void  joinCmd  (Client &caller, Server &server, std::vector<std::string> &args);
void  partCmd  (Client &caller, Server &server, std::vector<std::string> &args);
void  topicCmd (Client &caller, Server &server, std::vector<std::string> &args);
void  inviteCmd(Client &caller, Server &server, std::vector<std::string> &args);
void  kickCmd  (Client &caller, Server &server, std::vector<std::string> &args);
void  modeCmd  (Client &caller, Server &server, std::vector<std::string> &args);
void  privmsgCmd(Client &caller, Server &server, std::vector<std::string> &args);

#endif