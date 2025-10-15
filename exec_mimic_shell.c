#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    pid_t pid;
    int status;

    while (1) {
        // 1. Display prompt
        printf("mini-shell> ");
        fflush(stdout);

        // 2. Read input line
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;  // Exit on Ctrl+D
        }

        // Remove trailing newline
        input[strcspn(input, "\n")] = '\0';

        // Exit command
        if (strcmp(input, "exit") == 0)
            break;

        // 3. Tokenize input into args array
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;  // Null-terminate argument list

        if (args[0] == NULL)
            continue; // Empty command

        // 4. Create a child process
        pid = fork();

        if (pid < 0) {
            perror("fork failed");
            continue;
        }

        if (pid == 0) {
            // 5. Child process executes the command
            execvp(args[0], args);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else {
            // 6. Parent waits for child to finish
            waitpid(pid, &status, 0);
        }
    }

    printf("Exiting mini-shell.\n");
    return 0;
}

