// ALL Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <apra/inet.h>
#include <sys/socket.h>


// Synbolic Constants
#define PORT 8080
#define MAX_CLIENTS 1
#define BUFFER_SIZE 1024
#define LOG_FILE "server.log"

// Force 1-byte alignemt
#pragma pack(push, 1)
typedef struct 
{
    uint16_t srcPort; // Source Port
    uint16_t destPort; // Dest Port
    uint32_t seqNum; // Seequence Number
    uint8_t ackFlag; // ACK Flag
    uint8_t synFlag; // SYN Flag
    uint8_t finFlag;
    uint16_t plSIZE; // Payload Size
} CustomHeader;
#pragma pack(pop)

// Sybolic Constants 
#define HEADER_SIZE sizeof(CustomHeader)
FILE *log_file; 

ssize_t recv_all(int sock, char *buffer, size_t len)
{
    size_t total = 0;
    
    while(total < len)
    {
        ssize_t bytes = recv(sock, buffer + total, len - total, 0);

        if(bytes <= 0)
        {
            return bytes;
        }

        total += bytes;
    }

    return total;
}

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

        header.srcPort = ntohs(header.srcPort);
        header.destPort = ntohs(header.destPort);
        header.seqNum = ntohl(header.seqNum);
        header.plSIZE = ntohs(header.plSIZE);

        snprintf(logBuff, sizeof(logBuff), "Recieved Header\n Source Port: %u\n Dest Port: %u\n Sequence No: %u\n ACk: %d SYN: %d FIN: %d\n Payload Size: %u", header.srcPort, header.destPort, header.seqNum, header.ackFlag, header.synFlag, header.finFlag, header.plSIZE);
        log_message(logBuff);

        if(header.plSIZE > 0)
        {
            char *load = malloc(header.plSIZE + 1);

            if(!load)
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

            load[header.plSIZE] = '\0';
            snprintf(logBuff, sizeof(logBuff), "Client says:\n%s", load);
            log_message(logBuff);
            free(load);
        }

        char response[BUFFER_SIZE];
        get_response(&header, response, sizeof(response));

        if(send(cl_sock, response, strlen(response), 0) == -1)
        {
            perror("Send");
            break;
        }

        snprintf(logBuff, sizeof(logBuff), "Sent response: %s", response);
        log_message(logBuff);

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