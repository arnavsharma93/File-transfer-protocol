is is a small program that demonstrates the setup of a 
TCP/IP connection.

Before compiling it, you may want to edit port.h and set a
(hopefully unique for your environment) default port number
(in the range 1024...65535).

Then run make to compile it (on remus, you may need to add /usr/ccs/bin
to your PATH environment).

On Linux machines or Windows systems with cygwin installed, run:
	make -f Makefile.linux

1. Server
In one window, run
	./server
or
	./server -p <port#>
if you want to specify a port on the command line.
The server runs forever, accepting connections (one at a time).
Break out of it via ^C (or whatever your interrupt character is).

The server simply accepts connections and prints the address and port
of the connecting client.

2. Client
In another window (possibly on another machine), run the client:
	./client

The usage for client is
	client [-h host] [-p port]

If you are running the client on another machine, you must specify the
machine where the server is running. If you specified a port number on the
server with -p, you'll need to tell the client to use the same port. For example:
	client -h romulus -p 2422
