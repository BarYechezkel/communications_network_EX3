#include "RUDP_API.h"

#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <stdlib.h>
#include <netinet/tcp.h>
#include <time.h>




#define TIME_OUT 1 // Timeout in seconds

// creating RUDP socket
int rudp_sockets()
{
    int rudp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // open UDP socket
    if (rudp_socket == -1)
    {
        perror("fail to create socket ");
        return -1; // error
    }
    return rudp_socket; // code discriptor //////////////////
}

unsigned short int calculate_checksum(void *data, unsigned int bytes)
{
    unsigned short int *data_pointer = (unsigned short int *)data;
    unsigned int total_sum = 0;

    // Main summing loop
    while (bytes > 1)
    {
        total_sum += *data_pointer++;
        bytes -= 2;
    }

    // Add left-over byte, if any
    if (bytes > 0)
        total_sum += *((unsigned char *)data_pointer);

    // Fold 32-bit sum to 16 bits
    while (total_sum >> 16)
        total_sum = (total_sum & 0xFFFF) + (total_sum >> 16);

    return (~((unsigned short int)total_sum));
}

// Sending data to the peer. The function should wait for an
// acknowledgment packet, and if it didn’t receive any, retransmits the data.

// just to send data!
int rudp_send(int sock, const void *user_data, size_t size_D)
{
    int sq = 0;
    /////////////////////////////////////////////////

    int total_bytes_sent = 0;
    int packet_count = (size_D / Buffer);

    // calculate the size of the last packet
    int rest_data = size_D - packet_count * Buffer;

    for (int i = 0; i < packet_count; i++)
    {
        header packet;
        packet.length_data = Buffer;
        packet.flags = DATA;
        packet.sequence_number = sq++; ////////////////
        packet.checksum = 0;

        void *data_to_send = (void *)malloc(sizeof(header) + Buffer);
        if (data_to_send == NULL)
        {
            printf("FAILURE ALOCTION");
            exit(1);
        }

        // Copy header and user data into data_to_send
        memcpy(data_to_send, &packet, sizeof(header));
        memcpy(data_to_send + sizeof(header), user_data, sizeof(Buffer));
        packet.checksum = calculate_checksum(data_to_send + sizeof(header), sizeof(Buffer));
        do
        {
            // Try to send the message to the server using the created socket and the server structure.
            int bytes_sent = sendto(sock, data_to_send, sizeof(data_to_send), 0, NULL, 0);
            if (bytes_sent == -1)
            {
                printf("sendto() faild");
                return -1;
            }
            total_bytes_sent += bytes_sent;
            free(data_to_send);
        } while (wait_for_ACK(sock, packet.sequence_number, clock(), TIME_OUT) < 0);
    }

    if (rest_data > 0)
    {

        header packet;
        packet.length_data = rest_data;
        packet.flags = DATA;
        packet.sequence_number = sq++; ////////////////
        packet.checksum = 0;

        void *data_to_send = (void *)malloc(sizeof(header) + rest_data);
        if (data_to_send == NULL)
        {
            printf("FAILURE ALOCTION");
            exit(1);
        }

        // Copy header and user data into data_to_send
        memcpy(data_to_send, &packet, sizeof(header));
        memcpy(data_to_send + sizeof(header), user_data, rest_data);
        packet.checksum = calculate_checksum(data_to_send + sizeof(header), sizeof(rest_data));
        do
        {
            // Try to send the message to the server using the created socket and the server structure.
            int bytes_sent = sendto(sock, data_to_send, sizeof(data_to_send), 0, NULL, 0); /// TODO
            total_bytes_sent += bytes_sent;
            free(data_to_send);
            if (bytes_sent == -1)
            {
                printf("sendto() faild");
                return -1;
            }
            total_bytes_sent += bytes_sent;
            free(data_to_send);
        } while (wait_for_ACK(sock, packet.sequence_number, clock(), TIME_OUT) < 0);
    }
    // finish sending
    header packetEND;
    packetEND.length_data = 0;
    packetEND.flags = END;
    packetEND.sequence_number = 0;
    packetEND.checksum = 0;
    int bytes_sent = sendto(sock, &packetEND, sizeof(packetEND), 0, NULL, 0); 
    if (bytes_sent == -1)
    {
        perror("sendto() failed");
        close(sock);
        return -1;
    }
    return total_bytes_sent;
}

int RUDP_connect_reciever(int sock, int port)
{
    // Setup the server address structure.
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    int bind_Sock = bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bind_Sock == -1)
    {
        perror("bind() failed");
        close(sock);
        return -1;
    }
    // receive SYN message
    struct sockaddr_in sender_address;
    memset(&sender_address, 0, sizeof(sender_address));
     socklen_t sender_address_len = sizeof(sender_address);
    header packetRecv;
    int bytes_received = recvfrom(sock, &packetRecv, sizeof(packetRecv), 0, 
    (struct sockaddr*)&sender_address, &sender_address_len);

    if (bytes_received == EOF)
    {
        perror("recvfrom failed");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&sender_address, sizeof(sender_address)) == -1)
    {

        perror("connect() failed");
        return -1;
    }

    if (packetRecv.flags == SYN)
    {
        header send_ack;
        memset(&send_ack, 0, sizeof(send_ack));
        send_ack.flags = ACK;
        int sendResult = sendto(sock, &send_ack, sizeof(header), 0, NULL, 0);
        if (sendResult == -1)
        {
            printf("sendto() failed");
            return -1;
        }
            // set_timeout(sock, TIME_OUT * 10);

    }


    return 1;
}

int RUDP_connect_sender(int sock, char* ip ,int port)
{

// setup a timeout for the socket
//   if (set_timeout(socket, TIME_OUT) == -1) {
//     return -1;
//   }
  // Setup the server address structure.
  struct sockaddr_in reciver_address;
  memset(&reciver_address, 0, sizeof(reciver_address));
  reciver_address.sin_family = AF_INET;
  reciver_address.sin_port = htons(port);
  int rval = inet_pton(AF_INET, (char*)ip, &reciver_address.sin_addr);
  if (rval <= 0) {
    printf("inet_pton() failed");
    return -1;


    //inet_pton(AF_INET, RECIEVER_IP, &receiver.sin_addr) <= 0)
  }

    header packetSYN;
     memset(&packetSYN, 0, sizeof(packetSYN));
    packetSYN.flags = SYN;

    header packetAck;
    memset(&packetAck, 0, sizeof(packetAck));

    if (connect(sock, (struct sockaddr *)&reciver_address, sizeof(reciver_address)) == -1)
    {
        printf("connect failed");
        return -1;
    }

    int bytes_SYN_sent = sendto(sock, &packetSYN, sizeof(packetSYN), 0, NULL, 0);
    if (bytes_SYN_sent == EOF)
    {
        printf("sendto failed");
        return -1;
    }

    int bytes_received = recvfrom(sock, &packetAck, sizeof(packetAck), 0, NULL, 0);
    if (bytes_received == EOF)
    {
        printf("recvfrom failed");
        return -1;
    }
    if (packetAck.flags == ACK)
    {
        printf("connected complited\n");
        return 1;
    }
    return 1;
}

// Receive data from a peer.
int rudp_recv(int sock, int data_size)
{
    header packetRCV;
    int total_data_received = 0;

    do
    {
        // int num_of_packets_to_rcv = data_size / Buffer;
        memset(&packetRCV, 0, sizeof(packetRCV)); // put zero in header we are created
        void *data_to_recv = malloc(sizeof(packetRCV) + Buffer);
        if (data_to_recv == NULL)
        {
            printf("Memory allocation failed");
            return -1;
        }
        int bytes_received = recvfrom(sock, data_to_recv, sizeof(header) + Buffer, 0, NULL, 0);
         if (bytes_received == -1)
        {
            printf("recvfrom() FAILD");
            return -1;
        }


        int cal_check = calculate_checksum(data_to_recv + sizeof(header), sizeof(Buffer));
        memcpy(&packetRCV, data_to_recv, sizeof(packetRCV));

        // if (cal_check != packetRCV.checksum)//////////////////////TODO
        // {
        //     printf("checksum invalid");
        //     return -1;
        // }

        if (send_ack(sock, packetRCV) == -1)
        {
            printf("send_ack() faild");
        }

        total_data_received += packetRCV.length_data;
        free(data_to_recv);

    } while (packetRCV.flags == DATA);
    // int bytes_received = recvfrom(sock, &packetRCV, sizeof(header), 0, NULL, 0);
    if (packetRCV.flags == END)
    {
        printf("Received %d bytes", total_data_received);
    }

    if (packetRCV.flags == FIN)
    {
        header packetFIN_ACK;
        memset(&packetFIN_ACK, 0, sizeof(packetFIN_ACK));
        packetFIN_ACK.flags = FIN_ACK;
        int check = sendto(sock, &packetFIN_ACK, sizeof(packetFIN_ACK), 0, NULL, 0);
        if (check == -1)
        {
            printf("sendto() FAILD");
            return -1;
        }
    }

    return total_data_received;
}

// Closes a connection between peers.
int rudp_close(int sock)
{
    header packetCLOSE;
    memset(&packetCLOSE, 0, sizeof(packetCLOSE));
    packetCLOSE.flags = FIN;
    do
    {
        int check = sendto(sock, &packetCLOSE, sizeof(packetCLOSE), 0, NULL, 0);
        if (check == -1)
        {
            printf("sendto() FAILD");
            return -1;
        }
    } while (wait_for_ACK(sock, packetCLOSE.sequence_number, clock(), TIME_OUT) < 0);
    close(sock);
    printf("The connection ended successfully");
    return 1;
}

int send_ack(int socket, header packet) // for data
{
    header ack_packet;
    memset(&ack_packet, 0, sizeof(header));
    ack_packet.flags = ACK;
    //   if (packet->flags.FIN == 1) {
    //     ack_packet->flags.FIN = 1;
    //   }
    //   if (packet->flags.SYN == 1) {
    //     ack_packet->flags.SYN = 1;
    //   }
    if (packet.flags == DATA)
    {
        ack_packet.flags = ACK;
    }
    ack_packet.sequence_number = packet.sequence_number; // same sq
    // ack_packet->checksum = checksum(ack_packet);
    int sendResult = sendto(socket, &ack_packet, sizeof(header), 0, NULL, 0);
    if (sendResult == -1)
    {
        printf("sendto() failed ");
        return -1;
    }
    // free(ack_packet);
    return 1;
}

int wait_for_ACK(int socket, int seq_num, clock_t start_time, int timeout)
{
    header packetRCV;
    while ((double)(clock() - start_time) / CLOCKS_PER_SEC < timeout) /////////////
    {
        int bytes_recive = recvfrom(socket, &packetRCV, sizeof(packetRCV), 0, NULL, 0);
        if (bytes_recive == -1)
        {
            printf("recivefrom() failed");
            return -1;
        }
        if (packetRCV.sequence_number == seq_num && packetRCV.flags == ACK)
        {
            printf("got ACK for packet number: %d", seq_num);
            return 1;
        }
    }
    return -1; // timout!
}

int wait_for_FIN_ACK(int socket, int seq_num, clock_t start_time, int timeout)
{
    header packetRCV;
    while ((double)(clock() - start_time) / CLOCKS_PER_SEC < timeout) /////////////
    {
        int bytes_recive = recvfrom(socket, &packetRCV, sizeof(packetRCV), 0, NULL, 0);
        if (bytes_recive == -1)
        {
            printf("recivefrom() failed");
            return -1;
        }
        if (packetRCV.sequence_number == seq_num && packetRCV.flags == FIN_ACK)
        {
            printf("got ACK for packet number: %d", seq_num);
            return 1;
        }
    }
    return -1; // timout!
}