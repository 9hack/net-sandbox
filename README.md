# net-sandbox

Basic server and client networking library.  Each includes a main function to test the functionality, and runs on port `9000` by default.

### Server

Usage: `./server`

Runs a server on port `9000`.  Has a main thread that runs approximately every 300 ms, which reads from all connected clients and sends them a message.

### Client

Usage: `./client <host> <message>`

For example, `./client localhost kavin-smells` will start a client on port `9000` and will send `kavin-smells` to the server.  Has a main thread that runs approximately every 100 ms, which reads from the server and sends the message.

### What's in /trash?

Just some stuff I was messing with.  Most of it doesn't work properly, so don't worry about it!
