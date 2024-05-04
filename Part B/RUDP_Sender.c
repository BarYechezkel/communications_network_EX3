#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <stdlib.h>
#include <netinet/tcp.h>
#include <time.h>
#include "RUDP_API.h"


#define TIMEOUT 1 // Timeout in seconds

/*
 * @brief The buffer size to store the received message.
 * @note The default buffer size is 1024.
 */
#define BUFFER_SIZE 1024
#define FILE_SIZE 2097152 // 2MB

char* util_generate_random_data(unsigned int size);

/*
 * @brief A random data generator function based on srand() and rand().
 * @param size The size of the data to generate (up to 2^32 bytes).
 * @return A pointer to the buffer.
 */
char* util_generate_random_data(unsigned int size)
{
    char* buffer = NULL;

    // Argument check.
    if (size == 0)
        return NULL;

    buffer = (char*)calloc(size, sizeof(char));

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
int main(int argc, char* argv[])
{
    // check if the number of arguments that we get from command line is correcct
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s -ip <IP> -p <PORT>\n", argv[0]);
        return 1;
    }
    ///////paramters
    char* RECIEVER_IP = argv[2];
    int RECIEVER_PORT = atoi(argv[4]);

    // The variable to store the socket file descriptor.
    int sock = -1;


    // Create a file to send to the receiver.
    //////////////////////////////////////////////////////////////////////////////////////
    char* file = util_generate_random_data(FILE_SIZE);
    //////////////////////////////////////////////////////////////////////////////////////



    // Try to create a RUDP socket (IPv4, stream-based, default protocol).
    sock = rudp_sockets();

    // If the socket creation failed, print an error message and return 1.
    if (sock == -1)
    {
        perror("socket(2)");
        return 1;
    }

    fprintf(stdout, "Connecting to %s:%d...\n", RECIEVER_IP, RECIEVER_PORT);

    if (RUDP_connect_sender(sock, RECIEVER_IP, RECIEVER_PORT) < 0)
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
        int bytes_sent = rudp_send(sock, file, strlen(file)+1);
        //printf("byte sent %d\n",bytes_sent);

        // If the message sending failed, print an error message and return 1.
        // If no data was sent, print an error message and return 1. Only occurs if the connection was closed.
        if (bytes_sent <= 0)
        {
            perror("send(2)");
            close(sock);
            return 1;
        }

        fprintf(stdout, "Sent %d bytes to the receiver!\n", bytes_sent);

        printf("Do you want to send the file again?\n   No - press 0\n   Yes- press 1\n");
        scanf("%d", &decision);
       
    } while (decision);


    rudp_close(sock);

    return 0;
}
