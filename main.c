#include "shell.h" 
#include <stdio.h>  
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <sys/wait.h> 
#include <linux/limits.h>

/* 
 * Analizon komandën e dhënë nga përdoruesi dhe e ndan atë në argumente.
 * Përdor strtok për të ndarë stringun bazuar në hapësirë, tab dhe karaktere të reja.
 * Ruan argumentet në një varg args dhe përfundon vargun me NULL.
 */
void parse_command(char *input, char **args) {
    char *token = strtok(input, " \n\t");
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \n\t");
    }
    args[i] = NULL;
}

/*
 * Kontrollon nëse një file ekziston dhe nëse kemi të drejtat e duhura për të aksesuar.
 * Kthen -1 nëse file nuk ekziston ose nuk kemi leje, 0 nëse gjithçka është në rregull.
 * Shfaq mesazhe gabimi nëse ka probleme me aksesin.
 */
int check_file_access(const char *path, int mode) {
    if (access(path, F_OK) != 0) {
        fprintf(stderr, "Error: '%s': No such file or directory\n", path);
        return -1;
    }
    if (access(path, mode) != 0) {
        fprintf(stderr, "Error: '%s': Permission denied\n", path);
        return -1;
    }
    return 0;
}

/*
 * Kontrollon për ridrejtime input/output (< dhe >) dhe procese në background (&).
 * Hap filet e specifikuara për ridrejtim dhe ruan file descriptors.
 * Modifikon argumentet për të hequr karakteret e ridrejtimit dhe emrat e fileve.
 */
int check_redirections(char **args, int *input_fd, int *output_fd, int *background) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Error: Missing file for input redirection\n");
                return -1;
            }
            if (check_file_access(args[i + 1], R_OK) != 0) return -1;
            *input_fd = open(args[i + 1], O_RDONLY);
            args[i] = NULL;
            i++;
        } else if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Error: Missing file for output redirection\n");
                return -1;
            }
            *output_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            args[i] = NULL;
            i++;
        } else if (strcmp(args[i], "&") == 0) {
            *background = 1;
            args[i] = NULL;
        }
    }
    return 0;
}

/*
 * Ekzekuton një komandë duke përdorur execvp.
 * Para ekzekutimit, ridrejton input/output nëse është e nevojshme.
 * Mbyll file descriptors që nuk nevojiten më.
 */
void execute_command(char **args, int input_fd, int output_fd) {
    if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    if (output_fd != STDOUT_FILENO) {
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }
    
    execvp(args[0], args);
    fprintf(stderr, "Error: Command '%s' failed\n", args[0]);
    exit(1);
}

/*
 * Ekzekuton dy komanda të lidhura me pipe.
 * Krijon një pipe dhe dy procese fëmijë.
 * Procesi i parë shkruan në pipe, i dyti lexon nga pipe.
 */
void execute_pipe(char **args1, char **args2) {
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) < 0) {
        perror("pipe");
        return;
    }

    pid1 = fork();
    if (pid1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execute_command(args1, STDIN_FILENO, STDOUT_FILENO);
    }

    pid2 = fork();
    if (pid2 == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execute_command(args2, STDIN_FILENO, STDOUT_FILENO);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

/*
 * Funksioni kryesor që implementon shell-in.
 * Lexon komandat nga përdoruesi në një loop të pafund derisa të jepet komanda 'exit'.
 * Suporton komanda të thjeshta, pipe, ridrejtime, dhe procese në background.
 */
int main() {
    char input[MAX_in];
    char *args[MAX_ARGS];
    char *pipe_args[MAX_ARGS];
    char prompt[PATH_MAX + 32];

    printf("\n=== Welcome to Simple Shell ===\n");
    printf("Type 'help' for commands\n\n");

    while (1) {
        get_prompt(prompt, sizeof(prompt));
        printf("%s", prompt);
        fflush(stdout);

        if (!fgets(input, MAX_in, stdin)) break;
        add_to_history(input);

        char *pipe_pos = strchr(input, '|');
        if (pipe_pos != NULL) {
            *pipe_pos = '\0';
            parse_command(input, args);
            parse_command(pipe_pos + 1, pipe_args);
            if (args[0] && pipe_args[0]) {
                execute_pipe(args, pipe_args);
            }
            continue;
        }

        parse_command(input, args);
        if (args[0] == NULL) continue;
        
        if (strcmp(args[0], "exit") == 0) break;

        if (handle_builtin(args)) continue;

        int input_fd = STDIN_FILENO;
        int output_fd = STDOUT_FILENO;
        int background = 0;

        if (check_redirections(args, &input_fd, &output_fd, &background) == -1) {
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            execute_command(args, input_fd, output_fd);
        } else {
            if (!background) {
                waitpid(pid, NULL, 0);
            } else {
                printf("[1] %d\n", pid);
            }
        }
    }

    return 0;
}