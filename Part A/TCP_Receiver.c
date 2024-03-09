#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <time.h>
#include <netinet/tcp.h>
#include <stdlib.h>

#define CLOCKS_PER_SEC_B ((__clock_t)1000000)

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

void print_statistics(FILE *stats)
{
    printf("----------------------------------\n");
    printf("- * Statistics * -\n");
    rewind(stats);
    char print_buffer[1024];
    while (fgets(print_buffer, 1024, stats) != NULL)
    {
        printf("%s", print_buffer);
    }
}

/*
 * @brief TCP Receiver main function.
 * @param None
 * @return 0 if the Receiver runs successfully, 1 otherwise.
 */
int main(int argc, char *argv[])
{
    // check if the number of arguments that we get from command line is correcct
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s -p <PORT> -algo <ALGO>\n", argv[0]);
        return 1;
    }

    ///////paramters
    clock_t start, end;
    double time_taken;
    int RECEIVER_PORT = atoi(argv[2]);
    char *algo = argv[4];

    int sock = -1; // The variable to store the socket file descriptor.

    struct sockaddr_in receiver; // The variable to store the receiver's address.

    struct sockaddr_in sender; // The variable to store the sender's address.

    socklen_t sender_len = sizeof(sender); // Stores the sender's structure length.

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
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0)
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

    fprintf(stdout, "Stating Receiver... \n");
    printf("Waiting for TCP connection...\n");

    // The receiver's main loop.
    // Try to accept a new sender connection.
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

    int receiver_listen = 1;
    char *exit_message = "Sender close the connection";

    FILE *stats_file = fopen("stats", "w+");
    if (stats_file == NULL)
    {
        printf("error when open file");
        return 1;
    }
    int run = 1;
    double total_avg_time = 0, total_avg_speed = 0;
    while (receiver_listen)
    {
        size_t total_recv = 0; // the total bytes received so far
        start = clock();       // start measuring the time

        // check if the exit message was received
        int bytes_received = recv(sender_sock, buffer, sizeof(buffer), 0);
        total_recv += bytes_received;
        if (bytes_received == -1)
        { // check for errors
            perror("recv");
            return 1;
        }

        if (strncmp(buffer, exit_message, strlen(exit_message) + 1) == 0)
        {
            receiver_listen = 0;
            printf("exit message received\n");
            break;
        }

        // Receive a message from the sender and store it in the buffer.
        while (total_recv < BUFFER_SIZE)
        {
            int bytes_received = recv(sender_sock, buffer + total_recv, BUFFER_SIZE - total_recv, 0);
            total_recv += bytes_received;
            if (bytes_received == -1)
            {
                perror("recv(2)");
                close(sender_sock);
                close(sock);
                return 1;
            }

            // If the amount of received bytes is 0, the sender has disconnected.
            // Close the receiver's socket and continue to the next iteration.
            else if (bytes_received == 0)
            {
                fprintf(stdout, "Sender %s:%d disconnected\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
                close(sender_sock);
                receiver_listen = 0;
                break;
            }
        }
        end = clock();

        if (receiver_listen)
        {
            printf("File transfer completed.\n");
            // Calculate time taken and average bandwidth
            time_taken = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC_B;                      // in milliseconds
            double average_bandwidth = (total_recv / (1024.0 * 1024.0)) / (time_taken / 1000.0); // in MB/s
            fprintf(stats_file, "Run #%d Data: Time=%f S ; Speed=%f MB/S\n", run, time_taken, average_bandwidth);

            total_avg_time += time_taken;
            total_avg_speed += average_bandwidth;
            run++;
        }

    }

    fprintf(stats_file," Average time: %f S\n", total_avg_time/(run-1));
    fprintf(stats_file," Average speed: %f S\n", total_avg_speed/(run-1));
    
    print_statistics(stats_file);

    // Close the file
    fclose(stats_file);
    remove("stats");
    // Close the sender's socket and continue to the next iteration.
    close(sender_sock);
    fprintf(stdout, "Sender %s:%d disconnected\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
    fprintf(stdout, "Receiver finished!\n");

    return 0;
}