#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ADDRESS "127.0.0.1"
#define PORT 8080
#define BUFSIZE 1024

static int create_client_socket(void)
{
    int                sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(sock < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(PORT);

    if(inet_pton(AF_INET, ADDRESS, &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    return sock;
}

static void send_server_request(int sock, const char *conversion_type, const char *client_string)
{
    char    buffer[BUFSIZE];
    char    response[BUFSIZE];
    ssize_t bytes_read;

    snprintf(buffer, sizeof(buffer), "%s:%s", conversion_type, client_string);
    send(sock, buffer, strlen(buffer), 0);

    bytes_read = read(sock, response, BUFSIZE - 1);
    if(bytes_read > 0)
    {
        response[bytes_read] = '\0';
        printf("Processed String: %s\n", response);
    }
    else
    {
        perror("Error reading from socket");
    }

    close(sock);
}

int main(int argc, char *argv[])
{
    int         opt;
    const char *client_string   = NULL;
    const char *conversion_type = NULL;
    int         sock;

    while((opt = getopt(argc, argv, "s:f:")) != -1)
    {
        switch(opt)
        {
            case 's':
                client_string = optarg;
                break;
            case 'f':
                conversion_type = optarg;
                break;
            default:
                printf("Usage: %s -s <string> -f <conversion>\n", argv[0]);
                printf("<conversion> -> \"upper\", \"lower\", \"none\"\n");
                exit(EXIT_FAILURE);
        }
    }

    if(client_string == NULL || conversion_type == NULL)
    {
        printf("Usage: %s -s <string> -f <conversion>\n", argv[0]);
        printf("<conversion> -> \"upper\", \"lower\", \"none\"\n");
        exit(EXIT_FAILURE);
    }

    sock = create_client_socket();
    send_server_request(sock, conversion_type, client_string);

    return 0;
}
