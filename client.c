// All Libraries
#include <stdio.h> // Provides functions for input or output operations. Also used for logging. 
#include <unistd.h> // Gives access to system calls.
#include <stdlib.h> // Includes dynamic memory functions. This is essential for handling memory allocation.
#include <stdint.h>  // Defines types of uint8_t, uint16_t, and uint32_t which ensures specific bit sizes for cross platforms. Important to ake sure data sizes match exactly between systems.
#include <string.h> // Provides fuctions for handling strings and memory blocks.
#include <arpa/inet.h> // Includes fucntions to convert between IP address formats.
#include <sys/socket.h> // Contains the functions and structures needed for sockets.

// Symbolic Constants
#define SERVER_IP "127.0.0.1" // IP address of the server (local host)
#define PORT 8080 // Port number the server is listening on
#define BUFFER_SIZE 1024 // Buffer size for message data

/*
    A custom header structure that will be sent with each message. 
    The struture is packed to 1-byte alignment to match exact binary layout. 
*/
#pragma pack(push, 1)
typedef struct
{
    uint16_t srcPort; // Source Port of the client
    uint16_t destPort; // Destination Port 
    uint32_t seqNum; // Sequence Number (for tracking message order)
    uint8_t ackFlag; // Acknowledgment Flag (1 if ACK, 0 if not)
    uint8_t synFlag; // Syncronization Flag (used to initiate connection)
    uint8_t finFlag; // Finish Flag (used to close the connection)
    uint16_t plSIZE; // Size fo the payload (in bytes)
} CustomHeader;
#pragma pack(pop)

// Global constant for header size
const int HEADER_SIZE = sizeof(CustomHeader);


/*
    Prompts the user to enter a binary (0 or 1) value foa specific flag. 
    Validates the input until the user provided s correct value. 
*/
int get_val_flag(const char *name)
{
    int val;

    while(1)
    {
        printf("Enter %s (0 or 1): ", name);

        if(scanf("%d", &val) == 1 && (val == 0 || val == 1))
        {
            break; // Vaild flag value
        }
        else
        {
            printf("Invalid input. %s must be 0 or 1.\n", name);
            while(getchar() != '\n');
        }
    }

    return val;
}

int main()
{
    // Varibales
    int sock; // Socket file descriptor
    struct sockaddr_in add; // Structure to hold server address info

    // Checks to see if the creation of socket worked
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation has failed");
        exit(EXIT_FAILURE);
    }

    // sockadd_in fields
    add.sin_family = AF_INET; // IPv4
    add.sin_port = htons(PORT); // Conver port number to network byte order

    // converts human-readable IP address to binary format
    if(inet_pton(AF_INET, SERVER_IP, &add.sin_addr) <= 0)
    {
        perror("Invalid address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Checks connection to the server
    if(connect(sock, (struct sockaddr*)&add, sizeof(add)) < 0)
    {
        perror("Connection has failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", SERVER_IP, PORT);

    // Set basic fields for the message
    uint16_t srcPort = 8000; // Arbitrary source port value for identification
    uint16_t destPort = PORT; // Destination port matches the server's
    uint32_t seqNum = 1; // Start with sequence number 1

    // Prompts the user for flag values (ACK, SYN, FIN)
    int ackFlag = get_val_flag("ACK flag");
    int synFlag = get_val_flag("SYN flag");
    int finFlag = get_val_flag("FIN flag");

    // Prompts the user to enter a message (this is the payload)
    char load[BUFFER_SIZE];
    printf("Enter a message for server: ");
    getchar(); // Cconsume leftoever newline from previous scnaf
    fgets(load, sizeof(load), stdin); // Reads a full line of input
    load[strcspn(load, "\n")] = '\0'; // Removes trailling newline, if there is any

    // Calculate the payload size
    uint16_t plSIZE = strlen(load);

    // Construct the header, converts values to netwrok byte order
    CustomHeader header;
    header.srcPort = htons(srcPort);
    header.destPort = htons(destPort);
    header.seqNum = htonl(seqNum);
    header.ackFlag = ackFlag;
    header.synFlag = synFlag;
    header.finFlag = finFlag;
    header.plSIZE = htons(plSIZE);

    // Allocate memory for the full packer 
    char *pack = malloc(HEADER_SIZE + plSIZE);
    
    if(!pack)
    {
        perror("Memory allocation has failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Copy header and payload into the packet buffer
    memcpy(pack, &header, HEADER_SIZE);
    memcpy(pack + HEADER_SIZE, load, plSIZE);

    // Sends the complete packet to the server
    ssize_t total = HEADER_SIZE + plSIZE;

    if(send_all(sock, pack, total, 0) != total)
    {
        perror("Send has failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Sent message to server.\n");
    free(pack); // Frees the dynamically allocated buffer after sending

    // Buffer to receive server's response
    char response[BUFFER_SIZE] = {0};
    ssize_t r = recv(sock, response, sizeof(response) - 1, 0); // Leave space for null terminator

    // check if a response was received
    if(r > 0)
    {
        response[r] = '\0'; // Ensure null-termination
        printf("Server says: %s\n", response);
    }
    else
    {
        printf("No response or error.\n");
    }
    
   // Clean up
   close(sock);

   return 0; 
}