#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include "ep1.h"

int main(int argc, char** argv) {
    T_SIZE = 0, D = 0;
    if (argv[4] != NULL && strcmp(argv[4], "d") == 0) {
        D = 1;
    }
    if (!open_file(argv[2])) {
        switch (atoi(argv[1])) {
            case 1:
                first_come_first_served(argv[3]);
                break;
            case 2:
                shortest_remaining_time_next(argv[3]);
                break;
            case 3:
                round_robin(argv[3], 7);
                break;
            default:
                printf("Escalonador não selecionado!\n");
                return (EXIT_FAILURE);
        }
    }
    return (EXIT_SUCCESS);
}
