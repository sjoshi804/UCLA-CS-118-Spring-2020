#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 5000

int main()
{
 // *** Initialize Socket ***
 int sockfd;
 if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
   perror("Cannot create socket");
   exit(1);
 }

 // *** Initialize the server socket address ***
 struct sockaddr_in server_addr; // server socket address struct
 server_addr.sin_family = AF_INET; // protocol family
 server_addr.sin_port = htons(PORT); // port number
 struct hostent *host_name = gethostbyname("localhost"); // get IP from host name
 server_addr.sin_addr = *((struct in_addr *)host_name->h_addr); // set IP address
 memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero); // make the rest bytes zero

if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
    perror("Cannot connect");
    exit(1);
}

 // *** Socket Read & Write ***
 int recvline_size;
 char sendline[1024], recvline[1024];
 while (fgets(sendline, 1024, stdin) != NULL) {
   //Write to connection
   write(sockfd, sendline, strlen(sendline));

   memset(recvline, 0, 1024);
   recvline_size = read(sockfd, recvline, 1024);

   printf("String received from the server: %.*s", recvline_size, recvline);

   close(sockfd);
   exit(0);
 }
 return 0;
}

