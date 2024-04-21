#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <stdlib.h>
#include <netinet/tcp.h>
#include <time.h>

#define SYN 1
#define ACK 2
#define DATA 3
#define FIN 4
#define FIN_ACK 5
#define END 6


#define Buffer 65000



typedef struct RUDP_header{ 

    int length_data;
    int sequence_number;
    int checksum;
    int flags;

}header , *pheader;



//creating RUDP socket && handshake
int rudp_sockets();


//Sending data to the peer. The function should wait for an 
//acknowledgment packet, and if it didn’t receive any, retransmits the data.
int rudp_send(int sock, const void *user_data, size_t size_D, const struct sockaddr *reciver_address);


int RUDP_connect_sender(int sock, char* ip ,int port);

int RUDP_connect_reciever(int sock, int port);



//Receive data from a peer.
int rudp_recv(int sock, int data_size);

//Closes a connection between peers. 
int rudp_close(int sock);


int send_ack(int socket, header packet);

int wait_for_ACK(int socket, int seq_num, clock_t start_time, int timeout);

int wait_for_FIN_ACK(int socket, int seq_num, clock_t start_time, int timeout);


