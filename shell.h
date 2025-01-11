#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>      
#include <stdlib.h>    
#include <string.h>   
#include <unistd.h>     
#include <sys/wait.h>  
#include <fcntl.h>      
#include <errno.h>      
#include <limits.h> 


#define MAX_ARGS 64
#define HISTORY_SIZE 15
#define MAX_in 1000

void get_prompt(char *prompt, size_t size);
void shell_help(void);
void shell_cd(char **args);
void shell_pwd(void);
void shell_echo(char **args);
void shell_history(void);
void shell_clear(void);
void add_to_history(const char *command);
int handle_builtin(char **args);

extern char history[HISTORY_SIZE][MAX_in];
extern int history_count;

#endif 
