# Javelin
Javelin is a reliable message-based network library written in C (C11). It is primarily designed for use in client/server games.

**Work in progress**

Javelin is incomplete, and may have bugs. It's primary purpose is for use in my own multiplayer game projects, and will evolve as it gets more use.

## Features

* [X] Cross platform
* [X] Client/server connection management
* [X] Message-based API (for sending large numbers of small messages, rather than whole packets)
* [X] Reliable message ordering (messages are guaranteed to arrive in the order you send them)
* [X] Packet salting, for protection against basic attacks
* [ ] Unit tests
* [ ] Documentation
* [ ] Allow sending of unreliable messages
* [ ] Add bandwidth tracking and throttling

## Usage

Copy `javelin.h` and `javelin.c` into your project source tree.

See `example.c` for a simple example.
