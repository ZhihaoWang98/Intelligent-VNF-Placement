#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "possh.h"

int main(int argc, char** argv) {
    char current[1000] = "{", *user, dir[1000];
    user = getlogin();
    strcat(user, "@");
    strcat(current, user);
    getcwd(dir, sizeof (dir));
    strcat(dir, "/");
    strcat(dir, "} ");
    strcat(current, dir);
    while (1) {
        loop_commands(current);
    }
    return (EXIT_SUCCESS);
}

