// *** Including relevant headers ***
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

// *** Constants ***
#define PORT 5000
#define MAX_READ_BUF_SIZE 1024
#define MAX_WRITE_BUF_SIZE 1024

int main()
{
    // *** Initialize socket for listening ***
    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("Failed: Initializing socket \n");
        exit(1);
    }

    // *** Initialize local listening socket address ***
    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY allows to connect to any one of the host’s IP addresses

    // *** Socket Bind ***
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Failed: Binding socket file descriptor to transport layer address \n");
        exit(1);
    }

    // *** Socket Listen ***
    if (listen(sockfd, 10) == -1) 
    {
        perror("Failed: Listen on socketfd \n");
        exit(1);
    }

    // *** Accept and Read&Write **
    int client_fd;
    struct sockaddr_in client_addr; // client address
    unsigned int sin_size;
    char read_buf[MAX_READ_BUF_SIZE];
    char write_buf[MAX_WRITE_BUF_SIZE];
    int write_buf_size;
    int read_buf_size;

    while (1) 
    { 
        // main accept() loop, loop infinitely
        printf("Server is now accepting connections\n");

        //Accept connection from client
        sin_size = sizeof(struct sockaddr_in);
        if ((client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
            perror("Failed: Accepting connection from client \n");
            continue;
        }
        printf("Received new connection from %s\n", inet_ntoa(client_addr.sin_addr));
        
        //Read from socket
        memset(read_buf, 0, 1024);
        if ((read_buf_size = read(client_fd, read_buf, 1024)) < 0)
        {
            perror("Failed: Reading message from client. Closing connection. \n");
            close(client_fd);
            continue;
        }
        printf("Message from client: \n %s", read_buf);

        //TODO: Parse Request
        const char *end_of_method = strchr(read_buf, ' ');
        const char *end_of_url = strchr(end_of_method + 1, ' ');
        printf("Found end points of strings successfully");

        char method[end_of_method - read_buf + 1];
        char url[end_of_url - (end_of_method + 1) + 1];
        printf("Allocated memory for strings successfully");

        strncpy(method, read_buf,  end_of_method - read_buf);
        strncpy(url, end_of_method + 1, end_of_url - (end_of_method + 1));
        printf("Copied strings successfully");    

        method[sizeof(method) - 1] = 0;
        url[sizeof(url) - 1] = 0;
        printf("NULL terminated strings successfully"); 

        printf("Parsed request as: \n")
        printf("Method: %s\n", method);
        printf("URL: %s\n", url);

        //TODO: Retreive appropriate file - handle not found error

        //TODO: Set up write buffer    
        memset(write_buf, 0, MAX_WRITE_BUF_SIZE);
        char* helloWorld = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 13\n\nHello, world!";
        memcpy(write_buf, helloWorld, strlen(helloWorld)); //FIXME: Dummy code
        write_buf_size = strlen(write_buf);

        //TODO: Construct Header

        //Write to socket
        if (write(client_fd, write_buf, write_buf_size) < 0)
        {
            perror("Failed: Writing message to client. Closing connection. \n");
            close(client_fd);
            continue;
        }
        printf("Response to client: \n %s", write_buf);
        
        //Close connection
        close(client_fd);
    }
}
