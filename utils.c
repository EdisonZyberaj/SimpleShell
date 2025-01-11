#include "shell.h"       
#include <stddef.h>      
#include <stdio.h>       
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>      
#include <linux/limits.h> 

char history[HISTORY_SIZE][MAX_in];
int history_count = 0;

void get_prompt(char *prompt, size_t size) {
    char cwd[PATH_MAX];
    char *home = getenv("HOME");

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (home && strncmp(cwd, home, strlen(home)) == 0) {
            snprintf(prompt, size, "\033[1;34m~%s\033[1;32m$>\033[0m ", cwd + strlen(home));
        } else {
            snprintf(prompt, size, "\033[1;34m%s\033[1;32m$>\033[0m ", cwd);
        }
    } else {
        snprintf(prompt, size, "\033[1;31m???\033[1;32m$>\033[0m ");
    }
}

void shell_help() {
    printf("\nAvailable built-in commands:\n");
    printf("  help     - Show this help message\n");
    printf("  cd       - Change directory\n");
    printf("  pwd      - Print working directory\n");
    printf("  echo     - Print text to screen\n");
    printf("  history  - Show command history\n");
    printf("  clear    - Clear the screen\n");
    printf("  exit     - Exit the shell\n");
    printf("\nSpecial operators:\n");
    printf("  >        - Redirect output to file\n");
    printf("  <        - Redirect input from file\n");
    printf("  |        - Pipe output to another command\n");
    printf("  &        - Run command in background\n\n");
}

void shell_cd(char **args) {
    if (args[1] == NULL) {
        char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "Error: HOME environment variable not set\n");
            return;
        }
        if (chdir(home) != 0) {
            perror("cd");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
}

void shell_pwd() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
}

void shell_echo(char **args) {
    if (args[1] == NULL) {
        printf("\n");
        return;
    }

    int i = 1;
    while (args[i] != NULL) {
        printf("%s", args[i]);
        if (args[i + 1] != NULL) printf(" ");
        i++;
    }
    printf("\n");
}

void shell_history() {
    if (history_count == 0) {
        printf("No commands in history\n");
        return;
    }

    for (int i = 0; i < history_count; i++) {
        printf("%d: %s", i + 1, history[i]);
    }
}

void shell_clear() {
    printf("\033[H\033[J");
}

void add_to_history(const char *command) {
    if (strlen(command) <= 1) return;

    if (history_count < HISTORY_SIZE) {
        strncpy(history[history_count++], command, MAX_in - 1);
        history[history_count - 1][MAX_in - 1] = '\0'; 
    } else {
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            strncpy(history[i], history[i + 1], MAX_in);
        }
        strncpy(history[HISTORY_SIZE - 1], command, MAX_in - 1);
        history[HISTORY_SIZE - 1][MAX_in - 1] = '\0'; 
    }
}

int handle_builtin(char **args) {
    if (args[0] == NULL) return 0;

    if (strcmp(args[0], "help") == 0) { shell_help(); return 1; }
    if (strcmp(args[0], "cd") == 0) { shell_cd(args); return 1; }
    if (strcmp(args[0], "pwd") == 0) { shell_pwd(); return 1; }
    if (strcmp(args[0], "echo") == 0) { shell_echo(args); return 1; }
    if (strcmp(args[0], "history") == 0) { shell_history(); return 1; }
    if (strcmp(args[0], "clear") == 0) { shell_clear(); return 1; }

    return 0;
}
