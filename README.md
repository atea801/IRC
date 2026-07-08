*This project has been created as part of the 42 curriculum by bkaras-g, aautret and tlorette.*

# ft_irc

## Description

**ft_irc** is an Internet Relay Chat (IRC) server written in C++98. It implements the
core of the IRC protocol so that a real IRC client (our reference client being
**irssi**) can connect to it, register, join channels, exchange messages, and moderate
channels — just like connecting to any official IRC server.

The server handles multiple clients simultaneously over TCP/IP using a single
non-blocking `poll()` loop: no forking, no thread, no blocking I/O. Incoming data is
buffered per client and reassembled into complete commands before being parsed and
executed, which allows the server to correctly process messages received in several
fragments.

The goal of the project is to understand how a standard network protocol works from the
socket layer up to command parsing and state management, and to implement it cleanly
within the constraints of C++98.

## Features

Registration and connection:
- `PASS`, `NICK`, `USER` — password authentication and client registration
- `CAP` — minimal capability negotiation (no capability advertised)
- `PING` / `PONG` — keep-alive handling
- `QUIT` — clean disconnection with broadcast to shared channels

Messaging:
- `PRIVMSG` — private messages to a user, and messages broadcast to a channel
- Messages sent to a channel are forwarded to every other member of that channel

Channels and operators:
- `JOIN` — join or create a channel
- `PART` — leave a channel
- `KICK` — eject a user from a channel (operator only)
- `INVITE` — invite a user to a channel (operator only)
- `MODE` — change channel modes:
  - `i` — invite-only channel
  - `t` — restrict `TOPIC` changes to operators
  - `k` — set/remove the channel key (password)
  - `o` — give/take channel operator privilege
  - `l` — set/remove the user limit

Extras:
- A decorative welcome banner sent on connection
- Colored server messages (mIRC color codes, rendered by irssi)

## Instructions

### Requirements

- A C++ compiler supporting the C++98 standard (`c++` / `g++` / `clang++`)
- `make`
- An IRC client for testing (reference client: **irssi**)

### Compilation

```bash
make        # builds the ircserv executable
make clean  # removes object files
make fclean # removes object files and the executable
make re     # fclean + all
```

### Execution

```bash
./irc <port> <password>
```

- `port` — the port the server listens on for incoming connections
- `password` — the connection password required by every client

Example:

```bash
./irc 6667 mypassword
```

### Connecting with irssi

```
/connect 127.0.0.1 6667 mypassword
/join #general
/msg #general Hello everyone
```

Native irssi commands (`/join`, `/part`, `/msg`, `/nick`, `/kick`, `/mode`, ...) are
translated by the client into IRC messages, so you do not need `/quote` for them.

### Testing with nc

The server aggregates partial packets before processing a command. You can verify this
by sending a command in several fragments:

```bash
nc -C 127.0.0.1 6667
```

## Technical choices

- **Single `poll()` loop.** All I/O (accept, read, write) is driven by one `poll()`
  call. File descriptors are set non-blocking with `fcntl(fd, F_SETFL, O_NONBLOCK)`.
- **Per-client input buffer.** `recv()` appends to a per-client buffer; the server
  extracts and executes complete commands terminated by a newline, tolerating only
  `\r\n`.
- **Command dispatch.** Each raw line is parsed into a command and its arguments, then
  routed to a dedicated handler. Unknown commands return `ERR_UNKNOWNCOMMAND (421)` so
  the client never hangs waiting for a reply.
- **Numeric replies.** Registration is acknowledged with the standard welcome sequence
  (`001`–`004`), and errors follow the RFC numeric-reply format.
- **`PtrVec` wrapper.** A small template around `std::vector<T*>` is used to manage
  channel members, operators, and invited users without duplicating objects.

## Resources

Protocol references used to understand and implement the IRC standard:

- RFC 1459 — Internet Relay Chat Protocol
- RFC 2812 — Internet Relay Chat: Client Protocol (registration flow, numeric replies)
- Modern IRC documentation — <https://modern.ircdocs.horse/>
- Beej's Guide to Network Programming (sockets, `poll`, non-blocking I/O) —
  <https://beej.us/guide/bgnet/>
- The Linux man pages for `socket`, `bind`, `listen`, `accept`, `poll`, `recv`,
  `send`, `fcntl`

### Use of AI

AI tools were used as a support during development, and every AI-assisted part was
reviewed, tested, and understood before being kept. Specifically, AI was used to:

- Design and format the decorative welcome banner (ASCII art and mIRC color codes) sent
  to clients, and verify its alignment.
- Explain specific parts of the IRC protocol (the registration sequence, the meaning of
  the `RPL_MYINFO (004)` fields, capability negotiation with `CAP`).
- Help write and structure this README.

AI was **not** used to generate core project logic blindly: the socket handling, the
`poll()` loop, the command parsing, and the channel/operator logic were written and are
fully understood by the authors.
