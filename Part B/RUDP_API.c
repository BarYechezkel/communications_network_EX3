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
        return FAIL; // error
    }
    return rudp_socket; // code discriptor //////////////////
}

unsigned short int calculate_checksum(void* data, unsigned int bytes)
{
    unsigned short int* data_pointer = (unsigned short int*)data;
    unsigned int total_sum = 0;

    if (bytes == 0) { return 0; }

    // Main summing loop
    while (bytes > 1)
    {
        total_sum += *data_pointer++;
        bytes -= 2;
    }

    // Add left-over byte, if any
    if (bytes > 0)
        total_sum += *((unsigned char*)data_pointer);

    // Fold 32-bit sum to 16 bits
    while (total_sum >> 16)
        total_sum = (total_sum & 0xFFFF) + (total_sum >> 16);

    return (~((unsigned short int)total_sum));
}

// Sending data to the peer. The function should wait for an
// acknowledgment packet, and if it didn’t receive any, retransmits the data.

// just to send data!
int rudp_send(int sock, const void* user_data, size_t size_D)
{
    int sq = 0;
    /////////////////////////////////////////////////

    int total_bytes_sent = 0;
    int packet_count = (size_D / Buffer);
    //printf("packet count : %d" , packet_count);

    // calculate the size of the last packet
    int rest_data = size_D - packet_count * Buffer;


    for (int i = 0; i < packet_count; i++)
    {
        header packet;
        packet.length_data = Buffer;
        packet.flags = DATA;
        packet.sequence_number = sq++; ////////////////TODO
        memcpy(packet.data, user_data + i * Buffer, Buffer);
        packet.checksum = calculate_checksum(packet.data, packet.length_data);

        //  printf("DATA before: %s\n",packet.data);
          // printf("check in pakcet: %d\n",packet.checksum);
          // printf("calculate_checksum: %d\n",calculate_checksum(packet.data, packet.length_data));


        do
        {
            // Try to send the message to the server using the created socket and the server structure.
            int bytes_sent = sendto(sock, &packet, sizeof(header), 0, NULL, 0);

            // printf("DATA before: %s\n",packet.data);

            if (bytes_sent == -1)
            {
                printf("sendto() faild");
                // free(data_to_send);
                return FAIL;
            }
            total_bytes_sent += packet.length_data;

        } while (wait_for_ACK(sock, packet.sequence_number, clock(), TIME_OUT) < 0);
    }
    // free(data_to_send);

    if (rest_data > 0)
    {

        header packet;
        packet.length_data = rest_data;
        packet.flags = DATA;
        packet.sequence_number = sq++; ////////////////
        memcpy(packet.data, user_data + packet_count * Buffer, rest_data);
        packet.checksum = calculate_checksum(packet.data, packet.length_data);
        // printf("check sum before %d\n",packet.checksum);
        //  printf("two function: %d\n",calculate_checksum(packet.data,packet.length_data));
//printf("DATA before: %s\n",packet.data);

        // packet.checksum = calculate_checksum(data_to_send + sizeof(header), sizeof(rest_data));
        do
        {
            // Try to send the message to the server using the created socket and the server structure.
            int bytes_sent = sendto(sock, &packet, sizeof(header), 0, NULL, 0); /// TODO
            if (bytes_sent == -1)
            {
                printf("sendto() faild");
                return FAIL;
            }
            total_bytes_sent += packet.length_data;

        } while (wait_for_ACK(sock, packet.sequence_number, clock(), TIME_OUT) < 0);

    }
    // finish sending
    header packetEND;
    packetEND.length_data = 0;
    packetEND.flags = END;
    packetEND.sequence_number = 0;
    packetEND.checksum = 0;
    //printf("before send to -End");
    int bytes_sent = sendto(sock, &packetEND, sizeof(header), 0, NULL, 0);
    printf("after send to -End\n");
    if (bytes_sent == -1)
    {
        perror("sendto() failed");
        close(sock);
        return FAIL;
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
    int bind_Sock = bind(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bind_Sock == -1)
    {
        perror("bind() failed");
        close(sock);
        return FAIL;
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
        return FAIL;
    }

    if (connect(sock, (struct sockaddr*)&sender_address, sizeof(sender_address)) == -1)
    {

        perror("connect() failed");
        return FAIL;
    }
    // Print a message to the standard output to indicate that a new sender has connected.
    fprintf(stdout, "Sender %s:%d connected, beginning to receive file...\n", inet_ntoa(sender_address.sin_addr), ntohs(sender_address.sin_port));


    if (packetRecv.flags == SYN)
    {
        header send_ack;
        memset(&send_ack, 0, sizeof(send_ack));
        send_ack.flags = ACK;
        int sendResult = sendto(sock, &send_ack, sizeof(header), 0, NULL, 0);
        if (sendResult == -1)
        {
            printf("sendto() failed");
            return FAIL;
        }
        // set_timeout(sock, TIME_OUT * 10);

    }

    return 1;
}

int RUDP_connect_sender(int sock, char* ip, int port)
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
        return FAIL;
    }

    header packetSYN;
    memset(&packetSYN, 0, sizeof(packetSYN));
    packetSYN.flags = SYN;

    header packetAck;
    memset(&packetAck, 0, sizeof(packetAck));

    if (connect(sock, (struct sockaddr*)&reciver_address, sizeof(reciver_address)) == -1)
    {
        printf("connect failed");
        return FAIL;
    }

    int bytes_SYN_sent = sendto(sock, &packetSYN, sizeof(packetSYN), 0, NULL, 0);
    if (bytes_SYN_sent == EOF)
    {
        printf("sendto failed");
        return FAIL;
    }

    int bytes_received = recvfrom(sock, &packetAck, sizeof(packetAck), 0, NULL, 0);
    if (bytes_received == EOF)
    {
        printf("recvfrom failed");
        return FAIL;
    }
    if (packetAck.flags == ACK)
    {
        printf("connected complited\n");
        return SUCCESS;
    }
    return SUCCESS;
}

//Receive data from a peer.
int rudp_recv(int sock, int data_size)
{
    int total_data_received = 0;
    header packetRCV;
    int i = 0;//count 
    do
    {
        memset(&packetRCV, 0, sizeof(packetRCV)); // put zero in header we are created
        int bytes_received = recvfrom(sock, &packetRCV, sizeof(header), 0, NULL, 0);
        if (bytes_received == -1)
        {
            printf("recvfrom() FAILD");
            return FAIL;
        }


        //int cal_check = calculate_checksum(data_to_recv + sizeof(header), sizeof(Buffer));////TODO
        // printf("i got %d\n", packetRCV.flags);
        // printf("i got length data of packet%d\n", packetRCV.length_data);
        // printf("i got sq%d\n", packetRCV.sequence_number);

     //   printf("DATA after: %s\n",packetRCV.data);
        // printf("lenght: %d\n",packetRCV.length_data);
        // printf("one header:%d\n",packetRCV.checksum);
        // printf("two function: %d\n",calculate_checksum(packetRCV.data,packetRCV.length_data));

       



        if (packetRCV.checksum == calculate_checksum(packetRCV.data, packetRCV.length_data))
        {

            if (send_ack(sock, packetRCV) == -1)
            {
                printf("send_ack() faild");
                //return FAIL;/////////////////TODO
            }
        }
        else
        {
            printf("checksum invalid");
            return FAIL;
        }


        total_data_received = total_data_received + packetRCV.length_data;

        printf("length data %d number %d\n", packetRCV.length_data, i++);
        if (packetRCV.flags == END)
        {
            printf("i got END of packet num %d", i);
        }

    } while (packetRCV.flags == DATA);
    // int bytes_received = recvfrom(sock, &packetRCV, sizeof(header), 0, NULL, 0);

    if (packetRCV.flags == FIN)
    {
        printf("i got FINnnnnnnnnnn");

        header packetFIN_ACK;
        memset(&packetFIN_ACK, 0, sizeof(packetFIN_ACK));
        packetFIN_ACK.flags = FIN_ACK;
        int check = sendto(sock, &packetFIN_ACK, sizeof(header), 0, NULL, 0);
        if (check == -1)
        {
            printf("sendto() FAILD\n");
            return FAIL;
        }

        close(sock);


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
        int check = sendto(sock, &packetCLOSE, sizeof(header), 0, NULL, 0);
        if (check == -1)
        {
            printf("sendto() FAILD");
            return FAIL;
        }
    } while (wait_for_FIN_ACK(sock, packetCLOSE.sequence_number, clock(), TIME_OUT) < 0);
    close(sock);
    printf("The connection ended successfully\n");
    return SUCCESS;
}

int send_ack(int socket, header packet) // for data
{
    header ack_packet;
    memset(&ack_packet, 0, sizeof(header));

    if (packet.flags == DATA)
    {
        ack_packet.flags = ACK;

        ack_packet.sequence_number = packet.sequence_number; // same sq
        // ack_packet->checksum = checksum(ack_packet);
        int sendResult = sendto(socket, &ack_packet, sizeof(header), 0, NULL, 0);
        if (sendResult == -1)
        {
            printf("sendto() failed ");
            return FAIL;
        }
    }
    return SUCCESS;
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
            return FAIL;
        }
        if (packetRCV.sequence_number == seq_num && packetRCV.flags == ACK)
        {
            printf("got ACK for packet number: %d\n", seq_num);
            return SUCCESS;
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
            return FAIL;
        }
        if (packetRCV.sequence_number == seq_num && packetRCV.flags == FIN_ACK)
        {
            printf("got FINACK for packet number: %d\n", seq_num);
            return SUCCESS;
        }
    }
    return FAIL; // timout!
}