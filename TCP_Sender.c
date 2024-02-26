#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 1024
#define FILE_SIZE 2097152  // 2MB

void print_statistics(double time_taken, double average_bandwidth) {
    printf("----------------------------------\n");
    printf("- * Statistics * -\n");
    printf("- Run #1 Data: Time=%.1fms; Speed=%.2fMB/s\n", time_taken, average_bandwidth);
    printf("- Average time: %.1fms\n", time_taken);
    printf("- Average bandwidth: %.2fMB/s\n", average_bandwidth);
    printf("----------------------------------\n");
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s -ip <IP> -p <PORT> -algo <ALGO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip;
    int port;
    const char *algorithm;
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-ip") == 0) {
            ip = argv[i + 1];
        } else if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-algo") == 0) {
            algorithm = argv[i + 1];
        }
    }

    printf("Starting Sender...\n");

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);

            printf("Starting Sender...\n");

    }

    // Set congestion control algorithm
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, algorithm, strlen(algorithm)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);


            printf("Starting Sender.2..\n");

    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr)); // Initialize servaddr with zeros
    printf("Starting Sender3...\n");

    // Assign IP and port
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);
    printf("Starting Sender4...\n");

    // Connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
                            printf("Starting Sender5...\n");

        perror("connection with the server failed");
        close(sockfd);

        exit(EXIT_FAILURE);

    }

    printf("Connected to Receiver, beginning to send file...\n");

    clock_t start, end;
    double time_taken;
    int total_bytes_sent = 0;
            printf("Starting Sender6...\n");

    char *data = (char *)malloc(FILE_SIZE * sizeof(char));
    // Populate 'data' with random bytes, or read from a file

    // Send data
    start = clock();
    int bytes_sent = send(sockfd, data, FILE_SIZE, 0);
    end = clock();
            printf("Starting Sender7...\n");

    if (bytes_sent < 0) {
        perror("send failed");
        close(sockfd);
        free(data);
        exit(EXIT_FAILURE);
                    printf("Starting Sender8...\n");

    }

    total_bytes_sent += bytes_sent;

    printf("File transfer completed.\n");

    // Send exit message
    send(sockfd, "exit", 4, 0);

    printf("Waiting for Receiver response...\n");

    // Calculate time taken and average bandwidth
    time_taken = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC; // in milliseconds
    double average_bandwidth = (total_bytes_sent / (1024.0 * 1024.0)) / (time_taken / 1000.0); // in MB/s
            printf("Starting Sender9...\n");

    // Print statistics
    print_statistics(time_taken, average_bandwidth);

    // Clean up
    close(sockfd);
    free(data);

    printf("Sender end.\n");

    return 0;
}
