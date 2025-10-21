#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[2048];
    char command[1024];

    //Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Setup server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4/IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", SERVER_IP, PORT);
    printf("Type commands (INSERT <id> <name>, DELETE <id>, READ, EXIT):\n\n");

    // Example: automatic test sequence (you can comment this out for manual)
    // Sends multiple commands automatically
    char *commands[] = {
        "INSERT 1 Alice\n",
        "INSERT 2 Bob\n",
        "READ\n",
        "DELETE 1\n",
        "READ\n",
        "EXIT\n"
    };
    int n_commands = sizeof(commands) / sizeof(commands[0]);

    for (int i = 0; i < n_commands; i++) {
        printf(">>> %s", commands[i]);
        send(sock, commands[i], strlen(commands[i]), 0);

        // receive server response
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        printf("%s\n", buffer);

        sleep(1); // small delay for clarity
    }

    close(sock);
    printf("Disconnected from server.\n");
    return 0;
}

