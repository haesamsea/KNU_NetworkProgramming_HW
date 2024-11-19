#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 9190
#define BUFFER_SIZE 2048

int main() {
    int server_fd, client_fd[2] = {-1, -1}, max_fd;
    struct sockaddr_in address;
    fd_set read_fds, active_fds;
    char buffer[BUFFER_SIZE];
    int addrlen = sizeof(address);

    // Step 1: Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    FD_ZERO(&active_fds);
    FD_SET(server_fd, &active_fds);
    max_fd = server_fd;

    while (1) {
        read_fds = active_fds;

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Select error");
            break;
        }

        // Step 2: Accept new connections
        if (FD_ISSET(server_fd, &read_fds)) {
            for (int i = 0; i < 2; ++i) {
                if (client_fd[i] == -1) {
                    client_fd[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    if (client_fd[i] < 0) {
                        perror("Accept failed");
                    } else {
                        FD_SET(client_fd[i], &active_fds);
                        if (client_fd[i] > max_fd) max_fd = client_fd[i];
                        printf("Connected client %d\n", client_fd[i]);
                    }
                    break;
                }
            }
        }

        // Step 3: Forward data between clients
        for (int i = 0; i < 2; ++i) {
            int other = 1 - i; // Get the other client
            if (client_fd[i] != -1 && FD_ISSET(client_fd[i], &read_fds)) {
                int valread = read(client_fd[i], buffer, BUFFER_SIZE);
                if (valread > 0) {
                    buffer[valread] = '\0';
                    if (client_fd[other] != -1) {
                        send(client_fd[other], buffer, valread, 0);
                        printf("Forward [%d] ---> [%d]\n", client_fd[i], client_fd[other]);
                        printf("Backward [%d] <--- [%d]\n", client_fd[i], client_fd[other]);
                    }
                } else if (valread == 0) {
                    printf("Closed client: %d\n", client_fd[i]);
                    close(client_fd[i]);
                    FD_CLR(client_fd[i], &active_fds);
                    client_fd[i] = -1;
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
