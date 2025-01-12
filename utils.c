#include "shell.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

/* Vargu për të ruajtur historinë e komandave dhe numëratori i komandave */
char history[HISTORY_SIZE][MAX_in];
int history_count = 0;

/*
 * Gjeneron promptin e shell-it me ngjyra.
 * Zëvendëson path-in e home me '~' nëse jemi në direktorinë home.
 * Përdor kode ANSI për ngjyrat: blu për path, jeshile për prompt.
 */
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

/*
 * Shfaq listën e komandave të brendshme dhe operatorëve specialë.
 * Përfshin shpjegime të shkurtra për çdo komandë dhe operator.
 */
void shell_help() {
    printf("\nAvailable built-in commands:\n");
    printf(" help - Show this help message\n");
    printf(" cd - Change directory\n");
    printf(" pwd - Print working directory\n");
    printf(" echo - Print text to screen\n");
    printf(" history - Show command history\n");
    printf(" clear - Clear the screen\n");
    printf(" exit - Exit the shell\n");
    printf("\nSpecial operators:\n");
    printf(" > - Redirect output to file\n");
    printf(" < - Redirect input from file\n");
    printf(" | - Pipe output to another command\n");
    printf(" & - Run command in background\n\n");
}

/*
 * Ndryshon direktorinë aktuale.
 * Nëse nuk jepet argument, kalon në direktorinë home.
 * Përdor getenv për të marrë path-in e home dhe chdir për të ndryshuar direktorinë.
 */
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

/*
 * Shfaq direktorinë aktuale të punës.
 * Përdor getcwd për të marrë path-in aktual.
 */
void shell_pwd() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
}

/*
 * Shfaq tekst në ekran.
 * Printon të gjitha argumentet e dhëna me hapësirë midis tyre.
 * Nëse nuk ka argumente, printon vetëm një rresht të ri.
 */
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

/*
 * Shfaq historinë e komandave të ekzekutuara.
 * Printon çdo komandë me numrin e saj përkatës.
 */
void shell_history() {
    if (history_count == 0) {
        printf("No commands in history\n");
        return;
    }
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s", i + 1, history[i]);
    }
}

/*
 * Pastron ekranin duke përdorur kode ANSI.
 * \033[H zhvendos kursorin në fillim.
 * \033[J pastron ekranin nga pozicioni i kursorit.
 */
void shell_clear() {
    printf("\033[H\033[J");
}

/*
 * Shton një komandë në historinë e shell-it.
 * Nëse historia është plot, fshin komandën më të vjetër.
 * Injoron komandat boshe ose shumë të shkurtra.
 */
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

/*
 * Kontrollon dhe ekzekuton komandat e brendshme të shell-it.
 * Kthen 1 nëse komanda ishte e brendshme dhe u ekzekutua.
 * Kthen 0 nëse komanda nuk është e brendshme.
 */
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