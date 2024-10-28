#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_COMMAND_LENGTH 1024

void shell_loop() {
    char command[MAX_COMMAND_LENGTH];
    
    while (1) {
        // New command from the user
        printf("osh-> ");
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            perror("Err");
            exit(EXIT_FAILURE);
        }

        // New line
        command[strcspn(command, "\n")] = 0;

        // Exit control
        if (strcmp(command, "exit") == 0) {
            break;
        }
    }
}

int main() {
    // Start the shell loop
    shell_loop();
    return 0;
}
