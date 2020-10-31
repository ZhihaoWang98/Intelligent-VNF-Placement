#include <stdlib.h>
#include <string.h>
#include "ep2.h"

int main(int argc, char** argv) {
    int d = atoi(argv[1]);
    int n = atoi(argv[2]);
    int deb = 0;
    if (argv[3] != NULL && strcmp(argv[3], "d") == 0) {
        deb = 1;
    }
    competition(d, n, deb);
    return (EXIT_SUCCESS);
}

