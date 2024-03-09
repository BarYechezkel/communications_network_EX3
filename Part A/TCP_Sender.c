#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <stdlib.h>
#include <netinet/tcp.h>
#include <time.h>

/*
 * @brief The buffer size to store the received message.
 * @note The default buffer size is 1024.
 */
#define BUFFER_SIZE 1024
#define FILE_SIZE 2097152 // 2MB

/*
 * @brief A random data generator function based on srand() and rand().
 * @param size The size of the data to generate (up to 2^32 bytes).
 * @return A pointer to the buffer.
 */
char *util_generate_random_data(unsigned int size)
{
    char *buffer = NULL;

    // Argument check.
    if (size == 0)
        return NULL;

    buffer = (char *)calloc(size, sizeof(char));

    // Error checking.
    if (buffer == NULL)
        return NULL;

    // Randomize the seed of the random number generator.
    srand(time(NULL));

    for (unsigned int i = 0; i < size; i++)
        *(buffer + i) = ((unsigned int)rand() % 255 + 1);

    return buffer;
}

/*
 * TCP Sender main function.
 * return 0 if the Sender runs successfully, 1 otherwise.
 */
int main(int argc, char *argv[])
{
    // check if the number of arguments that we get from command line is correcct
    if (argc != 7)
    {
        fprintf(stderr, "Usage: %s -ip <IP> -p <PORT> -algo <ALGO>\n", argv[0]);
        return 1;
    }
    ///////paramters
    char *RECIEVER_IP = argv[2];
    int RECIEVER_PORT = atoi(argv[4]);
    char *algo = argv[6];

    // The variable to store the socket file descriptor.
    // uniq number for each socket
    int sock = -1;

    // The variable to store the receiver's address.
    struct sockaddr_in receiver;

    // Create a file to send to the receiver.
    //////////////////////////////////////////////////////////////////////////////////////
    char *file = util_generate_random_data(FILE_SIZE);
    //////////////////////////////////////////////////////////////////////////////////////

    // Reset the receiver structure to zeros.
    memset(&receiver, 0, sizeof(receiver));

    // Try to create a TCP socket (IPv4, stream-based, default protocol).
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Set the socket option to reuse the receiver's address.
    // This is useful to avoid the "Address already in use" error message when restarting the receiver.
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0)
    {
        perror("setsockopt(2)");
        close(sock);
        return 1;
    }

    // If the socket creation failed, print an error message and return 1.
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

    // Convert the receiver's address from text to binary form and store it in the receiver structure.
    // This should not fail if the address is valid (e.g. "127.0.0.1").
    if (inet_pton(AF_INET, RECIEVER_IP, &receiver.sin_addr) <= 0)
    {
        perror("inet_pton(3)");
        close(sock);
        return 1;
    }

    // Set the receiver's address family to AF_INET (IPv4).
    receiver.sin_family = AF_INET;

    // Set the receiver's port to the defined port. Note that the port must be in network byte order,
    // so we first convert it to network byte order using the htons function.
    receiver.sin_port = htons(RECIEVER_PORT);

    fprintf(stdout, "Connecting to %s:%d...\n", RECIEVER_IP, RECIEVER_PORT);

    // Try to connect to the receiver using the socket and the receiver structure.
    if (connect(sock, (struct sockaddr *)&receiver, sizeof(receiver)) < 0)
    {
        perror("connect(2)");
        close(sock);
        return 1;
    }

    fprintf(stdout, "Successfully connected to the receiver!\n"
                    "Sending message to the receiver: \n");

    int decision = 0;
    do
    {
        // Try to send the message to the receiver using the socket.
        int bytes_sent = send(sock, file, strlen(file) + 1, 0);

        // If the message sending failed, print an error message and return 1.
        // If no data was sent, print an error message and return 1. Only occurs if the connection was closed.
        if (bytes_sent <= 0)
        {
            perror("send(2)");
            close(sock);
            return 1;
        }

        fprintf(stdout, "Sent %d bytes to the receiver!\n",bytes_sent);

        printf("Do you want to send the file again?\n   No - press 0\n   Yes- press 1\n");
        scanf("%d", &decision);
    } while (decision);

    char *exit_message = "Sender close the connection";
    // Try to send the message to the receiver using the socket.
    int bytes_exit_message = send(sock, exit_message, strlen(exit_message) + 1, 0);

    // If the message sending failed, print an error message and return 1.
    // If no data was sent, print an error message and return 1. Only occurs if the connection was closed.
    if (bytes_exit_message <= 0)
    {
        perror("send(5)");
        close(sock);
        return 1;
    }

    // Close the socket with the receiver.
    close(sock);

    fprintf(stdout, "Connection closed!\n");

    // Return 0 to indicate that the sender ran successfully.
    return 0;
}