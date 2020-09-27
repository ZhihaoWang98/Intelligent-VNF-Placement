#ifndef EP1_H
#define EP1_H

struct tProcess {
    char name[30], status[30];
    int cpu, id_proc, t0, dt, dt_aux, deadline, pos_process;
    pthread_t td;
    sem_t t_sem;
};
typedef struct tProcess tProcess;

tProcess process_list[150];
int T_SIZE;
int D;

int open_file(char *);
void t_process_show(int);
int get_cpu(int);
int get_ctx_ch(int);
void *thread_f(void *);
void start_thread(int);
void change_ctx(int, int);
int first_come_first_served(char *);
void finish_process(int pos, int i, FILE *);
int first_in_time(void);
int get_in_time(int);
int shortest_remaining_in_time(int);
int shortest_remaining(void);
int shortest_remaining_time_next(char *);
int ready_in_time_rr(int, int);
void set_priority_rr(int, int);
int round_robin(char *, int);

#endif
