Title: Programming Assignment 2 – TCP Header Simulation in Client-Server Communication

Description: This program works with TCP socket programming to build a simplified client-server model. It simulates key features of TCP headers. It focuses on creating, sending, parsing, and responding to structured messages that contain a custom-defined header.

Purpose: Design a fixed-format message structure where the client includes a custom TCP-like header in each message. The goal is to reinforce low-level protocol concepts and provide hands-on experience with TCP/IP socket programming, structuring and parsing binary data, and conditional server behavior based on message headers.

Build Project: You will need three files: a client file, a server file, and a Makefile. The client and server files all share the same libraries: <stdio.h>, <unistd.h>, <stdlib.h>, <stdint.h>, <string.h>, <arpa/inet.h>, and <sys/socket.h>. 

Client: This is designed to communicate with a custom server using a custom binary protocol. The client file sends a structured packet that includes a header with metadata and a payload. This client file demonstrates: Socket programming with TCP, Custom binary messaging, Interactive flag setting (ACK, SYN, FIN), Dynamic memory usage, User input handling, Basic error checking and logging. 
How the client works: 
1. Connection Setup:  A TCP socket is created and configured. The client connects to the server.
2. User Input: The user is prompted to enter the three binary flags (ACK, SYN, FIN).
3. Packet Creation: The header is filled with network-byte-order values.
4. Sending Packet: The packet is sent to the server.
5. Waiting for Reply: The client waits for a reply from the server.
6. Cleanup: Memory is freed, the socket is closed, and the client exits cleanly.
Run the client file: ./client

Server: The server listens on port 8080 and accepts a connection from a client. It processes structured messages that include a header and a payload. The server file demonstrates: Socket programming, Handling custom binary protocols, Byte-order conversion, Logging server activity, Safe memory and buffer management
How the server works:
1. Socket Setup: A TCP socket is created and bound to INADDR_ANY:8080, then it listens for incoming connections.
2. Connection Handling: Accepts a single client connection and logs the client IP and connection info.
3. Receiving Messages: Reads the CustomHeader structure using a helper (recv_all) to ensure the full header is received. Converts multi-byte values from network byte order to host byte order using ntohs() and ntohl().
4. Payload Processing: Receives and logs the payload if present.
5. Response Creation: The server creates a meaningful message using get_response() and sends it to the client.
6. Logging: All actions are logged to server.log.
Run the server file; ./server

Makefile: This automates the building of both the client and server programs. The default target (make) builds both executables by compiling client.c and server.c. Individual targets such as make client or make server can be used to compile each component separately.

Run Project: In your virtual machine, open a terminal for server and one for client

1. Update package list: sudo apt-get update
2. Install necessary libraries: sudo apt install git
3. Type: git clone (the link) to get access to the program from github
4. Type: make to start the Makefile
5. Enter the password for your virtual machine if you have one
6. Type: ./server (runs the server file)
7. Type: ./client (runs the client file)
8. After that, it will ask you to send a message
9. It will respond with the message recieved
10. Press ctrl C to kill the server

Required Libraries: <stdio.h>, <unistd.h>, <stdlib.h>, <stdint.h>, <string.h>, <arpa/inet.h>, and <sys/socket.h>. 

Message Header: This project has a custom message header structure to facilitate communication between the client and server files over a TCP connection. The CustomHeader struct is defined using fixed-width integer types (uint8_t, uint16_t, uint32_t) to ensure consistent cross-platform binary compatibility.The message header includes fields for source and destination ports, sequence number, and control flags for acknowledgment (ACK), synchronization (SYN), and connection termination (FIN). It also stores the size of the accompanying payload. The message header is packed using #pragma pack(push, 1) to prevent the compiler from inserting padding between fields. This ensures the binary layout remains precise during transmission. All multi-byte values are converted to and from network byte order to maintain consistency across different machine architectures.

ChatGPT: Used for either clarification or checking. Finding the libraries need to run for struct and socket. Used for an idea of how to format the makefile and the coustom message header. 
