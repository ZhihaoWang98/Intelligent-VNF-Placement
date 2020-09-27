#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include "ep1.h"

int p_list_size;
int ctx_ch;
int r_size = 0;

int get_cpu(int tid) {
    FILE *proc;
    char *cpu_id, proc_path[] = "/proc/", tid_c[50], line[500];
    sprintf(tid_c, "%d", tid);
    strcat(proc_path, tid_c);
    strcat(proc_path, "/stat");
    proc = fopen(proc_path, "r");
    if (proc == NULL) {
        printf("ERRO! Arquivo PROC não encontrado!\n");
        return (EXIT_FAILURE);
    } else {
        fgets(line, 1000, proc);
        cpu_id = strtok(line, " ");
        for (int i = 1; i < 38; i++) {
            cpu_id = strtok(NULL, " ");
        }
        cpu_id = strtok(NULL, " ");
        fclose(proc);
    }
    return atoi(cpu_id);
}

int get_ctx_ch(int pid) {
    FILE *proc;
    char proc_path[100], line[500], pid_s[500], *sub1, *sub2;
    int ctx_ch = 0;
    strcpy(proc_path, "");
    strcat(proc_path, "/proc/");
    sprintf(pid_s, "%d", pid);
    strcat(proc_path, pid_s);
    strcat(proc_path, "/status");
    proc = fopen(proc_path, "r");
    if (proc == NULL) {
        printf("ERRO! Arquivo PROC não encontrado!\n");
        return (EXIT_FAILURE);
    } else {
        while ((fgets(line, 1000, proc)) != NULL) {
            sub1 = strtok(line, ":");
            if (strcmp(sub1, "voluntary_ctxt_switches") == 0) {
                sub2 = strtok(NULL, "\n");
                int aux = sub2[1] - '0';
                ctx_ch += aux;
            } else if (strcmp(sub1, "nonvoluntary_ctxt_switches") == 0) {
                sub2 = strtok(NULL, "\n");
                int aux = sub2[1] - '0';
                ctx_ch += aux;
            }
        }
        fclose(proc);
    }
    return ctx_ch;
}

void t_process_show(int pos) {
    printf("T_SIZE: %d\n", T_SIZE);
    printf("Process name: %s\n", process_list[pos].name);
    printf("Process process_id: %d\n", process_list[pos].id_proc);
    printf("Status: %s\n", process_list[pos].status);
    printf("CPU: %i\n", process_list[pos].cpu);
    printf("t0: %d\n", process_list[pos].t0);
    printf("dt: %d\n", process_list[pos].dt);
    printf("dt_aux: %d\n", process_list[pos].dt_aux);
    printf("deadline: %d\n\n", process_list[pos].deadline);
}

void *thread_f(void *arg) {
    int pos = *(int *) arg;
    process_list[pos].id_proc = getpid();
    process_list[pos].cpu = get_cpu(process_list[pos].id_proc);
    strcpy(process_list[pos].status, "executando");
    sem_init(&process_list[pos].t_sem, 0, 1);
    sem_wait(&process_list[pos].t_sem);
    while (1) {
        if (process_list[pos].dt_aux == 0) {
            break;
        } else {
            int sem_status;
            do {//aguardando
                sem_getvalue(&process_list[pos].t_sem, &sem_status);
            } while (sem_status != 0);
        }
    }
    strcpy(process_list[pos].status, "finalizado");
    if (D == 1) {
        printf("Thread %d finalizada.\n", pos);
    }
    sem_post(&process_list[pos].t_sem);
    sem_destroy(&process_list[pos].t_sem);
    pthread_exit(EXIT_SUCCESS);
}

void start_thread(int pos) {
    int k = pos;
    if (!pthread_create(&process_list[pos].td, NULL, thread_f, &k)) {
        sleep(1);
        if (D == 1) {
            fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[pos].cpu, process_list[pos].name);
        }
    }
}

void change_ctx(int p_candidate, int l_in_exec) {
    sem_post(&process_list[l_in_exec].t_sem);
    if (D == 1) {
        fprintf(stderr, "%s aguardando.\n\n", process_list[l_in_exec].name);
    }
    if (strcmp(process_list[p_candidate].status, "pronto") == 0) {
        start_thread(p_candidate);
    } else {
        sem_wait(&process_list[p_candidate].t_sem);
    }
}

void finish_process(int pos, int time, FILE *output_file) {
    sleep(1);
    p_list_size--;
    int tf = time + 1;
    int tr = tf - process_list[pos].t0;
    if (D == 1) {
        fprintf(stderr, "CPU %d liberada pelo %s.\n", process_list[pos].cpu, process_list[pos].name);
        fprintf(stderr, "Saída no arquivo: %s %d %d\n", process_list[pos].name, tf, tr);
        fprintf(stderr, "%d troca(s) de contexto.\n\n", ctx_ch);
    }
    fprintf(output_file, "%s %d %d\n", process_list[pos].name, tf, tr);
}

int first_in_time(void) {
    int aux, pos = -1;
    for (int i = 0; i < T_SIZE; i++) {
        if (strcmp(process_list[i].status, "finalizado") != 0) {
            pos = i;
            aux = process_list[i].t0;
            break;
        }
    }
    for (int i = 0; i < T_SIZE; i++) {
        if (strcmp(process_list[i].status, "finalizado") != 0 && process_list[i].t0 < aux) {
            pos = i;
            aux = process_list[i].t0;
        }
    }
    return pos;
}

int get_in_time(int time) {
    for (int i = 0; i < T_SIZE; i++) {
        if (process_list[i].t0 == time) {
            return i;
        }
    }
    return -1;
}

int first_come_first_served(char *path_output) {
    FILE *output_file;
    int i = 0;
    p_list_size = T_SIZE;
    ctx_ch = 0;
    output_file = fopen(path_output, "w");
    if (output_file == NULL) {
        printf("ERRO! Arquivo de saída não gerado!\n");
        return (EXIT_FAILURE);
    }
    while (p_list_size > 0) {
        int process = first_in_time(), aux = 0;
        if (process != -1) {
            if (process_list[process].t0 > i) {
                i = process_list[process].t0;
            }
            start_thread(process);
            ctx_ch++;
            while (process_list[process].dt_aux > 0) {
                if (D == 1) {
                    if (aux > 0) {
                        fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[process].cpu, process_list[process].name);
                    }
                    aux++;
                }
                sleep(1);
                process_list[process].dt_aux--;
                i++;
            }
            if (process_list[process].dt_aux == 0) {
                finish_process(process, i - 1, output_file);
            }
        }
    }
    for (int i = 0; i < T_SIZE; i++) {
        pthread_join(process_list[i].td, NULL);
    }
    fprintf(output_file, "%d\n", ctx_ch);
    fclose(output_file);
    return (EXIT_SUCCESS);
}

int shortest_remaining(void) {
    int aux, pos;
    for (int i = 0; i < T_SIZE; i++) {
        if (strcmp(process_list[i].status, "finalizado") != 0) {
            pos = i;
            aux = process_list[i].dt_aux;
            break;
        }
    }
    for (int i = 0; i < T_SIZE; i++) {
        if (strcmp(process_list[i].status, "finalizado") != 0 && process_list[i].dt_aux <= aux && i != pos) {
            if (process_list[i].dt_aux == aux) {
                if (process_list[i].t0 < process_list[pos].t0) {
                    pos = i;
                    aux = process_list[i].dt_aux;
                }
            } else {
                pos = i;
                aux = process_list[i].dt_aux;
            }
        }
    }
    return pos;
}

int shortest_remaining_in_time(int time) {
    int aux, pos = -1;
    for (int i = 0; i < T_SIZE; i++) {
        if (strcmp(process_list[i].status, "finalizado") != 0 && process_list[i].t0 <= time) {
            pos = i;
            aux = process_list[i].dt_aux;
            if (strcmp(process_list[i].status, "pronto") == 0) {
                process_list[i].pos_process = 1;
            }
            break;
        }
    }
    for (int i = 0; i < T_SIZE; i++) {
        if (strcmp(process_list[i].status, "finalizado") != 0 && process_list[i].t0 <= time && process_list[i].dt_aux <= aux && i != pos) {
            if (process_list[i].dt_aux == aux) {
                if (process_list[i].t0 < process_list[pos].t0) {
                    pos = i;
                    aux = process_list[i].dt_aux;
                }
            } else {
                pos = i;
                aux = process_list[i].dt_aux;
            }
            if (strcmp(process_list[i].status, "pronto") == 0) {
                process_list[i].pos_process = 1;
            }
        }
    }
    return pos;
}

int get_q_size() {
    int q_size = 0;
    for (int i = 0; i < T_SIZE; i++) {
        if (process_list[i].pos_process == 1) {
            q_size++;
        }
    }
    return q_size;
}

int shortest_remaining_time_next(char *path_output) {
    int i = 0, l_in_exec;
    FILE *output_file;
    ctx_ch = 0;
    p_list_size = T_SIZE;
    output_file = fopen(path_output, "w");
    if (output_file == NULL) {
        printf("ERRO! Arquivo de saída não gerado!\n");
        return (EXIT_FAILURE);
    }
    while (p_list_size > 0) {
        if (i == 0) {
            int first = first_in_time();
            if (process_list[first].t0 > i) {
                i = process_list[first].t0;
            }
            start_thread(first);
            ctx_ch++;
            process_list[first].pos_process = 1;
            l_in_exec = first;
            sleep(1);
            process_list[first].dt_aux--;
            if (process_list[first].dt_aux == 0) {
                finish_process(first, i, output_file);
            }
            i++;
        } else {
            if (get_q_size() < T_SIZE) {
                int s_r_t = shortest_remaining_in_time(i);
                if (s_r_t != -1) {
                    int aux = 0;
                    if (s_r_t == l_in_exec) {
                        if (D == 1) {
                            fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[s_r_t].cpu, process_list[s_r_t].name);
                        }
                        sleep(1);
                        process_list[s_r_t].dt_aux--;
                        if (process_list[s_r_t].dt_aux == 0) {
                            finish_process(s_r_t, i, output_file);
                        }
                    } else {
                        ctx_ch++;
                        if (strcmp(process_list[s_r_t].status, "pronto") != 0) {
                            aux = 1;
                        }
                        if (strcmp(process_list[l_in_exec].status, "executando") == 0) {
                            change_ctx(s_r_t, l_in_exec);
                        } else {
                            if (strcmp(process_list[s_r_t].status, "pronto") == 0) {
                                start_thread(s_r_t);
                            } else {
                                sem_wait(&process_list[s_r_t].t_sem);
                            }
                        }
                        l_in_exec = s_r_t;
                        if (D == 1) {
                            if (aux > 0) {
                                fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[s_r_t].cpu, process_list[s_r_t].name);
                            }
                            aux++;
                        }
                        sleep(1);
                        process_list[s_r_t].dt_aux--;
                        if (process_list[s_r_t].dt_aux == 0) {
                            finish_process(s_r_t, i, output_file);
                        }
                    }
                    i++;
                } else {
                    if (strcmp(process_list[l_in_exec].status, "executando") == 0) {
                        if (D == 1) {
                            fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[l_in_exec].cpu, process_list[l_in_exec].name);
                        }
                        sleep(1);
                        process_list[l_in_exec].dt_aux--;
                        if (process_list[l_in_exec].dt_aux == 0) {
                            finish_process(l_in_exec, i, output_file);
                        }
                        i++;
                    } else {
                        i++;
                    }
                }
            } else {
                int s_r, aux = 0;
                if (strcmp(process_list[l_in_exec].status, "executando") == 0) {
                    if (D == 1) {
                        fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[l_in_exec].cpu, process_list[l_in_exec].name);
                    }
                    while (process_list[l_in_exec].dt_aux > 0) {
                        if (D == 1) {
                            if (aux > 0) {
                                fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[l_in_exec].cpu, process_list[l_in_exec].name);
                            }
                            aux++;
                        }
                        sleep(1);
                        process_list[l_in_exec].dt_aux--;
                        i++;
                    }
                    if (process_list[l_in_exec].dt_aux == 0) {
                        finish_process(l_in_exec, i - 1, output_file);
                    }
                } else {
                    ctx_ch++;
                    s_r = shortest_remaining();
                    if (strcmp(process_list[s_r].status, "pronto") == 0) {
                        start_thread(s_r);
                    } else {
                        aux = 1;
                        int sem_status;
                        sem_getvalue(&process_list[s_r].t_sem, &sem_status);
                        if (sem_status == 1) {
                            sem_wait(&process_list[s_r].t_sem);
                        }
                    }
                    while (process_list[s_r].dt_aux > 0) {
                        if (D == 1) {
                            if (aux > 0) {
                                fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[s_r].cpu, process_list[s_r].name);
                            }
                            aux++;
                        }
                        sleep(1);
                        process_list[s_r].dt_aux--;
                        i++;
                    }
                    if (process_list[s_r].dt_aux == 0) {
                        finish_process(s_r, i - 1, output_file);
                    }
                }
            }
        }
    }
    for (int i = 0; i < T_SIZE; i++) {
        pthread_join(process_list[i].td, NULL);
    }
    fprintf(output_file, "%d\n", ctx_ch);
    fclose(output_file);
    return (EXIT_SUCCESS);
}

int ready_in_time_rr(int time, int l_exec) {
    int aux, pos = -1;
    for (int i = 0; i < T_SIZE; i++) {
        if (strcmp(process_list[i].status, "finalizado") != 0 && process_list[i].t0 <= time && i != l_exec && process_list[i].pos_process != -1) {
            pos = i;
            aux = process_list[i].pos_process;
            break;
        }
    }
    for (int i = 0; i < T_SIZE; i++) {
        if (strcmp(process_list[i].status, "finalizado") != 0 && process_list[i].t0 <= time && i != l_exec && process_list[i].pos_process < aux && process_list[i].pos_process != -1) {
            pos = i;
            aux = process_list[i].pos_process;
        }
    }
    return pos;
}

void set_priority_rr(int time, int l_exec) {
    int aux, pos = -1, bef = -1;
    for (int i = 0; i < T_SIZE; i++) {
        for (int j = 0; j < T_SIZE; j++) {
            if (strcmp(process_list[j].status, "finalizado") != 0 && process_list[j].t0 <= time && j != l_exec && j != bef && process_list[j].pos_process == -1) {
                pos = j;
                aux = process_list[j].t0;
                break;
            }
        }
        for (int j = 0; j < T_SIZE; j++) {
            if (strcmp(process_list[j].status, "finalizado") != 0 && process_list[j].t0 <= time && process_list[j].t0 < aux && j != bef && j != l_exec && process_list[j].pos_process == -1) {
                pos = j;
                aux = process_list[j].t0;
            }
        }
        if (i != l_exec && process_list[pos].pos_process == -1) {
            process_list[pos].pos_process = r_size;
            r_size++;
            bef = pos;
        }
    }
    if (strcmp(process_list[l_exec].status, "executando") == 0) {
        process_list[l_exec].pos_process = r_size;
        r_size++;
    }
}

int round_robin(char *path_output, int quantum) {
    int i = 0, l_in_exec;
    FILE *output_file;
    ctx_ch = 0;
    p_list_size = T_SIZE;
    output_file = fopen(path_output, "w");
    if (output_file == NULL) {
        printf("ERRO! Arquivo de saída não gerado!\n");
        return (EXIT_FAILURE);
    }
    while (p_list_size > 0) {
        if (i == 0) {
            int first = first_in_time(), cont = 0, aux = 0;
            if (process_list[first].t0 > i) {
                i = process_list[first].t0;
            }
            start_thread(first);
            ctx_ch++;
            process_list[first].pos_process = r_size;
            r_size++;
            l_in_exec = first;
            while (cont < quantum) {
                if (process_list[first].dt_aux > 0) {
                    if (D == 1) {
                        if (aux > 0) {
                            fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[first].cpu, process_list[first].name);
                        }
                        aux++;
                    }
                    sleep(1);
                    process_list[first].dt_aux--;
                } else {
                    break;
                }
                cont++;
                i++;
            }
            if (process_list[first].dt_aux == 0) {
                finish_process(first, i - 1, output_file);
            }
        } else {
            int cont = 0, p_ready;
            set_priority_rr(i, l_in_exec);
            p_ready = ready_in_time_rr(i, l_in_exec);
            if (p_ready != -1) {
                int aux = 0;
                ctx_ch++;
                if (strcmp(process_list[p_ready].status, "pronto") != 0) {
                    aux = 1;
                }
                if (strcmp(process_list[l_in_exec].status, "executando") == 0) {
                    change_ctx(p_ready, l_in_exec);
                } else {
                    if (strcmp(process_list[p_ready].status, "pronto") == 0) {
                        start_thread(p_ready);
                    } else {
                        sem_wait(&process_list[p_ready].t_sem);
                    }
                }
                l_in_exec = p_ready;
                while (cont < quantum) {
                    if (process_list[p_ready].dt_aux > 0) {
                        if (D == 1) {
                            if (aux > 0) {
                                fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[p_ready].cpu, process_list[p_ready].name);
                            }
                            aux++;
                        }
                        sleep(1);
                        process_list[p_ready].dt_aux--;
                    } else {
                        break;
                    }
                    cont++;
                    i++;
                }
                if (process_list[p_ready].dt_aux == 0) {
                    finish_process(p_ready, i - 1, output_file);
                }
            } else {
                if (strcmp(process_list[l_in_exec].status, "executando") == 0) {
                    while (cont < quantum) {
                        if (process_list[l_in_exec].dt_aux > 0) {
                            if (D == 1) {
                                fprintf(stderr, "CPU %d sendo utilizada pelo %s.\n", process_list[l_in_exec].cpu, process_list[l_in_exec].name);
                            }
                            sleep(1);
                            process_list[l_in_exec].dt_aux--;
                        } else {
                            break;
                        }
                        cont++;
                        i++;
                    }
                    if (process_list[l_in_exec].dt_aux == 0) {
                        finish_process(l_in_exec, i - 1, output_file);
                    }
                } else {
                    i++;
                }
            }
        }
    }
    for (int i = 0; i < T_SIZE; i++) {
        pthread_join(process_list[i].td, NULL);
    }
    fprintf(output_file, "%d\n", ctx_ch);
    fclose(output_file);
    return (EXIT_SUCCESS);
}

int open_file(char *path_input) {
    FILE *input_file;
    char process_instruc[30], *sub1, *sub2, *sub3, *sub4;
    input_file = fopen(path_input, "r");
    if (input_file == NULL) {
        printf("ERRO! Arquivo de entrada não encontrado!\n");
        return (EXIT_FAILURE);
    }
    while (fgets(process_instruc, 200, input_file) != NULL) {
        sub1 = strtok(process_instruc, " ");
        if (strcmp(sub1, "\n") != 0) {
            sub2 = strtok(NULL, " ");
            sub3 = strtok(NULL, " ");
            sub4 = strtok(NULL, "");
            strcpy(process_list[T_SIZE].name, sub1);
            strcpy(process_list[T_SIZE].status, "pronto");
            process_list[T_SIZE].t0 = atoi(sub2);
            process_list[T_SIZE].dt = atoi(sub3);
            process_list[T_SIZE].dt_aux = process_list[T_SIZE].dt;
            process_list[T_SIZE].deadline = atoi(sub4);
            process_list[T_SIZE].pos_process = -1;
            if (D == 1) {
                fprintf(stderr, "%s %d %d %d incluído na fila de prontos.\n", process_list[T_SIZE].name, process_list[T_SIZE].t0, process_list[T_SIZE].dt, process_list[T_SIZE].deadline);
            }
            T_SIZE++;
        }

    }
    if (D == 1) {
        fprintf(stderr, "\n");
    }
    fclose(input_file);
    return (EXIT_SUCCESS);
}
