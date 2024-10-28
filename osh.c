#include "osh.h"

Command *history_head = NULL;
int history_count = 0;

// Function to add command to history
void add_to_history(const char *cmd) {
    Command *new_command = (Command *)malloc(sizeof(Command));
    strncpy(new_command->command, cmd, MAX_INPUT_SIZE);
    new_command->next = history_head;
    history_head = new_command;
    if (++history_count > MAX_HISTORY_SIZE) { // remove oldest if exceeds MAX_HISTORY_SIZE
        Command *current = history_head;
        for (int i = 1; i < MAX_HISTORY_SIZE - 1; i++) {
            current = current->next;
        }
        free(current->next);
        current->next = NULL;
        history_count--;
    }
}

// Retrieves the `index`-th last command, with 0 being the most recent
char *get_command_by_index(int index) {
    Command *current = history_head;
    for (int i = 0; i < index; i++) {
        if (!current->next) {
            return NULL;  // Out of bounds
        }
        current = current->next;
    }
    return current ? current->command : NULL;
}

// Function to free command history
void free_history() {
    Command *current = history_head;
    while (current) {
        Command *temp = current;
        current = current->next;
        free(temp);
    }
}

void execute_command(char *command) {
    char *args[MAX_ARG_SIZE];
    char *token;
    int arg_count = 0;
    int out_redirect = 0, in_redirect = 0;
    char *output_file = NULL;
    char *input_file = NULL;

    // Parse command and detect redirection symbols
    token = strtok(command, " ");
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            out_redirect = 1;
            token = strtok(NULL, " ");
            if (token) output_file = token;
        } else if (strcmp(token, "<") == 0) {
            in_redirect = 1;
            token = strtok(NULL, " ");
            if (token) input_file = token;
        } else {
            args[arg_count++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    pid_t pid = fork();
    if (pid == 0) {  // Child process
        // Handle output redirection
        if (out_redirect && output_file) {
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        // Handle input redirection
        if (in_redirect && input_file) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        execvp(args[0], args);  // Execute the command
        perror("execvp");  // Only reached if exec fails
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        wait(NULL);  // Parent process waits for child to complete
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

int main() {
    char command[MAX_INPUT_SIZE];
    
    while (1) {
        printf("osh-> ");
        fflush(stdout);

        if (!fgets(command, MAX_INPUT_SIZE, stdin)) break;
        command[strlen(command) - 1] = '\0';  // Remove newline character

        if (strcmp(command, "exit") == 0) break;

        // Check if command starts with "!!"
        if (strncmp(command, "!!", 2) == 0) {
            int index = atoi(command + 3);  // Extract the index after "!! "
            if (index < 0 || index >= history_count) {
                printf("Error: No command at index %d in history.\n", index);
                continue;
            }

            // Get the `index`-th last command
            char *history_command = get_command_by_index(index);
            if (history_command) {
                printf("%s\n", history_command);  // Echo the retrieved command
                strcpy(command, history_command);  // Set it as the current command
            } else {
                printf("No such command in history.\n");
                continue;
            }
        }

        add_to_history(command);    // Add current command to history
        execute_command(command);   // Execute the current command
    }

    free_history();
    return 0;
}
