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
    char *cmd_segments[MAX_ARG_SIZE];
    int num_segments = 0;
    char *segment = strtok(command, "|");

    // Split the command by "|"
    while (segment != NULL) {
        cmd_segments[num_segments++] = segment;
        segment = strtok(NULL, "|");
    }

    int pipes[MAX_ARG_SIZE - 1][2];  // Pipes between each command segment

    for (int i = 0; i < num_segments; i++) {
        if (i < num_segments - 1) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {  // Child process
            // Check for input redirection
            char *input_file = NULL;
            char *input_redirect = strchr(cmd_segments[i], '<');
            if (input_redirect) {
                *input_redirect = '\0';  // Split the command at '<'
                input_file = strtok(input_redirect + 1, " ");  // Get the filename
                // Open the file for reading
                int fd = open(input_file, O_RDONLY);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);  // Redirect stdin to the file
                close(fd);  // Close the file descriptor
            }

            // Check for output redirection
            char *output_file = NULL;
            char *output_redirect = strchr(cmd_segments[i], '>');
            if (output_redirect) {
                *output_redirect = '\0';  // Split the command at '>'
                output_file = strtok(output_redirect + 1, " ");  // Get the filename
                // Open the file for writing
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);  // Redirect stdout to the file
                close(fd);  // Close the file descriptor
            }

            // Redirect stdin for commands after the first segment
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
            }

            // Redirect stdout for commands before the last segment
            if (i < num_segments - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            // Parse arguments for the command segment
            char *args[MAX_ARG_SIZE];
            char *arg = strtok(cmd_segments[i], " ");
            int arg_count = 0;
            while (arg != NULL) {
                args[arg_count++] = arg;
                arg = strtok(NULL, " ");
            }
            args[arg_count] = NULL;

            execvp(args[0], args);  // Execute command
            perror("execvp");  // If execvp fails
            exit(EXIT_FAILURE);
        } else {  // Parent process
            // Close used pipe ends
            if (i > 0) {
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
            }
        }
    }

    // Wait for all child processes to complete
    for (int i = 0; i < num_segments; i++) {
        wait(NULL);
    }
}

// main loop
int main() {
    char command[MAX_INPUT_SIZE];
    
    while (1) {
        printf("osh-> ");
        fflush(stdout);

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
