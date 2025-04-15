// All Libraries
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Symbolic Constants
#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

#pragma pack(push, 1)
typedef struct
{
    uint16_t srcPort;
    uint16_t destPort;
    uint32_t seqNum;
    uint8_t ackFlag;
    uint8_t synFlag;
    uint8_t finFlag;
    uint16_t plSIZE;
} CustomHeader;
#pragma pack(pop)

const int HEADER_SIZE = sizeof(CustomHeader);

int get_val_flag(const char *name)
{
    int val;

    while(1)
    {
        printf("Enter %s (0 or 1): ", name);

        if(scanf("%d", &val) == 1 && (val == 0 || val == 1))
        {
            break;
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
    int sock;
    struct sockaddr_in add;

    // Checks to see if the creation of socket worked
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation has failed");
        exit(EXIT_FAILURE);
    }

    // sockadd_in fields
    add.sin_family = AF_INET;
    add.sin_port = htons(PORT);

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

    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_IP, SERVER_PORT);

    uint16_t srcPort = 8000;
    uint16_t destPort = PORT;
    uint32_t seqNum = 1;

    int ackFlag = get_val_flag("ACK flag");
    int synFlag = get_val_flag("SYN flag");
    int finFlag = get_val_flag("FIN flag");

    char load[BUFFER_SIZE];
    printf("Enter a message for server: ");
    getchar();
    fgets(load, sizeof(load), stdin);
    load[strcspn(load, "\n")] = '\0';

    uint16_t plSIZE = strlen(load);

    CustomHeader header;
    header.srcPort = htons(srcPort);
    header.destPort = htons(destPort);
    header.seqNum = htonl(seqNum);
    header.ackFlag = ackFlag;
    header.synFlag = synFlag;
    header.finFlag = finFlag;
    header.plSIZE = htons(plSIZE);

    char *pack = malloc(HEADER_SIZE + plSIZE);
    
    if(!pack)
    {
        perror("Memory allocation has failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    memcpy(pack, &header, HEADER_SIZE);
    memcpy(pack + HEADER_SIZE, load, plSIZE);

    ssize_t total = HEADER_SIZE + plSIZE;

    if(send(sock, pack, total, 0) != total)
    {
        perror("Send has failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Sent message to server.\n");
    free(pack);

    char response[BUFFER_SIZE] = {0};
    ssize_t r = recv(sock, response, sizeof(response) - 1, 0);

    if(r > 0)
    {
        response[r] = '\0';
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