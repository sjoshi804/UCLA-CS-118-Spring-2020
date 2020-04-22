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
#define MAX_READ_BUF_SIZE 2048 //2KB
#define MAX_WRITE_BUF_SIZE 2097152//2MB
#define MAX_HEAD_BUF_SIZE 2048 //2KB
#define METHOD_GET "GET"
#define EMPTY_STRING ""
#define METHOD_NOT_ALLOWED "HTTP/1.0 405 Method Not Allowed\nServer: Siddharth Joshi\r\n\r\n"
#define FILE_NOT_FOUND "HTTP/1.0 404 Not Found\nServer: Siddharth Joshi\n\r\n\r\n"
#define DEFAULT "HTTP/1.0 200 OK\nServer: Siddharth Joshi\nContent-Type: text/html\nContent-Length: 13\r\n\r\nHello, world!"
#define MAX_SMALL_BUF_SIZE 100
#define OK "HTTP/1.0 200 OK\nServer: Siddharth Joshi\n"
#define CONTENT_LENGTH "Content-Length: "
#define CRLF "\r\n\r\n"
#define HTML_EXT ".html"
#define HTML_CONT_TYPE "Content-Type: text/html; charset=utf-8\n"
#define TXT_EXT ".html"
#define TXT_CONT_TYPE "Content-Type: text/plain; charset=utf-8\n"
#define JPG_EXT ".jpg"
#define JPG_CONT_TYPE "Content-Type: image/jpeg\n"
#define PNG_EXT ".png"
#define PNG_CONT_TYPE "Content-Type: image/png\n"
#define BIN_CONT_TYPE "Content-Type: application/octet-stream\n"

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
    char header_buf[MAX_HEAD_BUF_SIZE];
    char tmp_buf[MAX_WRITE_BUF_SIZE];
    char content_length_buf[MAX_SMALL_BUF_SIZE];
    int write_buf_size;
    int read_buf_size;
    int header_offset;

    while (1) 
    { 
        // main accept() loop, loop infinitely
        printf("Server is now accepting connections\n");

        //RESET BUFFERS
        memset(read_buf, 0, MAX_READ_BUF_SIZE);
        memset(write_buf, 0, MAX_WRITE_BUF_SIZE);
        memset(header_buf, 0, MAX_HEAD_BUF_SIZE);
        memset(tmp_buf, 0, MAX_WRITE_BUF_SIZE);
        memset(content_length_buf, 0, MAX_SMALL_BUF_SIZE);

        // *** Accept connection from client ***
        sin_size = sizeof(struct sockaddr_in);
        if ((client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
            perror("Failed: Accepting connection from client \n");
            continue;
        }
        printf("Received new connection from %s\n", inet_ntoa(client_addr.sin_addr));
        
        // *** Read from socket ***
        if ((read_buf_size = read(client_fd, read_buf, MAX_READ_BUF_SIZE)) <= 0)
        {
            perror("Failed: Reading message from client. Closing connection. \n");
            close(client_fd);
            continue;
        }
        printf("Message from client: \n %s", read_buf);

        // *** Parse Request ***
        const char *end_of_method = strchr(read_buf, ' ');
        const char *start_of_url = strchr(read_buf, '/') + 1;
        const char *end_of_url = strchr(end_of_method + 1, ' ');
        //DEBUG: printf("Found start/end points of strings successfully");

        char method[end_of_method - read_buf + 1];
        char url[end_of_url - start_of_url + 1];
        //DEBUG: printf("Allocated memory for strings successfully\n");

        strncpy(method, read_buf,  end_of_method - read_buf);
        strncpy(url, start_of_url, end_of_url - start_of_url);
        //DEBUG: printf("Copied strings successfully\n");    

        method[sizeof(method) - 1] = 0;
        url[sizeof(url) - 1] = 0;
        //DEBUG: printf("NULL terminated strings successfully\n"); 

        printf("Parsed request as: \n");
        printf("Method: %s\n", method);
        printf("URL: %s\n\n\n", url);

        // *** Check if method is supported i.e. Method is GET ***
        if (strcmp(method, METHOD_GET))
        {
            printf("Client used an unsupported method type. \n");
            if (write(client_fd, METHOD_NOT_ALLOWED, strlen(METHOD_NOT_ALLOWED)) < 0)
            {
                perror("Failed: Writing message to client. Closing connection. \n");
                close(client_fd);
                continue;
            }
            printf("Response to Client:\n%s\n", METHOD_NOT_ALLOWED);
            close(client_fd);
            continue;
        }

        // *** Retreive appropriate file - handle not found error ***
        FILE *fp = fopen(url, "r");
        if (!strcmp(url, EMPTY_STRING))
        {
            printf("Client requested root url: /  \n");
            if (write(client_fd, DEFAULT, strlen(DEFAULT)) < 0)
            {
                perror("Failed: Writing message to client. Closing connection. \n");
                close(client_fd);
                continue;
            }
            printf("Response to Client:\n%s\n", DEFAULT);
            close(client_fd);
            continue;
        }
        else if (!fp) //Unable to open file -> assume 404 file not found and return that
        {   
            perror("Error opening a file");
            printf("Client requested a file that doesn't exist \n");
            if (write(client_fd, FILE_NOT_FOUND, strlen(FILE_NOT_FOUND)) < 0)
            {
                perror("Failed: Writing message to client. Closing connection. \n");
                close(client_fd);
                continue;
            }
            printf("Response to Client:\n%s\n", FILE_NOT_FOUND);
            close(client_fd);
            continue;
        }
        
        // *** Read File into TMP Buffer ***
        size_t tmp_buf_size = fread(tmp_buf, sizeof(char), MAX_WRITE_BUF_SIZE, fp);
        if (ferror(fp)) 
        {      
            perror("Failed: Reading file into buffer. Closing connection. \n");
            fclose(fp);
            close(client_fd);
            continue;
        } 
        fclose(fp);
        
        // *** Construct Header ***
        header_offset = 0;

        //Set Response Status Code to OK
        strcpy(header_buf + header_offset, OK);
        header_offset += strlen(OK);
        //DEBUG: printf("Header Status Code set to OK successfully\n\n");

        //Set Content-Type
        const char *ext = strchr(url, '.');
        if (!ext) //If binary requested, there is no . 
        {
            strcpy(header_buf + header_offset, BIN_CONT_TYPE);
            header_offset += strlen(BIN_CONT_TYPE);
        }
        else if (!strcmp(ext, HTML_EXT))
        {
            strcpy(header_buf + header_offset, HTML_CONT_TYPE);
            header_offset += strlen(HTML_CONT_TYPE);
        }
        else if (!strcmp(ext, TXT_EXT))
        {
            strcpy(header_buf + header_offset, TXT_CONT_TYPE);
            header_offset += strlen(TXT_CONT_TYPE);
        }
        else if (!strcmp(ext, JPG_EXT))
        {
            strcpy(header_buf + header_offset, JPG_CONT_TYPE);
            header_offset += strlen(JPG_CONT_TYPE);
        }
        else if (!strcmp(ext, PNG_EXT))
        {
            strcpy(header_buf + header_offset, PNG_CONT_TYPE);
            header_offset += strlen(PNG_CONT_TYPE);
        }
        else //If file type not supported, send as binary
        {
            strcpy(header_buf + header_offset, BIN_CONT_TYPE);
            header_offset += strlen(BIN_CONT_TYPE);
        }
        //DEBUG: printf("Extension determined and copied into header\n\n");

        //Set Content-Length
        strcpy(header_buf + header_offset, CONTENT_LENGTH);
        header_offset += strlen(CONTENT_LENGTH);
        sprintf(content_length_buf, "%lu", tmp_buf_size);
        strcpy(header_buf + header_offset, content_length_buf);
        header_offset += strlen(content_length_buf);

        //Append CRLF to Header
        strcpy(header_buf + header_offset, CRLF);

        // *** Merge Header and Body into Write Buffer *** //
        strcpy(write_buf, header_buf);
        memcpy(write_buf + strlen(header_buf), tmp_buf, tmp_buf_size);
        write_buf_size = tmp_buf_size + strlen(header_buf);

        // *** Write to socket ***
        if (write(client_fd, write_buf, write_buf_size) < 0)
        {
            perror("Failed: Writing message to client. Closing connection. \n");
            close(client_fd);
            continue;
        }
        printf("Response to client:\n\n%s\n", write_buf);
        
        // *** Close connection ***
        close(client_fd);
        printf("Connection closed. \n\n");
    }
}
