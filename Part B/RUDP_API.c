#include "RUDP_API.h"

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
int rudp_send(int sock, const void *user_data, size_t size_D, const struct sockaddr *reciver_address)
{
    static int sq = 1;
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
        packet.sequence_number = ~sq; ////////////////
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

        // Try to send the message to the server using the created socket and the server structure.
        int bytes_sent = sendto(sock, data_to_send, sizeof(data_to_send), 0, (struct sockaddr *)&reciver_address, sizeof(reciver_address)); /// TODO
        total_bytes_sent += bytes_sent;
        free(data_to_send);
    }
    if (rest_data > 0)
    {

        header packet;
        packet.length_data = rest_data;
        packet.flags = DATA;
        packet.sequence_number = ~sq; ////////////////
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
        // Try to send the message to the server using the created socket and the server structure.
        int bytes_sent = sendto(sock, data_to_send, sizeof(data_to_send), 0, (struct sockaddr *)&reciver_address, sizeof(reciver_address)); /// TODO
        total_bytes_sent += bytes_sent;
        free(data_to_send);
    }
    // finifh sending
    header packetFIN;
    packetFIN.length_data = 0;
    packetFIN.flags = FIN;
    packetFIN.sequence_number = 0;
    packetFIN.checksum = 0;
    int bytes_sent = sendto(sock, &packetFIN, sizeof(packetFIN), 0, (struct sockaddr *)&reciver_address, sizeof(reciver_address)); /// TODO

    return total_bytes_sent;
}

int RUDP_connect(int sock, const struct sockaddr *reciver_address)
{
    header packetSYN;
    packetSYN.length_data = 0;
    packetSYN.flags = SYN;
    packetSYN.sequence_number = 0;
    packetSYN.checksum = 0;

    header packetAck;
    packetAck.length_data;
    packetAck.flags;
    packetAck.sequence_number;
    packetAck.checksum;
    memset(&packetAck, 0, sizeof(packetAck));

    if (connect(socket, (struct sockaddr *)&reciver_address, sizeof(reciver_address)) == -1)
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
        printf("connected complited");
        return 1;
    }
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
        int cal_check= calculate_checksum(data_to_recv +sizeof(header),sizeof(Buffer));
        memcpy(&packetRCV, data_to_recv, sizeof(packetRCV));
        if(cal_check != packetRCV.checksum){
///////////////////




        }
        if(send_ack(sock ,packetRCV) == -1){
            printf("send_ack() faild");
        }

        total_data_received += packetRCV.length_data;
        free(data_to_recv);

    } while (packetRCV.flags == DATA);
    int bytes_received = recvfrom(sock, &packetRCV, sizeof(header), 0, NULL, 0);
    if (packetRCV.flags == FIN)
    {
        printf("Received %d bytes", total_data_received);
    }
    return total_data_received;
}

// Closes a connection between peers.
int rudp_close(int sock)
{
   
   

//when ack coming

   
    close(sock);

    return 0;
}



int send_ack(int socket, header packet) {
  header ack_packet;
  memset(&ack_packet, 0, sizeof(header));
  ack_packet.flags = ACK;
//   if (packet->flags.FIN == 1) {
//     ack_packet->flags.FIN = 1;
//   }
//   if (packet->flags.SYN == 1) {
//     ack_packet->flags.SYN = 1;
//   }
  if (packet.flags==DATA ) {
    ack_packet.flags == ACK;
  }
  ack_packet.sequence_number = packet.sequence_number;
  //ack_packet->checksum = checksum(ack_packet);
  int sendResult = sendto(socket, &ack_packet, sizeof(header), 0, NULL, 0);
  if (sendResult == -1) {
    printf("sendto() failed ");
    return -1;
  }
  //free(ack_packet);
  return 1;
}