#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ep3.h"

int main(int argc, char** argv) {
    if (argv[1] == NULL) {
        char current[] = "[ep3]: ";
        while (1) {
            loop_commands(current);
        }
    } else {
        if (argc == 3) {
            test_mode(atoi(argv[1]), atoi(argv[2]));
        } else {
            fprintf(stderr, "Erro! Parâmetros inválidos para a execução do modo de teste.\n");
        }
    }
    return (EXIT_SUCCESS);
}