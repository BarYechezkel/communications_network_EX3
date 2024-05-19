#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <time.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include "RUDP_API.h"


/*
 * @brief The maximum number of senders that the receiver can handle.
 * @note The default maximum number of senders is 1.
 */
#define MAX_SENDERS 1

 /*
  * @brief The buffer size to store the received message.
  * @note The default buffer size is 1024.
  */
#define BUFFER_SIZE 2097152

void print_statistics(FILE* stats)
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
 * @brief RUDP Receiver main function.
 * @param None
 * @return 0 if the Receiver runs successfully, 1 otherwise.
 */
int main(int argc, char* argv[])
{
    // check if the number of arguments that we get from command line is correcct
    if (argc != 3)
    {

        fprintf(stderr, "Usage: %s -p <PORT>\n", argv[0]);
        return 1;
    }
    printf("receiver start\n");

    ///////paramters
    double time_taken = 0;
    int RECEIVER_PORT = atoi(argv[2]);


    // Try to create a RUDP socket (IPv4, stream-based, default protocol).
    int sock = rudp_sockets();
    if (sock == -1)
    {
        perror("socket(2)");
        return 1;
    }


    int connect_recieve = RUDP_connect_reciever(sock, RECEIVER_PORT);
    if (connect_recieve == -1)
    {
        perror("connect receiver error");
        return 1;
    }

    FILE* stats_file = fopen("stats", "w+");
    if (stats_file == NULL)
    {
        printf("error when open file");
        return 1;
    }
    int run = 1;
    double total_avg_time = 0, total_avg_speed = 0;
    size_t total_recv = 0; // the total bytes received so far

    int receiver_listen = 1;
    struct timeval start, end;
    while (receiver_listen)
    {
        gettimeofday(&start, NULL);
        int arr[33] = { 0 };

        // printf("%d",start);
        // check if the exit message was received
        int bytes_received = rudp_recv(sock, BUFFER_SIZE, arr);
         if (bytes_received == -1)
        { // check for errors
            perror("rudp_recv faild");
            return FAIL;
        }
        gettimeofday(&end, NULL);


        printf("byte recive:%d\n",bytes_received);
        printf("****************************************\n");
        if (bytes_received == -2)//got FIN
        {
            receiver_listen = 0;
        }
      
      
        printf("time taken %f\n",((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_usec - start.tv_usec) / 1000.0));
        

       


        if (bytes_received > 0) {
             printf("got total recv: %d\n", bytes_received);
            total_recv += bytes_received;
            // Calculate time taken and average bandwidth
            time_taken = ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_usec - start.tv_usec) / 1000.0);               // in milliseconds
            double average_bandwidth = (bytes_received / (1024.0 * 1024.0)) / (time_taken / 1000.0);
            fprintf(stats_file, "Run #%d Data: Time= %f ms ; Speed= %f MB/S\n", run, time_taken, average_bandwidth);

            total_avg_time += time_taken;
            total_avg_speed += average_bandwidth;
            run++;
        }
    }/////////////////TODO
    fprintf(stats_file, " Average time: %f ms\n", total_avg_time / (run - 1));
    fprintf(stats_file, " Average speed: %f MB/s\n", total_avg_speed / (run - 1));

    close_RUDP_recive(sock);
    print_statistics(stats_file);

    // Close the file
    fclose(stats_file);
    remove("stats");
    // Close the sender's socket and continue to the next iteration.
    fprintf(stdout, "Sender ip%s port:%d disconnected\n", "127.0.0.1", 5060);
    fprintf(stdout, "Receiver finished!\n");

    return 0;
}