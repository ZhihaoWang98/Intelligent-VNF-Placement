#ifndef EP3_H
#define EP3_H

#define BLK_SIZE 4096 //tamanho do bloco em bytes
#define NUM_BLOCK 24415 //disco de 100MB com blocos de 4KB
#define BIT_SIZE 6//24415 blocos * 1 byte por cada posição = 24415 bytes (6 blocos)
#define DIR_SIZE 3350//24415-6 blocos * 562 bytes por cada tArchive = 13717858 bytes (3350 blocos)

//char = 1 byte
//int = 4 bytes
//unsigned short int = 2 bytes

struct tTime {//12 bytes
    unsigned short int day, mon, year, hour, min, sec;
};
typedef struct tTime tTime;

struct tArchive {//562 bytes
    char name[256], fath[256]; //512 bytes
    unsigned short int type, block, son_qtd; //6 bytes
    int size, *son; //8 bytes
    tTime created, modified, accessed; //36 bytes
};
typedef struct tArchive tArchive;

struct tData {//4096 bytes
    char data[BLK_SIZE - 2]; //4094 bytes
    unsigned short int next; //2 bytes
};
typedef struct tData tData;

uint8_t bitmap[NUM_BLOCK]; //1 byte por bloco 
tArchive direct[DIR_SIZE];
tData data_v[NUM_BLOCK - DIR_SIZE - BIT_SIZE];

void df(void);
void touch(char*);
void cat(char*);
void updt_fath(int, int, int);
void delete(int);
void rm(char*);
void rmdir_(char*);
void mkdir_(char*);
int find_data_next(void);
int find_data(int);
int find_fat(void);
void ls(char*);
int find_pos(char*, char*);
void find(char*, char*);
void cp(char*, char*);
void set_time(int, int);
void save(void);
void start(void);
void mount_(void);
void free_fat(void);
void loop_commands(char*);
void test_mode(int, int);
char* build_arc(char*);

#endif
