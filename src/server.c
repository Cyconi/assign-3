#include "../include/filter.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define BUFSIZE 1024

static void process_client_request(int new_socket)
{
    char    buffer[BUFSIZE];
    ssize_t bytes_read;
    char   *saveptr;

    bytes_read = read(new_socket, buffer, BUFSIZE);
    if(bytes_read > 0)
    {
        char        response[BUFSIZE];
        const char *conversion_type = strtok_r(buffer, ":", &saveptr);
        char       *client_string   = strtok_r(NULL, ":", &saveptr);
        buffer[bytes_read]          = '\0';

        if(conversion_type && client_string)
        {
            printf("Processing Request: %s:%s\n", conversion_type, client_string);
            for(int i = 0; client_string[i]; i++)
            {
                if(strcmp(conversion_type, "upper") == 0)
                {
                    client_string[i] = upper_filter(client_string[i]);
                }
                else if(strcmp(conversion_type, "lower") == 0)
                {
                    client_string[i] = lower_filter(client_string[i]);
                }
                else
                {
                    client_string[i] = null_filter(client_string[i]);
                }
            }
            snprintf(response, BUFSIZE, "%s", client_string);
        }
        else
        {
            snprintf(response, BUFSIZE, "Invalid format.");
        }

        write(new_socket, response, strlen(response));
    }
    close(new_socket);
}

static int create_server_socket(void)
{
    int                server_fd;
    struct sockaddr_in address;
    int                opt = 1;
    server_fd              = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(PORT);

    // Binding the socket to the network address and port
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    // Check server_fd if its valid
    if(0 > server_fd)
    {
        perror("Socket is an invalid file descriptor");
        exit(EXIT_FAILURE);
    }
    // Listen
    if(listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

static void handle_sigint(int signal) __attribute__((noreturn));

static void handle_sigint(int signal)
{
    (void)signal;
    _exit(EXIT_SUCCESS);
}

int main(void)
{
    int                server_fd;
    struct sockaddr_in address;
    int                addrlen = sizeof(address);

    signal(SIGINT, handle_sigint);

    // Creating socket file descriptor
    server_fd = create_server_socket();

    printf("Server is running...\n");

    while(1)
    {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if(new_socket < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        if(fork() == 0)
        {
            close(server_fd);
            process_client_request(new_socket);
            exit(0);
        }
        else
        {
            close(new_socket);
        }
    }
}
