#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"

class Channel
{
	private:
	Client Clients; // these are the ones connected to the channel
	// should the ops be a derived from client?
	//sm more important cmds (like ban, kick)
	//most cmds (like join, privmsg)

	public:
	//OCF
};
#endif