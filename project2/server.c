#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>

#define MAX_BUFF_SIZE 524
#define MAX_PAYLOAD_SIZE 512
#define MAX_FILE_SIZE 10485760
#define FLAG_SYN 1
#define FLAG_ACK 2
#define FLAG_FIN 4

// Driver code 
int main(int argc, char **argv)
{ 
    if (argc != 2)
    {
        printf("Incorrect usage. Usage is ./server PORT_NUM");
        exit(1);
    }

	int sockfd; 
	unsigned char read_buffer[MAX_BUFF_SIZE]; 
	unsigned char write_buffer[MAX_BUFF_SIZE];
    char recv_log_buffer[MAX_BUFF_SIZE];
    unsigned char file_buffer[MAX_BUFF_SIZE];
    time_t random_seed;
	struct sockaddr_in servaddr, cliaddr; 
	
	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(atoi(argv[1])); 
	
	// Bind the socket with the server address 
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
			sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	socklen_t len;
	len = sizeof(cliaddr); 
    int client_count = 0;
    while(1)
    {
        //Clear Read, Write Buffers
        memset(read_buffer, 0, MAX_BUFF_SIZE);
        memset(write_buffer, 0, MAX_BUFF_SIZE);

        //Set Random Seed
        srand((unsigned) time(&random_seed) + getpid());
        //!!!! Setup Connection !!!!
        //**** Receive SYN message ******
        //Receive message
        recvfrom(sockfd, read_buffer, MAX_BUFF_SIZE, 
                    0, ( struct sockaddr *) &cliaddr, 
                    &len); 
        short recv_seq_num = (read_buffer[0] << 8) + read_buffer[1];
        short recv_ack_num = (read_buffer[2] << 8) + read_buffer[3];
        size_t recv_data_len = 0;
        char recv_syn_flag = read_buffer[4] & 1;
        char recv_ack_flag = (read_buffer[4] >> 1) & 1;
        char recv_fin_flag = (read_buffer[4] >> 2) & 1;
        //Write Log
        memset(recv_log_buffer, 0, MAX_BUFF_SIZE);
        sprintf(recv_log_buffer, "RECV %d %d", recv_seq_num, recv_ack_num);
        if (recv_syn_flag)
        {
            sprintf(recv_log_buffer + strlen(recv_log_buffer), " SYN");
        }
        if (recv_ack_flag)
        {
            sprintf(recv_log_buffer + strlen(recv_log_buffer), " ACK");
        }
        if (recv_fin_flag)
        {
            sprintf(recv_log_buffer + strlen(recv_log_buffer), " FIN");
        }
        //Clean up buffer
        memset(read_buffer, 0, MAX_BUFF_SIZE);
        //Print Log
        printf("%s\n", recv_log_buffer);

        //**** Send SYN ACK Message *****
        //Contruct Message
        short send_seq_num = rand() % 25601;
        short send_ack_num = recv_seq_num + 1;
        short datagram_len = 0;
        //Write SEQ NUM and ACK NUM to datagram
        write_buffer[0] = (send_seq_num >> 8) & 0xff;
        write_buffer[1] = send_seq_num & 0xff;
        write_buffer[2] = (send_ack_num >> 8) & 0xff;
        write_buffer[3] = send_ack_num & 0xff;
        //Set SYN and ACK Flag
        write_buffer[4] = FLAG_SYN + FLAG_ACK;
        //Write datagram length
        write_buffer[5] = (datagram_len >> 8) & 0xff;
        write_buffer[6] = datagram_len & 0xff;
        //No Data, Hence Message complete
        //Send Packet
        sendto(sockfd, write_buffer, MAX_BUFF_SIZE, 
        0, (const struct sockaddr *) &cliaddr, 
                sizeof(cliaddr)); 
        //Clean up buffer
        memset(write_buffer, 0, MAX_BUFF_SIZE);
        //Log Message
        printf("SEND %d %d SYN ACK\n", send_seq_num, send_ack_num);
        //Increment sequence number
        send_seq_num = (send_seq_num + 1) % 25601;

        //***** Loop, Read and Copy into Buffer Until Fin Message Received
        char fin_msg_recvd = 0;
        int bytes_recvd = 0;
        while(!fin_msg_recvd)
        {
            //Wait for message from server
            recvfrom(sockfd, read_buffer, MAX_BUFF_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &servaddr, 
                &len); 
            recv_seq_num = (read_buffer[0] << 8) + read_buffer[1];
            recv_ack_num = (read_buffer[2] << 8) + read_buffer[3];
            recv_syn_flag = read_buffer[4] + 1;
            recv_ack_flag = (read_buffer[4] >> 1) & 1;
            recv_fin_flag = (read_buffer[4] >> 2) & 1;
            recv_data_len = (read_buffer[5] << 8) + read_buffer[6];
            //Write Log
            memset(recv_log_buffer, 0, MAX_BUFF_SIZE);
            sprintf(recv_log_buffer, "RECV %d %d", recv_seq_num, recv_ack_num);
            if (recv_syn_flag)
            {
                sprintf(recv_log_buffer + strlen(recv_log_buffer), " SYN");
            }
            if (recv_ack_flag)
            {
                sprintf(recv_log_buffer + strlen(recv_log_buffer), " ACK");
            }
            if (recv_fin_flag)
            {
                fin_msg_recvd = 1;
                sprintf(recv_log_buffer + strlen(recv_log_buffer), " FIN");
            }
            printf("%s\n", recv_log_buffer);
            //Copy contents into file
            memcpy(file_buffer + bytes_recvd, read_buffer + 12, recv_data_len);
            //Increment bytes recvd
            bytes_recvd += recv_data_len;
            //Clean up buffer
            memset(read_buffer, 0, MAX_BUFF_SIZE);
        }

        //Open file for writing
        char file_name[6];
        memset(file_name, 0, 6);
        client_count += 1;
        //sprintf(file_name, "%d.file", client_count);
        FILE *fp = fopen("test", "w");
        if (!fp)
        {
            perror("Error opening a file");
            close(sockfd);
            exit(1);
        }

        //Write contents of file_buffer to file
        if (fwrite(file_buffer, sizeof(char), bytes_recvd, fp) < 0)
        {
            perror("Error writing to a file");
            close(sockfd);
            exit(1);
        }

        //Close file
        fclose(fp);

        //!!!!!! Close connection !!!!!!!

        //***** Send Ack for FIN Message *****
        //Contruct Message
        send_ack_num = recv_seq_num + 1;
        datagram_len = 0;
        //Write SEQ NUM and ACK NUM to datagram
        write_buffer[0] = (send_seq_num >> 8) & 0xff;
        write_buffer[1] = send_seq_num & 0xff;
        write_buffer[2] = (send_ack_num >> 8) & 0xff;
        write_buffer[3] = send_ack_num & 0xff;
        //Set ACK flag
        write_buffer[4] = FLAG_ACK;
        //Write datagram length
        write_buffer[5] = (datagram_len >> 8) & 0xff;
        write_buffer[6] = datagram_len & 0xff;
        //Send Packet
        sendto(sockfd, write_buffer, MAX_BUFF_SIZE, 
        0, (const struct sockaddr *) &cliaddr, 
                sizeof(cliaddr)); 
        //Log Message
        printf("SEND %d %d ACK\n", send_seq_num, send_ack_num);

        //***** Send FIN Message *****
        //Contruct Message
        send_ack_num = recv_seq_num + 1;
        datagram_len = 0;
        //Write SEQ NUM and ACK NUM to datagram
        write_buffer[0] = (send_seq_num >> 8) & 0xff;
        write_buffer[1] = send_seq_num & 0xff;
        write_buffer[2] = (send_ack_num >> 8) & 0xff;
        write_buffer[3] = send_ack_num & 0xff;
        //Set FIN flag
        write_buffer[4] = FLAG_FIN;
        //Write datagram length
        write_buffer[5] = (datagram_len >> 8) & 0xff;
        write_buffer[6] = datagram_len & 0xff;
        //Send Packet
        sendto(sockfd, write_buffer, MAX_BUFF_SIZE, 
        0, (const struct sockaddr *) &cliaddr, 
                sizeof(cliaddr)); 
        //Log Message
        printf("SEND %d %d FIN\n", send_seq_num, send_ack_num);

        //Wait for ACK for FIN message
        recvfrom(sockfd, read_buffer, MAX_BUFF_SIZE, 
            MSG_WAITALL, (struct sockaddr *) &servaddr, 
            &len); 
        recv_seq_num = (read_buffer[0] << 8) + read_buffer[1];
        recv_ack_num = (read_buffer[2] << 8) + read_buffer[3];
        recv_syn_flag = read_buffer[4] & 1;
        recv_ack_flag = (read_buffer[4] >> 1) & 1;
        recv_fin_flag = (read_buffer[4] >> 2) & 1;
        recv_data_len = (read_buffer[5] << 8) + read_buffer[6];
        //Write Log
        memset(recv_log_buffer, 0, MAX_BUFF_SIZE);
        sprintf(recv_log_buffer, "RECV %d %d", recv_seq_num, recv_ack_num);
        if (recv_syn_flag)
        {
            sprintf(recv_log_buffer + strlen(recv_log_buffer), " SYN");
        }
        if (recv_ack_flag)
        {
            sprintf(recv_log_buffer + strlen(recv_log_buffer), " ACK");
        }
        if (recv_fin_flag)
        {
            fin_msg_recvd = 1;
            sprintf(recv_log_buffer + strlen(recv_log_buffer), " FIN");
        }
        //Clean up buffer
        memset(read_buffer, 0, MAX_BUFF_SIZE);
    }

    close(sockfd);
	return 0; 
} 
