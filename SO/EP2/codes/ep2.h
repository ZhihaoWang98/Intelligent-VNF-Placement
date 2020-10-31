#ifndef EP2_H
#define EP2_H

struct tCyclist {
    int num, speed, met, line, f_pos, br;
    double *time, f_time;
    char **status;
    pthread_t td;
};
typedef struct tCyclist tCyclist;

struct tTrack {
    int line[10], size;
    pthread_mutex_t lock_t;
};
typedef struct tTrack tTrack;

tCyclist *CYCL;
tTrack *TRACK;
int T_SIZE, C_NUM, L_SIZE, D;

void decrease(int);
int eliminated(int);
int elimination(int, int);
void run(int, int, clock_t);
void *thread_f(void *);
void start_running(void);
void start_cycl(void);
void start_track(void);
void show_report(int);
void finish_running(void);
int competition(int, int, int);

#endif