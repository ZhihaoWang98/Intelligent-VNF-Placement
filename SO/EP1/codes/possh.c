#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "possh.h"

char *command_buff;

void loop_commands(char *current) {
    char *sub1, *sub2, *sub3, *sub4, *sub5;
    command_buff = readline(current);
    if (strlen(command_buff) > 0) {
        add_history(command_buff);
    }
    sub1 = strtok(command_buff, " ");
    if (sub1 != NULL) {
        if (strcmp(sub1, "/usr/bin/du") == 0) {
            pid_t son;
            son = fork();
            if (son == 0) {
                execl("/usr/bin/du", "/usr/bin/du", "-hs", NULL);
            } else {
                waitpid(-1, NULL, 0);
            }
        } else if (strcmp(sub1, "/usr/bin/traceroute") == 0) {
            pid_t son;
            son = fork();
            if (son == 0) {
                sub2 = strtok(NULL, " ");
                execl("/usr/bin/traceroute", "/usr/bin/traceroute", sub2, NULL);
            } else {
                waitpid(-1, NULL, 0);
            }
        } else if (strcmp(sub1, "./ep1") == 0) {
            pid_t son;
            son = fork();
            if (son == 0) {
                sub2 = strtok(NULL, " ");
                sub3 = strtok(NULL, " ");
                sub4 = strtok(NULL, " ");
                sub5 = strtok(NULL, "");
                if (sub5 != NULL){
                    execl("./ep1", "./ep1", sub2, sub3, sub4, sub5, NULL);
                } else {
                    execl("./ep1", "./ep1", sub2, sub3, sub4, NULL);
                }
            } else {
                waitpid(-1, NULL, 0);
            }
        } else if (strcmp(sub1, "mkdir") == 0) {
            sub2 = strtok(NULL, " ");
            mkdir(sub2, 777);
        } else if (strcmp(sub1, "kill") == 0) {
            sub2 = strtok(NULL, " ");
            sub3 = strtok(NULL, "");
            kill(atoi(sub3), SIGKILL);
        } else if (strcmp(sub1, "ln") == 0) {
            sub2 = strtok(NULL, " ");
            sub3 = strtok(NULL, " ");
            sub4 = strtok(NULL, "");
            link(sub3, sub4);
        } else if (strcmp(sub1, "sair") == 0 || strcmp(sub1, "exit") == 0) {
            exit(EXIT_SUCCESS);
        }
    }
}
