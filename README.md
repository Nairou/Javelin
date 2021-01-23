# Javelin
Javelin is a reliable message-based network library written in C. It is primarily designed for use in client/server games.

**Work in progress**

Javelin is incomplete and has bugs. It's primary purpose is for use in my own multiplayer game projects, and will evolve as it gets more use.

Existing features:

* Client/server connection management
* Message-based API (for sending large numbers of small messages, rather than whole packets)
* Reliable message ordering (messages are guaranteed to arrive in the order you send them)
* Cross platform

TODO:

* Connection security
  * Randomized connection challenge/response
  * Tagged packet headers
* Allow sending of unreliable messages
* Add bandwidth tracking and throttling
* Documentation...

**Usage**

Just include `javelin.h` and `javelin.c` in your project.

Javelin is written in C, and uses some C11 features. It might not compile in C++.

See `test.c` for a simple example.
