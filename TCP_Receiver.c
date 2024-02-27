#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <time.h>
//##################################################################################################################
//####################################TODO
// GET THE PARAMETERS FROM COMMAND LINE
//##################################################################################################################

#define CLOCKS_PER_SEC  ((__clock_t) 1000000)

/*
 * @brief The TCP's sender port.
 * @note The default port is 5060.
 */
//#define RECEIVER_PORT 5060

/*
 * @brief The maximum number of senders that the receiver can handle.
 * @note The default maximum number of senders is 1.
 */
#define MAX_SENDERS 1

/*
 * @brief The buffer size to store the received message.
 * @note The default buffer size is 1024.
 */
// #define BUFFER_SIZE 1024
#define BUFFER_SIZE 2097152



void print_statistics(double time_taken, double average_bandwidth) {
    printf("----------------------------------\n");
    printf("- * Statistics * -\n");
    printf("- Run #1 Data: Time= %.1fms; Speed= %.2fMB/s\n", time_taken, average_bandwidth);
    printf("- Average time: %.1fms\n", time_taken);
    printf("- Average bandwidth: %.2fMB/s\n", average_bandwidth);
    printf("----------------------------------\n");
}



/*
 * @brief TCP Server main function.
 * @param None
 * @return 0 if the server runs successfully, 1 otherwise.
 */
int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Usage: %s -p <PORT> -algo <ALGO>\n", argv[0]);
        return 1;
    }

    ////////////////////////////////////////////////////////////////////
    clock_t start, end;
    double time_taken;
    int RECEIVER_PORT =  atoi(argv[2]);
    char algo = argv[4];
    //////////////////////////////////////////////////////////////////////

    // The variable to store the socket file descriptor.
    int sock = -1;

    // The variable to store the receiver's address.
    struct sockaddr_in receiver;

    // The variable to store the sender's address.
    struct sockaddr_in sender;

    // Stores the sender's structure length.
    socklen_t sender_len = sizeof(sender);

    // Create a message to send to the sender.
    char *message = "Good morning, Vietnam\n";

    // Get the message length.
    int messageLen = strlen(message) + 1;

    // The variable to store the socket option for reusing the receiver's address.
    int opt = 1;

    // Reset the receiver and sender structures to zeros.
    memset(&receiver, 0, sizeof(receiver));
    memset(&sender, 0, sizeof(sender));

    // Try to create a TCP socket (IPv4, stream-based, default protocol).
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        perror("socket(2)");
        return 1;
    }

    // Set the socket option to reuse the receiver's address.
    // This is useful to avoid the "Address already in use" error message when restarting the receiver.
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(2)");
        close(sock);
        return 1;
    }

    // Set the receiver's address to "0.0.0.0" (all IP addresses on the local machine).
    receiver.sin_addr.s_addr = INADDR_ANY;

    // Set the receiver's address family to AF_INET (IPv4).
    receiver.sin_family = AF_INET;

    // Set the receiver's port to the specified port. Note that the port must be in network byte order.
    receiver.sin_port = htons(RECEIVER_PORT);



    // Try to bind the socket to the receiver's address and port.
    if (bind(sock, (struct sockaddr *)&receiver, sizeof(receiver)) < 0)
    {
        perror("bind(2)");
        close(sock);
        return 1;
    }

    // Try to listen for incoming connections.
    if (listen(sock, MAX_SENDERS) < 0)
    {
        perror("listen(2)");
        close(sock);
        return 1;
    }

    fprintf(stdout, "Stating Receiver... port: %d\n", RECEIVER_PORT);
    printf("Waiting for TCP connection...\n");

    // The receiver's main loop.
    while (1)
    {
        // Try to accept a new client connection.
        int sender_sock = accept(sock, (struct sockaddr *)&sender, &sender_len);

        // If the accept call failed, print an error message and return 1.
        if (sender_sock < 0)
        {
            perror("accept(2)");
            close(sock);
            return 1;
        }

        // Print a message to the standard output to indicate that a new sender has connected.
        fprintf(stdout, "Sender %s:%d connected, beginning to receive file...\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));

        // Create a buffer to store the received message.
        char buffer[BUFFER_SIZE] = {0};

        // Receive a message from the sender and store it in the buffer.
        int total_recv = 0;
        start = clock();
        while (total_recv < BUFFER_SIZE)
        {
            int bytes_received = recv(sender_sock, buffer + total_recv, BUFFER_SIZE - total_recv, 0);
            total_recv += bytes_received;
            if (bytes_received < 0)
            {
                perror("recv(2)");
                close(sender_sock);
                close(sock);
                return 1;
            }
            
            // If the amount of received bytes is 0, the client has disconnected.
            // Close the receiver's socket and continue to the next iteration.
            else if (bytes_received == 0)
            {
                fprintf(stdout, "Sender %s:%d disconnected\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
                close(sender_sock);
                continue;
            }
        }
        end = clock();

        // Ensure that the buffer is null-terminated, no matter what message was received.
        // This is important to avoid SEGFAULTs when printing the buffer.
        if (buffer[BUFFER_SIZE - 1] != '\0')
            buffer[BUFFER_SIZE - 1] = '\0';
        printf("File transfer completed.\n");
        fprintf(stdout, "Received %d bytes from the sender %s:%d: \n", total_recv, inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));

        // Send back a message to the client.
        int bytes_sent = send(sender_sock, message, messageLen, 0);

        // If the message sending failed, print an error message and return 1.
        // We do not need to check for 0 bytes sent, as if the sender disconnected, we would have already closed the socket.
        if (bytes_sent < 0)
        {
            perror("send(2)");
            close(sender_sock);
            close(sock);
            return 1;
        }

        fprintf(stdout, "Sent %d bytes to the sender %s:%d!\n", bytes_sent, inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
        printf("Waiting for Sender response\n");
        // Close the sender's socket and continue to the next iteration.
        close(sender_sock);

        fprintf(stdout, "Sender %s:%d disconnected\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));


        // Calculate time taken and average bandwidth
        time_taken = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC; // in milliseconds
        double average_bandwidth = (total_recv / (1024.0 * 1024.0)) / (time_taken / 1000.0); // in MB/s

        // Print statistics
        print_statistics(time_taken, average_bandwidth);

        // TODO: check if need to stop listening
        break;
    }

    
    fprintf(stdout, "Receiver finished!\n");

    return 0;
}