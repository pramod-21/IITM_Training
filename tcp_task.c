#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define MAX_RECORDS 100

// Record structure
typedef struct {
    int id;
    char name[50];
} Record;

Record records[MAX_RECORDS];
int record_count = 0;
pthread_mutex_t lock;


void insert_record(int id, const char *name, char *response) {
    pthread_mutex_lock(&lock);

    if (record_count >= MAX_RECORDS) {
        sprintf(response, "ERROR: Record list full.\n");
        pthread_mutex_unlock(&lock);
        return;
    }

    // check if id already exists
    for (int i = 0; i < record_count; i++) {
        if (records[i].id == id) {
            sprintf(response, "ERROR: Record with ID %d already exists.\n", id);
            pthread_mutex_unlock(&lock);
            return;
        }
    }

    records[record_count].id = id;
    strncpy(records[record_count].name, name, sizeof(records[record_count].name) - 1);
    record_count++;

    sprintf(response, "INSERT OK (id=%d, name=%s)\n", id, name);
    pthread_mutex_unlock(&lock);
}

void delete_record(int id, char *response) {
    pthread_mutex_lock(&lock);

    int found = 0;
    for (int i = 0; i < record_count; i++) {
        if (records[i].id == id) {
            // move last record to deleted position
            records[i] = records[record_count - 1];
            record_count--;
            found = 1;
            break;
        }
    }

    if (found)
        sprintf(response, "DELETE OK (id=%d)\n", id);
    else
        sprintf(response, "ERROR: Record with ID %d not found.\n", id);

    pthread_mutex_unlock(&lock);
}

void read_records(char *response) {
    pthread_mutex_lock(&lock);

    if (record_count == 0) {
        sprintf(response, "No records found.\n");
        pthread_mutex_unlock(&lock);
        return;
    }

    strcpy(response, "ID\tNAME\n----------------\n");
    for (int i = 0; i < record_count; i++) {
        char line[100];
        sprintf(line, "%d\t%s\n", records[i].id, records[i].name);
        strcat(response, line);
    }

    pthread_mutex_unlock(&lock);
}

//Client Handler

void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    char buffer[1024], response[1024];
    int bytes_read;

    printf("[+] New client connected (socket %d)\n", client_sock);

    while ((bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        response[0] = '\0';

        printf("Client %d sent: %s", client_sock, buffer);  // <-- ADD THIS LINE

        char command[20], name[50];
        int id;

        if (sscanf(buffer, "INSERT %d %49s", &id, name) == 2)
            insert_record(id, name, response);
        else if (sscanf(buffer, "DELETE %d", &id) == 1)
            delete_record(id, response);
        else if (strncmp(buffer, "READ", 4) == 0)
            read_records(response);
        else if (strncmp(buffer, "EXIT", 4) == 0) {
            sprintf(response, "Goodbye!\n");
            send(client_sock, response, strlen(response), 0);
            printf("Client %d disconnected.\n", client_sock);  // <-- ADD THIS
            close(client_sock);
            pthread_exit(NULL);
        } else
            sprintf(response, "Unknown command.\n");

        send(client_sock, response, strlen(response), 0);
    }

    printf("[-] Client %d connection closed.\n", client_sock);
    close(client_sock);
    pthread_exit(NULL);
}


//Main Server

int main() {
    int server_fd, new_sock;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    pthread_mutex_init(&lock, NULL);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for clients
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d...\n", PORT);

    // Accept loop
    while (1) {
        new_sock = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_sock < 0) {
            perror("accept");
            continue;
        }

        int *pclient = malloc(sizeof(int));
        *pclient = new_sock;

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, pclient);
        pthread_detach(tid);  // auto-clean threads
    }

    pthread_mutex_destroy(&lock);
    close(server_fd);
    return 0;
}

