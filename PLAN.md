# IRC Server Architecture Explanation & Client Class Design

## Current Implementation Overview

### 1. **Server Class** (inc/Server.hpp, src/Server.cpp)
The core IRC server that manages:
- **Socket management**: Creates and configures a non-blocking TCP listening socket
- **Client connections**: Stores connected clients using `std::vector<pollfd>` for poll-based I/O multiplexing
- **Channel management**: Placeholder for managing IRC channels
- **Authentication**: Port and password-based server access

**Key methods:**
- `parseArguments()` - Validates and stores port/password
- `setup()` - Creates the listening socket with:
  - Non-blocking mode (O_NONBLOCK)
  - Address reuse (SO_REUSEADDR)
  - IPv4 binding (INADDR_ANY)
- `runtime()` - Main event loop (currently empty, needs poll() logic)
- `connectClient()` - New connection handler (accepts connection, creates Client instance)
  - **No persistence**: Clients register and connect instantly (no database of past clients)

### 2. **Client Class** (Currently Empty Placeholder)
Right now it's just `class Client{};` on line 9 of Server.hpp.

**This is what you mentioned needing "only one class" for** - a single Client class to represent each connected user.

### 3. **Channel Class** (Skeleton Only)
Represents IRC channels where clients can join and communicate.

**Design approach:**
- Server maintains a container of Channel objects
- Each Channel maintains a list of Client pointers (members of that channel)

## Client Class Needs

Based on IRC protocol (RFC 1459) and your server structure, a Client class needs to:

**Essential Data:**
- `fd` - Socket file descriptor (for socket communication)
- `nick` - Client nickname (IRC identity)
- `user` - Client username (no realname needed per subject requirements)
- **Server operator flag (IRCop)**: Is this client a server operator? (global privilege)
- Message buffers (incoming/outgoing data) - to be review

**Essential Functionality:**
- Send/receive messages over the socket (using fd)
- Handle PRIVMSG (private messages to other clients)
- Handle JOIN (join channels)
- Parse incoming IRC commands (NICK, USER, JOIN, PRIVMSG, etc.)
- Track registration state (need PASS + NICK + USER before full registration)

## Server Architecture & Data Containers

**Server manages two main containers:**
1. **Container of Clients** - All clients currently connected to the server
2. **Container of Channels** - All active channels on the server
   - Each Channel has a list of Client pointers (members in that channel)

## How It All Works Together

1. **Server starts** → `main.cpp` calls `server.setup()` 
   - Creates listening socket on specified port
   - Socket is non-blocking and ready to accept connections

2. **Main loop** → `server.runtime()` needs to:
   - Use `poll()` with `clientsPoll` vector to monitor all sockets
   - Accept new connections → create Client objects (instant registration, no persistence)
   - Read data from existing clients → parse IRC commands
   - Route messages between clients/channels

3. **Client lifecycle:**
   - New connection → Server accepts → creates Client instance
   - Client sends PASS, NICK, USER → Server validates → Client registered (instant, no database)
   - Client sends commands → Server routes to handlers
   - Client disconnects → Server removes from poll and destroys Client

4. **Channel system:**
   - Clients JOIN channels → Channel adds client to its member list, Client adds channel to its list
   - PRIVMSG: private messages sent directly from one client to another client
   - Channel messages use a different mechanism (not PRIVMSG)
   - **Channel modes**: operator status (per-channel), invite-only, topic restrictions, etc.
   - Channel operator flags are stored per-channel (not global client property)

## Current State Summary

**Implemented:**
- ✅ Basic Server skeleton with socket setup
- ✅ Non-blocking socket configuration
- ✅ Command-line argument handling structure

**Not Implemented (Empty/Stub):**
- ❌ Client class (completely empty)
- ❌ Channel class (skeleton only)
- ❌ Poll loop in runtime()
- ❌ Connection acceptance logic
- ❌ IRC command parsing
- ❌ Message routing
- ❌ Authentication flow
- ❌ Error handling/exceptions

## Why "Only One Class" for Client?

You only need one Client class (not multiple client types) because:
- IRC clients are uniform - all have the same core properties
- Server operator and channel operator roles are handled via flags, not inheritance
- One Client class + flags is simpler than a class hierarchy

**Important distinction - Two types of operator flags:**
1. **Channel operator flags** (in Channel): which clients are operators *in that specific channel*
   - A client can be channel operator in Channel A but not in Channel B
   - Independent from server operator status
