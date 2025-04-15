// All Libraries
#include <stdio.h> // Provides functions for input or output operations. Also used for logging. 
#include <unistd.h> // Gives access to system calls.
#include <stdlib.h> // Includes dynamic memory functions. This is essential for handling memory allocation.
#include <stdint.h>  // Defines types of uint8_t, uint16_t, and uint32_t which ensures specific bit sizes for cross platforms. Important to ake sure data sizes match exactly between systems.
#include <string.h> // Provides fuctions for handling strings and memory blocks.
#include <arpa/inet.h> // Includes fucntions to convert between IP address formats.
#include <sys/socket.h> // Contains the functions and structures needed for sockets.

// Symbolic Constants
#define PORT 8080 // Port number the server is listening on
#define MAX_CLIENTS 1 // Number of clients the server is able to have. 
#define BUFFER_SIZE 1024 // Buffer size for message data
#define LOG_FILE "server.log" // Log file name used to store server-side events and messages.

// Ensures no padding is added between fields of the struct.
#pragma pack(push, 1)
typedef struct 
{
    uint16_t srcPort; // Source Port of the client
    uint16_t destPort; // Destination Port 
    uint32_t seqNum; // Sequence Number (for tracking message order)
    uint8_t ackFlag; // Acknowledgment Flag (1 if ACK, 0 if not)
    uint8_t synFlag; // Synchronization Flag (used to initiate connection)
    uint8_t finFlag; // Finish Flag (used to close the connection)
    uint16_t plSIZE; // Size of the payload (in bytes)
} CustomHeader;
#pragma pack(pop)

// Symbolic Constants 
#define HEADER_SIZE sizeof(CustomHeader) // The total size of the header in bytes.
FILE *log_file; // File pointer used to handle log writing

/*
    Reads 'len' bytes form the socket into the buffer. 
    The function loops until all requested bytes are received or an error or disconnection occurs.
*/
ssize_t recv_all(int sock, char *buffer, size_t len)
{
    size_t total = 0;
    
    while(total < len)
    {
        ssize_t bytes = recv(sock, buffer + total, len - total, 0);

        if(bytes <= 0)
        {
            return bytes; // Error or disconnection
        }

        total += bytes;
    }

    return total;
}

/*
    Generates a server side response based on flags in the custom header. 
    Returns a essage string appropriate to the type of request or flag. 
*/
const char* get_response(const CustomHeader * header, char *response, size_t bufsize)
{
    if(header->synFlag == 1)
    {
        snprintf(response, bufsize, "SYN received - connection was initiated");
    }
    else if (header->ackFlag == 1)
    {
        snprintf(response, bufsize, "ACK received - message was acknowledged");
    }
    else if(header->finFlag == 1)
    {
        snprintf(response, bufsize, "FIN received - connection is closing");
    }
    else
    {
        snprintf(response, bufsize, "Data received - payload length: %u", header->plSIZE);
    }
    
    return response; 
}

// Function to log messages to a file
/*
    Logs a message to a file safely
*/
void log_message(const char *mess)
{
    log_file = fopen(LOG_FILE, "a");

    // Checks to see if file was opened
    if(log_file)
    {
        fprintf(log_file, "%s\n", mess);
        fclose(log_file);
    }
}

int main()
{
    // Variables
    int sock, cl_sock, size;
    struct sockaddr_in add;
    socklen_t cl_len = sizeof(add);
    char logBuff[BUFFER_SIZE];

    // Checks to see if the creation of the socket worked
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Allows the port to be reused immediately after the server is restarted
    size = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &size, sizeof(size));

    // sockaddr_in fields
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = INADDR_ANY;
    add.sin_port = htons(PORT);

    // Binds the socket
    if(bind(sock, (struct sockaddr*)&add, sizeof(add)) < 0)
    {
        perror("The bind has failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // This listens for incomng connections
    if(listen(sock, MAX_CLIENTS) < 0)
    {
        perror("Listening has failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);
    log_message("Server has started");

    // Accept a client connection
    if((cl_sock = accept(sock, (struct sockaddr *)&add, &cl_len)) < 0)
    {
        perror("accept");
        close(sock);
        exit(EXIT_FAILURE);
    }

    snprintf(logBuff, sizeof(logBuff), "Client connected: %s", inet_ntoa(add.sin_addr));
    log_message(logBuff);

    // This accpects incoming connnections 
    while(1)
    {
       CustomHeader header;
        ssize_t r = recv_all(cl_sock, (char *)&header, HEADER_SIZE);

        if(r <= 0)
        {
            log_message("Client was disconnected or there was an error in receiving header.");
            break;
        }

        // convert fields from network to host byte order
        header.srcPort = ntohs(header.srcPort);
        header.destPort = ntohs(header.destPort);
        header.seqNum = ntohl(header.seqNum);
        header.plSIZE = ntohs(header.plSIZE);

        // Log header info
        snprintf(logBuff, sizeof(logBuff), "Recieved Header\n Source Port: %u\n Dest Port: %u\n Sequence No: %u\n ACk: %d SYN: %d FIN: %d\n Payload Size: %u", header.srcPort, header.destPort, header.seqNum, header.ackFlag, header.synFlag, header.finFlag, header.plSIZE);
        log_message(logBuff);

        // If payload is expected, receive it 
        if(header.plSIZE > 0)
        {
            char *load = malloc(header.plSIZE + 1); // +1 for null terminator

            if(load == NULL)
            {
                log_message("Memory allocation has failed for the payload");
                break;
            }

            r = recv_all(cl_sock, load, header.plSIZE);

            if(r <= 0)
            {
                free(load);
                log_message("There was an error when receiving payload or the client disconnected");
                break;
            }

            load[header.plSIZE] = '\0'; // Null terminate the payload string
            snprintf(logBuff, sizeof(logBuff), "Client says:\n%s", load);
            log_message(logBuff);
            free(load);
        }

        // Prepare and send a response message
        char response[BUFFER_SIZE];
        get_response(&header, response, sizeof(response));

        if(send(cl_sock, response, strlen(response), 0) == -1)
        {
            perror("Send");
            break;
        }

        snprintf(logBuff, sizeof(logBuff), "Sent response: %s", response);
        log_message(logBuff);

        // Check if client wants to erminate the connection
        if(header.finFlag == 1)
        {
            log_message("Fin flag has been received. Closing the connection.");
            break;
        }
    }

    // Clean up
    close(cl_sock);
    close(sock);
    log_message("Server has shut down.");

    return 0;
}