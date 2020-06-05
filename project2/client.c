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
#define FLAG_SYN 1
#define FLAG_ACK 2
#define FLAG_FIN 4

int main(int argc, char **argv) {

    if (argc != 4)
    {
        printf("Incorrect usage. Please enter ./client HOST_NAME PORT_NUM FILE_NAME");
        exit(1);
    }

	int sockfd; 
	unsigned char read_buffer[MAX_BUFF_SIZE]; 
	unsigned char write_buffer[MAX_BUFF_SIZE];
    char recv_log_buffer[MAX_BUFF_SIZE];
    time_t random_seed;
	struct sockaddr_in servaddr; 

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(atoi(argv[2])); //Second argument is port
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	
    socklen_t len; 
	
    //Clear Read, Write Buffers
    memset(read_buffer, 0, MAX_BUFF_SIZE);
    memset(write_buffer, 0, MAX_BUFF_SIZE);

    //Set Random Seed
    srand((unsigned) time(&random_seed) + getpid());

    // !!!!!!!!! Establish Connection !!!!!!!!!
    // ****** SEND SYN Packet, ACK # = 0, SEQ # = random number under 25601 ******
    //Contruct Message
    short send_seq_num = rand() % 25601;
    short send_ack_num = 0;
    short datagram_len = 0;
    //Write SEQ NUM and ACK NUM to datagram
    write_buffer[0] = (send_seq_num >> 8) & 0xff;
    write_buffer[1] = send_seq_num & 0xff;
    write_buffer[2] = (send_ack_num >> 8) & 0xff;
    write_buffer[3] = send_ack_num & 0xff;
    //Set SYN Flag
    write_buffer[4] = FLAG_SYN;
    //Write datagram length
    write_buffer[5] = (datagram_len >> 8) & 0xff;
    write_buffer[6] = datagram_len & 0xff;
    //No Data, Hence Message complete
    //Send Packet
	sendto(sockfd, write_buffer, MAX_BUFF_SIZE, 
    0, (const struct sockaddr *) &servaddr, 
			sizeof(servaddr)); 
    //Clean up buffer
    memset(write_buffer, 0, MAX_BUFF_SIZE);
    //Log Message
	printf("SEND %d %d SYN\n", send_seq_num, send_ack_num);
    //Increment sequence number
	send_seq_num = (send_seq_num + 1) % 25601;

    // ****** Receive SYN ACK Datagram ******
	recvfrom(sockfd, read_buffer, MAX_BUFF_SIZE, 
				MSG_WAITALL, (struct sockaddr *) &servaddr, 
				&len); 
    short recv_seq_num = (read_buffer[0] << 8) + read_buffer[1];
    short recv_ack_num = (read_buffer[2] << 8) + read_buffer[3];
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
    printf("%s\n", recv_log_buffer);
    //Clean up buffer
    memset(read_buffer, 0, MAX_BUFF_SIZE);

    // ******* Open file for sending ******
    int bytes_sent = 0;
    FILE *fp = fopen(argv[3], "r");
    if (!fp)
    {
        perror("Error opening a file");
        close(sockfd);
        exit(1);
    }

    // !!!!! Loop until file sent completely !!!!!!
    ssize_t bytes_file_read = fread(write_buffer + 12, sizeof(char), MAX_PAYLOAD_SIZE, fp);
    int window_used = 0;
    while(bytes_file_read > 0)
    {
        if (window_used >= 10)
        {
            //Wait for message from server
            recvfrom(sockfd, read_buffer, MAX_BUFF_SIZE, 
				MSG_WAITALL, (struct sockaddr *) &servaddr, 
				&len); 
            recv_seq_num = (read_buffer[0] << 8) + read_buffer[1];
            recv_ack_num = (read_buffer[2] << 8) + read_buffer[3];
            recv_syn_flag = read_buffer[4] & 1;
            recv_ack_flag = (read_buffer[4] >> 1) & 1;
            recv_fin_flag = (read_buffer[4] >> 2) & 1;
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
            printf("%s\n", recv_log_buffer);
            //Clean up buffer
            memset(read_buffer, 0, MAX_BUFF_SIZE);

            //Determine how many unacked packets remain and that is the window used now
            window_used = (send_seq_num + 512 - recv_ack_num) / 512;
        }

        // ***** Send  Data Packet *****
        //Contruct Message
        send_seq_num += bytes_sent;
        send_ack_num = recv_seq_num + 1;
        datagram_len = bytes_file_read;
        //Write SEQ NUM and ACK NUM to datagram
        write_buffer[0] = (send_seq_num >> 8) & 0xff;
        write_buffer[1] = send_seq_num & 0xff;
        write_buffer[2] = (send_ack_num >> 8) & 0xff;
        write_buffer[3] = send_ack_num & 0xff;
        //Write datagram length
        write_buffer[5] = (datagram_len >> 8) & 0xff;
        write_buffer[6] = datagram_len & 0xff;
        //Send Packet
        sendto(sockfd, write_buffer, MAX_BUFF_SIZE, 
        0, (const struct sockaddr *) &servaddr, 
                sizeof(servaddr)); 
        //Clean up buffer
        memset(write_buffer, 0, MAX_BUFF_SIZE);
        //Incremenet Bytes sent
        bytes_sent += bytes_file_read;
        //Increment window used
        window_used += 1;
        //Log Message
        printf("SEND %d %d SYN\n", send_seq_num, send_ack_num);

        //Read data from file into write buffer for next packet
        bytes_file_read = fread(write_buffer + 12, sizeof(char), MAX_PAYLOAD_SIZE, fp + bytes_sent);
    }
    

    //!!!!!! Close connection !!!!!!!

    //***** Send Fin Message *****
    //Contruct Message
    send_seq_num += bytes_sent;
    send_ack_num = 0;
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
    0, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr)); 
    //Log Message
    printf("SEND %d %d FIN\n", send_seq_num, send_ack_num);

    //***** Loop Until Fin Message Received
    char fin_msg_recvd = 0;
    while(!fin_msg_recvd)
    {
        //Wait for message from server
        recvfrom(sockfd, read_buffer, MAX_BUFF_SIZE, 
            MSG_WAITALL, (struct sockaddr *) &servaddr, 
            &len); 
        recv_seq_num = (read_buffer[0] << 8) + read_buffer[1];
        recv_ack_num = (read_buffer[2] << 8) + read_buffer[3];
        recv_syn_flag = read_buffer[4] & 1;
        recv_ack_flag = (read_buffer[4] >> 1) & 1;
        recv_fin_flag = (read_buffer[4] >> 2) & 1;
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
        //Clean up buffer
        memset(read_buffer, 0, MAX_BUFF_SIZE);
    }

    //**** ACK the FIN message ******
    //Contruct Message
    send_seq_num += bytes_sent;
    send_ack_num = 0;
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
    0, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr)); 
    //Log Message
    printf("SEND %d %d ACK\n", send_seq_num, send_ack_num);

    //Close socket and exit
	close(sockfd);
	return 0; 
} 
