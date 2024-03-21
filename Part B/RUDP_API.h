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
int rudp_send();

//Receive data from a peer.
int rudp_recv();

//Closes a connection between peers. 
int rudp_close();
