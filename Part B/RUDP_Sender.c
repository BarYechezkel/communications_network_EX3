#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <stdlib.h>
#include <netinet/tcp.h>
#include <time.h>

#define TIMEOUT 1  // Timeout in seconds

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
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s -ip <IP> -p <PORT>\n", argv[0]);
        return 1;
    }
    ///////paramters
    char *RECIEVER_IP = argv[1];
    int RECIEVER_PORT = atoi(argv[2]);










return 0;

}


