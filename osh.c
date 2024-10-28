#include "osh.h"

Command *history_head = NULL;
int history_count = 0;

// Function to add command to history
void add_to_history(const char *cmd) {
    Command *new_command = malloc(sizeof(Command));
    strncpy(new_command->command, cmd, MAX_INPUT_SIZE);

    new_command->next = history_head;
    history_head = new_command;
    history_count = history_count + 1;
}

// Retrieves the index-th last command, 0 is the most recent
char *get_command_by_index(int index) {
    Command *current = history_head;
    for (int i = 0; i < index; i++) {
        if (current->next == NULL) {
            return NULL;
        }
        current = current->next;
    }

    // In last iteration current->next can be NULL
    if(current == NULL) return NULL;
    else return current->command;
}

// Function to free command history
void free_history() {
    Command *temp;
    while (history_head != NULL) {
        temp = history_head;
        history_head = history_head->next;
        free(temp);
    }
}

// Function to execute commands using UNIX Shell
void execute_command(char *command) {
    char *args[MAX_ARG_SIZE];
    char *token;
    int arg_count = 0;
    int out_redirect = 0;
    int in_redirect = 0;
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

        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        wait(NULL);  // Parent process waits for child to complete
    }
}

// main loop
int main() {
    char command[MAX_INPUT_SIZE];
    
    while (1) {
        printf("osh-> ");

        if (!fgets(command, MAX_INPUT_SIZE, stdin)) break;
        command[strlen(command) - 1] = '\0';  // Remove newline character

        if (strcmp(command, "exit") == 0) break;

        // Check if command starts with "!!"
        char temp[3];
        temp[0] = command[0];
        temp[1] = command[1];
        temp[2] = '\0';
        if (strcmp(temp, "!!") == 0) {
            int index = atoi(command + 3);  // Extract the index after "!! "
            if (index < 0 || index >= history_count) {
                printf("Error: No command at index %d in history.\n", index);
                continue;
            }

            // Get the index-th command
            char *history_command = get_command_by_index(index);
            if (history_command) {
                strcpy(command, history_command);
            } else {
                printf("Error: No such command in history.\n");
                continue;
            }
        }

        // Add current command to history
        add_to_history(command);
        // Execute the current command
        execute_command(command);
    }

    free_history();
    return 0;
}
