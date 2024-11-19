#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>

#define BUFFER_SIZE 2048
#define TIMEOUT_SEC 3

void sender(int sock) {
    char buffer[BUFFER_SIZE];
    int file_fd = open("rfc1180.txt", O_RDONLY);
    if (file_fd < 0) {
        perror("File open error");
        return;
    }

    printf("Sender started. Reading and sending file...\n");

    while (1) {
        int bytes_read = read(file_fd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            printf("End of file reached. Sender exiting.\n");
            break;
        }
        send(sock, buffer, bytes_read, 0);
        printf("Sent: %.*s\n", bytes_read, buffer);
    }

    close(file_fd);
}

void receiver(int sock) {
    char buffer[BUFFER_SIZE];
    printf("Receiver started. Receiving and forwarding data...\n");

    while (1) {
        int bytes_read = read(sock, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            printf("Server closed connection. Receiver exiting.\n");
            break;
        }
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);
        send(sock, buffer, bytes_read, 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP> <Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip_address = argv[1];
    int port = atoi(argv[2]);
    int sock;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Choose function:\n1. Sender\n2. Receiver\n=> ");
    int choice;
    scanf("%d", &choice);

    if (choice == 1) {
        sender(sock);
    } else if (choice == 2) {
        receiver(sock);
    } else {
        printf("Invalid choice. Exiting.\n");
    }

    close(sock);
    return 0;
}
