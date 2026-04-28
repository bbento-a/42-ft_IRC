*This project has been created as part of the 42 curriculum by bbento-a and joafaust*

# 42 FT_IRC

## — Description ————————

- Summary of the project
- Learning goals of this project
    - IRC Protocol
    - What is a Client-Server model
    - Sockets
    - Non-Blocking calls
    - Working with a IRC Client (Hexchat chosen)

### Summary of the project

In this project we had to create an IRC server, implement some commands for basic usage and use an official IRC Client to connect to our server to test its functionality like any other official IRC server.

### Learning goals of this project

#### RFC 1459 — IRC Protocol

The RFC 1459, known as Internet Relay Chat Protocol (IRC Protocol), was a very well-known Protocol for Internet chatting during the 90’s. It based on relaying messages between clients and servers, hence the name “Relay” on the Protocol, and it works mostly by a client-server model connection. One of its most known characteristics is that it has a good system concept in terms of privacy, since the protocol doesn’t require servers to keep clients’ data.

#### What is a Client-Server model

For this project we had to use a Client-Server model for establishing connections and communications, and this consists in system where various clients will request data from a server that will manage and redirect data according to those requests.

#### Sockets

You can see sockets as a type of file descriptor created for network and inter-process communications. A socket is composed by a communication domain and protocol, and a type, and is generated through socket().

#### Non-Blocking calls

Non-blocking system calls don’t block the current calling process, so this one can continue its execution, while the operation is still in progress. For example, in our project what happens is that clients can send messages while also receiving them from other clients.

#### Working with a IRC Client (Hexchat chosen)

For easier usage of the protocol, most people use IRC Clients, that are applications with an graphical interface that connect through networks that are already predefined for being frequently used (for example libera.chat), or through user-defined networks (that is what we did for this project). We used Hexchat by adding our server host’s address in a new network to be able to connect and use our server with an IRC Client.

## — Instructions ————————

### Compilation and installation

To prepare the project to run simply do:

```bash
$ make
```

To rebuild the project do:

```bash
$ make re
```

To clean the executable and objects from the project do:

```bash
$ make fclean
```

### Execution

To start up the IRC server, do this in the project’s executable directory:

```bash
$ ./ircserv <port> <pass>
```

- <port> is a number between 1024 to 65535
- <pass> is the password required to register as a client in the server

After that you should have the IRC server running in your computer

#### Using IRC through Console

To connect to the server through terminal, you can use:

```
$ nc -C <server's address> <port>
```

- netcat (nc) is a utility tool that uses TCP and UDP connections to read and write in a network
- <server's address> is the server host’s address
- <port> that the server is using to listen for connections

After this point you should be connected to the server. This is the following syntax for commands inserted through terminal:

```bash
CMD <arguments>
```

Once connected to the server, you have to register in the server to be able to use all functionalities available.

To register do:

- PASS <server’s password>

And these 2 commands, with the other being irrelevant to registration:

- NICK <nickname>
- USER <username> 0 * <realname>

After that you can do the following commands:
```
JOIN → Joins given channel
PART → Leaves given channel
TOPIC → Changes channel’s topic
INVITE → Invites a given user to a given channel
KICK → Kicks a given user from a given channel
MODE → Changes the channel’s configurations, with being these the available options:
           · i: Set/remove Invite-only channel
          · t: Set/remove the restrictions of the TOPIC command to channel
               operators
          · k: Set/remove the channel key (password)
          · o: Give/take channel operator privilege
          · l: Set/remove the user limit to channel
PRIVMSG → Sends a message to a given recipient (channel/user)
QUIT → Leaves server
```

Here’s the parameters each command can or must take:
```
JOIN → <channel>{,<channel>} [<key>{,<key>}]
PART → <channel>{,<channel>} [<reason>]
TOPIC → <channel> [<topic>]
INVITE → <nickname> <channel>
KICK → <channel> <user> [<comment>]
MODE → <target> [<modestring> [<mode arguments>...]]
PRIVMSG → <target>{,<target>} <text to be sent>
QUIT → [<reason>]
```



## — Resources —————————

Here are listed all resources that helped us understand concepts and build this project from scratch:

### Official documentation

- [https://datatracker.ietf.(╭ರ_•́)org/doc/html/rfc1459](https://datatracker.ietf.org/doc/html/rfc1459) -> Internet Relay Chat Protocol Documentation
- https://modern.ircdocs.horse/ -> Modern interface of the official documentation

### Blogs and Community Posts

- https://www.geeksforgeeks.org/computer-networks/internet-relay-chat-irc/
- https://www.geeksforgeeks.org/computer-networks/chat-conferencing-protocols/
- https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9 -> From a 42 Student
- https://medium.com/@jayzalani34/building-an-http-server-from-scratch-using-tcp-sockets-in-c-10f5f8479fb9
- https://deepwiki.com/smol-rs/polling/4.1-tcp-server-example
- http://chi.cs.uchicago.edu/chirc/irc_examples.html -> great examples if you need visual representation to understand better concepts
- https://www.geeksforgeeks.org/computer-networks/rfc-request-for-comment/ -> explanation of RFCs
- https://www.geeksforgeeks.org/system-design/client-server-model/
- https://sachintolay.substack.com/p/blocking-vs-non-blocking-vs-asynchronous
- https://linuxvox.com/blog/socket-descriptor-vs-file-descriptor/

### Videos

- I/O multiplexing — ****https://youtu.be/dEHZb9JsmOU?si
- Poll function — https://youtu.be/O-yMs3T0APU?si
- Creating a TCP Server — https://www.youtube.com/watch?v=cNdlrbZSkyQ

And of course, some of our peers from 42 Lisboa :)