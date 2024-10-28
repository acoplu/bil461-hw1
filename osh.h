#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 64
#define MAX_HISTORY_SIZE 100

// Command history structure
typedef struct Command {
    char command[MAX_INPUT_SIZE];
    struct Command *next;
} Command;

// History functions
void add_to_history(const char *cmd);
char *get_command_by_index(int index);  // Add this function prototype
void free_history();

// Command execution
void execute_command(char *command);
