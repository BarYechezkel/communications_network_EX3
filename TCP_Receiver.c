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
    if (argc != 5) {
        fprintf(stderr, "Usage: %s -p <PORT> -algo <ALGO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
    const char *algorithm = argv[4];

    printf("Starting Receiver...\n");

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set congestion control algorithm
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, algorithm, strlen(algorithm)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr)); // Initialize servaddr with zeros

    // Assign IP and port
    servaddr.sin_family = AF_INET;
    //
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // Bind the socket with the server address
    if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if ((listen(sockfd, 5)) != 0) {
        perror("listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for TCP connection...\n");

    while (1) {
        socklen_t len = sizeof(cliaddr);
        printf("alalalalalalalalalala");
        // Accept the data packet from client
        int connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        if (connfd < 0) {
            printf("dfgdfgfdfgfgdg");
            perror("server accept failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("tertetgdgegergggggggggggggggggggggggggggggggg");
        printf("Sender connected, beginning to receive file...\n");

        clock_t start, end;
        double time_taken;
        int total_bytes_received = 0;

        // Receive data
        char buffer[BUFFER_SIZE];
        start = clock();
        int bytes_received = recv(connfd, buffer, BUFFER_SIZE, 0);
        end = clock();

        if (bytes_received < 0) {
            perror("recv failed");
            close(connfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        total_bytes_received += bytes_received;

        printf("File transfer completed.\n");
        printf("Waiting for Sender response...\n");

        // Receive exit message
        bytes_received = recv(connfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("recv failed");
            close(connfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        printf("Sender sent exit message.\n");

        // Calculate time taken and average bandwidth
        time_taken = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC; // in milliseconds
        double average_bandwidth = (total_bytes_received / (1024.0 * 1024.0)) / (time_taken / 1000.0); // in MB/s

        // Print statistics
        print_statistics(time_taken, average_bandwidth);

        close(connfd);
    }

    // Clean up
    close(sockfd);

    printf("Receiver end.\n");

    return 0;
}
