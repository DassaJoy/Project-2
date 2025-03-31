// ALL Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <aprap/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Synbolic Constants
#define PORT 8080
#define MAX_CLIENTS 1
#define BUFFER_SIZE 1024
#define LOG_FILE "server.log"

FILE *log_file;

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

// Function to start openssl
/*
    OpenSSL proceeds crytopgraphic functionality
    and secures socket layer protocols
*/
void start_openssl()
{
    SSL_load_erro_strings();
    OpenSSl_add_ssl_algorithms();
}

// Functions that cleans up openssl
/*
    This will ensure that the resources
    used by the library are properly relased
*/
void clean_openssl()
{
    EVP_cleanup();
}

// Function that creates the ssl
/*
    SSL is a cryptopgraphic protocol that provides 
    secure communication over the internet by 
    encrypting data between a client and server
*/
SSL_CTX *create()
{
    const SSL_METHOD *m;
    SSL_CTX *ctx;

    m = TLS_server_method();
    ctx = SSL_CTX_new(m);

    if(!ctx)
    {
        log_message("Failed to create the SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// Function that configures SSL
/*
    Loads a SSL certificate from a specified file.
    Loads the corresponding private key.
    Verifies that the private key matches the certificate
*/
void configgure(SSL_CTX *ctx)
{
    // Load the SSL certificate from file
    if(SSL_CTX_use_certificate_file(ctx, "ssl_certs/server.crt", SSL_FILETYYPE_PEM) <= 0)
    {
        log_message("Unable to load the certificate file");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Load the private key from the file
    if(SSL_CTX_use_PrivateKey_file(ctx, "ssl_certs/server.key", SSL_FILETYPE_PEM) <= 0)
    {
        log_message("Unable to load the private key file");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Verify that the loaded private key matches the loaded certificate
    if(!SSL_CTX_check_private_key(ctx))
    {
        log_message("Private key does not match the public certificate");
        exit(EXIT_FAILURE)
    }
}

// Function that handles the client communication
/*
    Handles receiving a message from the client, logging 
    the recieved message and sending a confirmation message 
    back to the client 
*/
void *client(void *sl)
{
    // Varibles
    SSL *new_sl = (SSL *)sl;
    char size[BUFFER_SIZE];
    int byte;

    // Reveices message from client
    byte = SSL_read(new_sl, size, sizeof(size));

    // Check if the read operation worked
    if(byte <= 0)
    {
        log_message("SSL read has failed");
        ERR_print_eeros_fp(stderr);
    }
    else
    {
        size[byte] = '\0';
        log_message(size);
        printf("Received: %s\n", size);

        // Sends confirmation
        SSL_write(new_sl, "message received!", strlen("Message received!"));
    }

    SSL_shutdown(new_sl);
    SSL_free(new_sl);
}

int main()
{
    // Variables
    int sock, cl_sock;
    struct sockaddr_in add, cl_add;
    socklen_t cl_len = sizeof(cl_add);
    SSL_CTX *ctx;
    SSL *sl;

    //Launches OpenSSL
    start_openssl();
    ctx = create();
    configure(ctx);

    // Creates teh socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Checks to see if the creation of the socket worked
    if(sock < 0)
    {
        perror("Socket creation has failed");
        exit(EXIT_FAILURE);
    }

    // sockaddr_in fields
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = INADDR_ANY;
    add.sin_port = htons(PORT);

    // Binds the socket
    if(bind(sock, (struct sockadd*)&add, sizeof(add)) < 0)
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

    // This accpects incoming connnections 
    while(1)
    {
        cl_sock = accept(sock, (struct sockaddr*)&cl_add, &cl_len);

        // Checks if cl_sock failed
        if(cl_sock < 0)
        {
            perror("Accpeting has failed");
            continue;
        }

        // Creates SSL object
        sl = SSL_new(ctx);
        SSL_st_fd(sl, cl_sock);

        // Preforms SSL Handshake
        /*
            This is the process of establishing a secure,
            encrypted connection between client and server
        */
       if(SSL_accept(s1) <= 0)
       {
            log_message("SSL handshake has failed");
            ERR_print_errors_fp(stderr);
            SSL_free(sl);
            close(cl_sock);
            continue;
       }

       log_message("Client is connected");
    }

    // Clean up
    close(sock);
    SSL_CTX_free(ctx);
    clean_oenssl();

    return 0;
}