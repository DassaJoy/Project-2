// All Libraries
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Symbolic Constants
#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

// Fucntion to start openssl
/*
    OpenSSl proceeds crytopgraphic functionality
    and secures socket layer protocols
*/
void start_openssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// Function that cleans up openssl
/*
    This will ensure that the resources
    used by the library are properly released
*/
void clean_openssl()
{
    EVP_cleanup();
}

// Functions that creates teh ssl
/*
    SSL is a cryptographic protocol that provides
    secure communication over the internet by 
    encrypting data between a client and server  
*/
SSL_CTX *create()
{
    const SSL_METHOD *m;
    SSL_CTX *ctx;

    m = TLS_client_method();
    ctx = SSL_CTX_new(m);

    if(!ctx)
    {
        perror("Not able to create SSL");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// Function that communicates with server
/*
    Handles user input, sends it to the server via SSL
    and waits for and prints the server's response
*/
void communicate_server(SSL *sl)
{
    char size[BUFFER_SIZE];

    // This gets user's input
    printf("Enter message: ");
    fgets(size, seizeof(size), stdin);

    // This sends the message to server
    SSL_write(sl, sizeof(size), stdin);

    // Reads the server's response
    int byte = SSl_read(sl, size, sizeof(size));

    // Checks if SSL_read fialed
    if(byte <= 0)
    {
        perror("SSL read has failed");
    }
    else
    {
        size[byte] = '\0'
        printf("Server response: %s\n", size);
    }
}

int main()
{
    // Varibales
    int sock;
    struct sockaddr_in add;
    SSL_CTX *ctx;
    SSL *sl;

    // Launches OpenSSL
    start_openssl();
    ctx = create();

    // Creates the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Checks to see if the creation of socket worked
    if(sock < 0)
    {
        perror("Socket creation has failed");
        exit(EXIT_FAILURE);
    }

    // sockadd_in fields
    add.sin_family = AF_INET;
    add.sin_port = htons(PORT);
    add.sin-add.s_addr = inet_addr(SERVER_IP);

    // Checks connection to the server
    if(connect(sock, (struct sockadd*)&add, sizeof(add)) < 0)
    {
        perror("Connection has failed");
        exit(EXIT_FAILURE);
    }

    // Creates SSL object
    sl = SSL_new(ctx);
    SSL_set_fd(sl, sock);

    // Preforms SSl Handshake
    /*
        This the process of establishing a secure,
        encrypted connection between client and server
    */
   if(SSL_connect(s1) < 0)
   {
        perror("SSL handshake ahs failed");
   }
   else
   {
        communicate_server(sl);
   }

   // Clean up
   close(sock);
   SSL_free(sl);
   clean_openssl();

   return 0; 
}