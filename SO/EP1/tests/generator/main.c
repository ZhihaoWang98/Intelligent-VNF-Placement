#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char** argv) {
    FILE *output_file, *input, *output;
    int size, cont = 0, g = 1;

    
    //size = 20;
    //size = 50;
    size = 100;
    


    //numero = x + rand() % (y-x+1)
    //Gera um número aleatório entre x e y (com ambos x e y inclusos no intervalo). 
    if (g == 0) {
        srand(time(NULL));
        for (int j = 0; j < 30; j++) {
            char out[50], str[3];
            strcpy(out, "input");

            strcat(out, "L");

            sprintf(str, "%d", j);
            strcat(out, str);
            strcat(out, ".txt");
            output_file = fopen(out, "w");
            if (output_file == NULL) {
                printf("ERRO! Arquivo de saída não gerado!\n");
                return (EXIT_FAILURE);
            }
            int med_t[size], med_a[size], med_d[size], medt = 0, meda = 0, medd = 0;
            for (int i = 0; i < size; i++) {
                int t0;
                if (i == 0) {
                    t0 = 0;
                } else {
                    t0 = i + (1 + rand() % 2);
                }
                int p = (1 + rand() % 10);
                med_a[i] = p;
                int d = (p + t0) * (3 + rand() % 3);


                med_d[i] = d;
                fprintf(output_file, "p%d %d %d %d\n", i, t0, p, d);
            }
            fclose(output_file);
            for (int i = 0; i < size; i++) {
                meda += med_a[i];
                medd += med_d[i];
            }
            //printf("Arquivo %s:\n", out);
            printf("%d ", meda / size);
            printf("%d\n", medd / size);
        }
    } else {
        char process_i[200], process_o[200], *sub1, *sub2, *sub3, *sub4, *sub5, *sub6;
        int deadline, tf;
        input = fopen("/home/guilherme/Documentos/Projetos C e CPP/SO/EP1/codes/inputL29.txt", "r");
        if (input == NULL) {
            printf("ERRO! Arquivo não encontrado!\n");
            return (EXIT_FAILURE);
        }

        while (fgets(process_i, 200, input) != NULL) {
            sub1 = strtok(process_i, " ");
            if (strcmp(sub1, "\n") != 0) {
                sub2 = strtok(NULL, " ");
                sub3 = strtok(NULL, " ");
                sub4 = strtok(NULL, "");
                deadline = atoi(sub4);

                output = fopen("/home/guilherme/Documentos/Projetos C e CPP/SO/EP1/codes/outL29.txt", "r");
                if (output == NULL) {
                    printf("ERRO! Arquivo não encontrado!\n");
                    return (EXIT_FAILURE);
                }
                while (fgets(process_o, 200, output) != NULL) {
                    sub5 = strtok(process_o, " ");
                    if (strcmp(sub5, "\n") != 0) {
                        if (strcmp(sub5, sub1) == 0) {
                            sub6 = strtok(NULL, " ");
                            tf = atoi(sub6);
                            if (tf <= deadline) {
                                printf("%s finalizado dentro do deadline.\n\n", sub5);
                                cont++;
                            }
                        }
                    }
                }
                fclose(output);
            }
        }
        fclose(input);
        printf("%d processos finalizados dentro do deadline!\n", cont);
    }
    sleep(1);
    return (EXIT_SUCCESS);
}

