#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "ep2.h"

pthread_barrier_t barr;
int barr_count;
pthread_mutex_t lock_b;
int last;
pthread_mutex_t lock_l;
int l_run = 0;
pthread_mutex_t lock_r;
int final_pos;
pthread_mutex_t lock_p;

void decrease(int td) {
    for (int i = 0; i < C_NUM; i++) {
        if (CYCL[i].met < CYCL[td].met && CYCL[i].line == CYCL[td].line) {
            CYCL[i].speed = 30;
        }
    }
}

int eliminated(int lap) {
    int pos;
    double aux;
    for (int i = 0; i < C_NUM; i++) {
        if (strcmp(CYCL[i].status[lap], "correndo") == 0) {
            pos = i;
            aux = CYCL[i].time[lap];
            break;
        }
    }
    for (int i = 0; i < C_NUM; i++) {
        if (strcmp(CYCL[i].status[lap], "correndo") == 0 && CYCL[i].time[lap] >= aux) {
            if (CYCL[i].time[lap] > aux) {
                pos = i;
                aux = CYCL[i].time[lap];
            } else {
                int candidate = rand() % 1;
                if (candidate == 1) {
                    pos = i;
                    aux = CYCL[i].time[lap];
                }
            }
        }
    }
    return pos;
}

int elimination(int td, int lap) {
    if (CYCL[td].num == last) {
        if (barr_count != 1) {
            strcpy(CYCL[td].status[lap], "eliminado");
        }
        pthread_mutex_lock(&lock_b);
        barr_count--;
        pthread_barrier_destroy(&barr);
        pthread_barrier_init(&barr, NULL, barr_count);
        pthread_mutex_unlock(&lock_b);
        CYCL[td].f_time = CYCL[td].time[lap];
        CYCL[td].f_pos = final_pos;
        pthread_mutex_lock(&lock_p);
        final_pos--;
        pthread_mutex_unlock(&lock_p);
        show_report(lap - 1);
        show_report(lap);
        pthread_exit(EXIT_SUCCESS);
    }
    return (EXIT_FAILURE);
}

void run(int td, int lap, clock_t t) {
    int count = 0, place = 0;
    strcpy(CYCL[td].status[lap], "correndo");
    if (lap > 2) {
        int speed = rand() % 99;
        if ((lap == (L_SIZE - 2) || lap == (L_SIZE - 1)) && speed < 10 && l_run == 0) {
            pthread_mutex_lock(&lock_r);
            l_run = 1;
            pthread_mutex_unlock(&lock_r);
            CYCL[td].speed = 90;
        } else if (CYCL[td].speed == 30) {
            if (speed < 80) {
                CYCL[td].speed = 60;
            } else {
                CYCL[td].speed = 30;
                decrease(td);
            }
        } else {
            if (speed < 60) {
                CYCL[td].speed = 60;
            } else {
                CYCL[td].speed = 30;
                decrease(td);
            }
        }
    }
    while (count < T_SIZE) {
        if (CYCL[td].speed == 30) {
            usleep(1200);
        } else if (CYCL[td].speed == 60) {
            usleep(600);
        } else {
            usleep(400);
        }
        place = (place + 1) % (T_SIZE + 1);
        if (place == 0) {
            place = 1;
        }
        if (CYCL[td].met != 0) {
            pthread_mutex_lock(&TRACK[CYCL[td].met].lock_t);
            TRACK[CYCL[td].met].line[CYCL[td].line] = -1;
            pthread_mutex_unlock(&TRACK[CYCL[td].met].lock_t);
        }
        CYCL[td].met = place;
        pthread_mutex_lock(&TRACK[place].lock_t);
        TRACK[place].line[TRACK[place].size] = CYCL[td].num;
        CYCL[td].line = TRACK[place].size;
        TRACK[place].size = (TRACK[place].size + 1) % 10;
        pthread_mutex_unlock(&TRACK[place].lock_t);
        count++;
    }
    CYCL[td].time[lap] = ((double) (clock() - t)) / CLOCKS_PER_SEC;
    if ((lap % 6) == 0 && barr_count > 5) {
        int cr = rand() % 99;
        if (cr < 5) {
            strcpy(CYCL[td].status[lap], "quebrado");
            CYCL[td].br = lap;
            fprintf(stderr, "\nCiclista %d quebrou na volta %d.\n", CYCL[td].num, CYCL[td].br);
            CYCL[td].f_pos = 1000000;
            pthread_mutex_lock(&lock_p);
            final_pos--;
            pthread_mutex_unlock(&lock_p);
        }
    }
}

void *thread_f(void *arg) {
    int pos = *(int *) arg, lap = 1;
    clock_t beg = clock();
    while (1) {
        if (lap == 1) {
            run(pos, lap, beg);
            lap++;
            run(pos, lap, beg);
            pthread_mutex_lock(&lock_l);
            last = eliminated(lap);
            pthread_mutex_unlock(&lock_l);
            pthread_barrier_wait(&barr);
            elimination(pos, lap);
            lap++;
        } else {
            if (strcmp(CYCL[pos].status[lap - 1], "quebrado") == 0) {
                pthread_mutex_lock(&lock_b);
                barr_count--;
                pthread_barrier_destroy(&barr);
                pthread_barrier_init(&barr, NULL, barr_count);
                pthread_mutex_unlock(&lock_b);
                pthread_exit(EXIT_SUCCESS);
            } else {
                run(pos, lap, beg);
                lap++;
                run(pos, lap, beg);
                pthread_mutex_lock(&lock_l);
                last = eliminated(lap);
                pthread_mutex_unlock(&lock_l);
                pthread_barrier_wait(&barr);
                elimination(pos, lap);
                lap++;
            }
        }
        usleep(100000);
        if (barr_count < 2) {
            CYCL[pos].f_time = CYCL[pos].time[lap - 1];
            CYCL[pos].f_pos = final_pos;
            pthread_exit(EXIT_SUCCESS);
        }
    }
}

void start_running(void) {
    srand((unsigned) time(NULL));
    int *init = (int *) malloc(C_NUM * sizeof (int));
    for (int i = 0; i < C_NUM; i++) {
        init[i] = i;
    }
    for (int i = 0; i < C_NUM; i++) {
        int pos = rand() % C_NUM;
        int aux = init[i];
        init[i] = init[pos];
        init[pos] = aux;
    }
    fprintf(stderr, "Preparando os ciclistas...\n");
    if (C_NUM < 6) {
        for (int i = 0; i < C_NUM; i++) {
            int k = init[i];
            pthread_create(&CYCL[init[i]].td, NULL, thread_f, &k);
            usleep(100000);
        }
    } else {
        int cont = 0;
        while (cont < C_NUM) {
            for (int j = 0; j < 5; j++) {
                if (cont < C_NUM) {
                    int k = init[cont];
                    pthread_create(&CYCL[init[cont]].td, NULL, thread_f, &k);
                    usleep(100000);
                    cont++;
                }
            }
            usleep(300000);
        }
    }
    fprintf(stderr, "Ciclistas correndo!\n");
    free(init);
}

void start_cycl(void) {
    CYCL = (tCyclist *) malloc(C_NUM * sizeof (tCyclist));
    for (int i = 0; i < C_NUM; i++) {
        CYCL[i].num = i;
        CYCL[i].speed = 30;
        CYCL[i].met = 0;
        CYCL[i].line = -1;
        CYCL[i].time = (double *) malloc(L_SIZE * sizeof (double));
        CYCL[i].status = (char **) malloc(L_SIZE * sizeof (char*));
        for (int j = 1; j < L_SIZE; j++) {
            CYCL[i].status[j] = (char*) malloc(sizeof (char));
            strcpy(CYCL[i].status[j], "********");
        }
    }
}

void start_track(void) {
    TRACK = (tTrack *) malloc((T_SIZE + 1) * sizeof (tTrack));
    for (int i = 0; i < T_SIZE; i++) {
        TRACK[i].size = 0;
        pthread_mutex_init(&TRACK[i].lock_t, NULL);
        for (int j = 0; j < 10; j++) {
            TRACK[i].line[j] = -1;
        }
    }
    barr_count = C_NUM;
    pthread_barrier_init(&barr, NULL, barr_count);
    pthread_mutex_init(&lock_b, NULL);
    pthread_mutex_init(&lock_l, NULL);
    pthread_mutex_init(&lock_r, NULL);
    pthread_mutex_init(&lock_p, NULL);
}

void merge(tCyclist *c, int beg, int mid, int end, int lap) {
    int beg1 = beg, beg2 = mid + 1, beg_aux = 0, size = end - beg + 1;
    tCyclist *vet_aux = (tCyclist*) malloc(size * sizeof (tCyclist));
    if (lap > 0) {
        for (int i = 0; i < size; i++) {
            vet_aux[i].time = (double *) malloc(L_SIZE * sizeof (double));
            vet_aux[i].status = (char **) malloc(L_SIZE * sizeof (char*));
            for (int j = 1; j < L_SIZE; j++) {
                vet_aux[i].status[j] = (char*) malloc(sizeof (char));
            }
        }
        while (beg1 <= mid && beg2 <= end) {
            if (c[beg1].time[lap] < c[beg2].time[lap]) {
                vet_aux[beg_aux].num = c[beg1].num;
                vet_aux[beg_aux].time[lap] = c[beg1].time[lap];
                strcpy(vet_aux[beg_aux].status[lap], c[beg1].status[lap]);
                beg1++;
            } else {
                vet_aux[beg_aux].num = c[beg2].num;
                vet_aux[beg_aux].time[lap] = c[beg2].time[lap];
                strcpy(vet_aux[beg_aux].status[lap], c[beg2].status[lap]);
                beg2++;
            }
            beg_aux++;
        }
        while (beg1 <= mid) {
            vet_aux[beg_aux].num = c[beg1].num;
            vet_aux[beg_aux].time[lap] = c[beg1].time[lap];
            strcpy(vet_aux[beg_aux].status[lap], c[beg1].status[lap]);
            beg_aux++;
            beg1++;
        }
        while (beg2 <= end) {
            vet_aux[beg_aux].num = c[beg2].num;
            vet_aux[beg_aux].time[lap] = c[beg2].time[lap];
            strcpy(vet_aux[beg_aux].status[lap], c[beg2].status[lap]);
            beg_aux++;
            beg2++;
        }
        for (beg_aux = beg; beg_aux <= end; beg_aux++) {
            c[beg_aux].num = vet_aux[beg_aux - beg].num;
            c[beg_aux].time[lap] = vet_aux[beg_aux - beg].time[lap];
            strcpy(c[beg_aux].status[lap], vet_aux[beg_aux - beg].status[lap]);
        }
        for (int i = 0; i < size; i++) {
            for (int j = 1; j < L_SIZE; j++) {
                free(vet_aux[i].status[j]);
            }
            free(vet_aux[i].status);
            free(vet_aux[i].time);
        }
    } else {
        while (beg1 <= mid && beg2 <= end) {
            if (c[beg1].f_pos < c[beg2].f_pos) {
                vet_aux[beg_aux].num = c[beg1].num;
                vet_aux[beg_aux].f_pos = c[beg1].f_pos;
                vet_aux[beg_aux].f_time = c[beg1].f_time;
                beg1++;
            } else {
                vet_aux[beg_aux].num = c[beg2].num;
                vet_aux[beg_aux].f_pos = c[beg2].f_pos;
                vet_aux[beg_aux].f_time = c[beg2].f_time;
                beg2++;
            }
            beg_aux++;
        }
        while (beg1 <= mid) {
            vet_aux[beg_aux].num = c[beg1].num;
            vet_aux[beg_aux].f_pos = c[beg1].f_pos;
            vet_aux[beg_aux].f_time = c[beg1].f_time;
            beg_aux++;
            beg1++;
        }
        while (beg2 <= end) {
            vet_aux[beg_aux].num = c[beg2].num;
            vet_aux[beg_aux].f_pos = c[beg2].f_pos;
            vet_aux[beg_aux].f_time = c[beg2].f_time;
            beg_aux++;
            beg2++;
        }
        for (beg_aux = beg; beg_aux <= end; beg_aux++) {
            c[beg_aux].num = vet_aux[beg_aux - beg].num;
            c[beg_aux].f_pos = vet_aux[beg_aux - beg].f_pos;
            c[beg_aux].f_time = vet_aux[beg_aux - beg].f_time;
        }
    }
    free(vet_aux);
}

void merge_sort(tCyclist *c, int beg, int end, int lap) {
    if (beg < end) {
        int mid = (end + beg) / 2;
        merge_sort(c, beg, mid, lap);
        merge_sort(c, mid + 1, end, lap);
        merge(c, beg, mid, end, lap);
    }
}

void show_report(int lap) {
    if (lap > 0 && lap < L_SIZE - 2) {
        int pos = 1;
        tCyclist *cycl_aux = (tCyclist *) malloc(C_NUM * sizeof (tCyclist));
        for (int i = 0; i < C_NUM; i++) {
            cycl_aux[i].num = CYCL[i].num;
            cycl_aux[i].time = (double *) malloc(L_SIZE * sizeof (double));
            cycl_aux[i].time[lap] = CYCL[i].time[lap];
            cycl_aux[i].status = (char **) malloc(L_SIZE * sizeof (char*));
            for (int j = 1; j < L_SIZE; j++) {
                cycl_aux[i].status[j] = (char*) malloc(sizeof (char));
            }
            strcpy(cycl_aux[i].status[lap], CYCL[i].status[lap]);
        }
        merge_sort(cycl_aux, 0, C_NUM - 1, lap);
        fprintf(stderr, "\n--------------- Volta de número %d ---------------\n", lap);
        fprintf(stderr, "Posição -- Ciclista --- Tempo ------ Estado\n");
        for (int i = 0; i < C_NUM; i++) {
            if (strcmp(cycl_aux[i].status[lap], "********") != 0) {
                fprintf(stderr, "%d --------- %d -------- %.4f ---- %s\n", pos++, cycl_aux[i].num, cycl_aux[i].time[lap], cycl_aux[i].status[lap]);
            }
        }
        for (int i = 0; i < C_NUM; i++) {
            if (strcmp(cycl_aux[i].status[lap], "********") == 0) {
                fprintf(stderr, "%d --------- %d -------- %.4f ---- %s\n", pos++, cycl_aux[i].num, cycl_aux[i].time[lap], cycl_aux[i].status[lap]);
            }
        }
        for (int i = 0; i < C_NUM; i++) {
            for (int j = 1; j < L_SIZE; j++) {
                free(cycl_aux[i].status[j]);
            }
            free(cycl_aux[i].status);
            free(cycl_aux[i].time);
        }
        free(cycl_aux);
    } else {
        merge_sort(CYCL, 0, C_NUM - 1, lap);
        fprintf(stderr, "\n---------- Classificação Final ----------\n");
        fprintf(stderr, "Posição -- Ciclista --- Tempo final\n");
        for (int i = 0; i < C_NUM; i++) {
            if (CYCL[i].f_pos == 1000000) {
                fprintf(stderr, "* --------- %d -------- quebrou na volta %d\n", CYCL[i].num, CYCL[CYCL[i].num].br);
            } else {
                fprintf(stderr, "%d --------- %d ---------- %.4f\n", i + 1, CYCL[i].num, CYCL[i].f_time);
            }
        }
        usleep(100000);
    }
}

void finish_running(void) {
    for (int i = 0; i < C_NUM; i++) {
        for (int j = 1; j < L_SIZE; j++) {
            free(CYCL[i].status[j]);
        }
        free(CYCL[i].status);
        free(CYCL[i].time);
    }
    for (int i = 0; i < T_SIZE; i++) {
        pthread_mutex_destroy(&TRACK[i].lock_t);
    }
    pthread_barrier_destroy(&barr);
    pthread_mutex_destroy(&lock_b);
    pthread_mutex_destroy(&lock_l);
    pthread_mutex_destroy(&lock_r);
    pthread_mutex_destroy(&lock_p);
    free(CYCL);
    free(TRACK);
}

int competition(int d, int n, int deb) {
    T_SIZE = d;
    C_NUM = n;
    L_SIZE = (2 * C_NUM) + 1;
    final_pos = C_NUM;
    if (deb == 1) {
        D = 1;
    } else {
        D = 0;
    }
    start_track();
    start_cycl();
    start_running();
    if (D == 1) {
        while (final_pos > 1) {
            if (final_pos > 2) {
                usleep(600);
            } else {
                usleep(400);
            }
            for (int i = 0; i < T_SIZE; i++) {
                for (int j = 0; j < 10; j++) {
                    if (TRACK[i].line[j] != -1) {
                        fprintf(stderr, "Metro %d ciclista %d na linha %d.\n", i + 1, TRACK[i].line[j], j);
                    }
                }
            }
        }
    }
    for (int i = 0; i < C_NUM; i++) {
        pthread_join(CYCL[i].td, NULL);
    }
    show_report(0);
    finish_running();
    return (EXIT_SUCCESS);
}