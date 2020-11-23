#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "ep3.h"

FILE *arch;
char path[256];
int mounted = 0;
char *command_buff;

void df(void) {
    int dir_qtd = 0, arc_qtd = 0, free_spc = (NUM_BLOCK - BIT_SIZE) * (BLK_SIZE - 2), lost_spc = 0;
    fprintf(stdout, "------------Simulador FAT------------\n");
    for (int i = BIT_SIZE; i < DIR_SIZE + BIT_SIZE; i++) {
        if (bitmap[i] == 0) {
            if (direct[i - BIT_SIZE].type == 0) {
                dir_qtd++;
            } else if (direct[i - BIT_SIZE].type == 1) {
                arc_qtd++;
                free_spc -= direct[i - BIT_SIZE].size;
            }
        }
    }
    for (int i = DIR_SIZE + BIT_SIZE; i < NUM_BLOCK; i++) {
        if (bitmap[i] == 0) {
            for (int j = 0; j < BLK_SIZE - 2; j++) {
                if (data_v[i - (DIR_SIZE + BIT_SIZE)].data[j] == '|') {
                    lost_spc++;
                }
            }
        }
    }
    fprintf(stdout, "Diretórios: %d\n", dir_qtd);
    fprintf(stdout, "Arquivos: %d\n", arc_qtd);
    fprintf(stdout, "Espaço livre: %d bytes\n", free_spc);
    fprintf(stdout, "Espaço desperdiçado: %d bytes\n", lost_spc);
    fprintf(stdout, "-------------------------------------\n");
}

int find_pos(char *dir, char *arc) {
    for (int i = BIT_SIZE; i < DIR_SIZE + BIT_SIZE; i++) {
        if (bitmap[i] == 0) {
            if (strcmp(arc, "") == 0) {
                if (strcmp(direct[i - BIT_SIZE].name, dir) == 0 && direct[i - BIT_SIZE].type == 0) {
                    return i - BIT_SIZE;
                }
            }
            if (strcmp(dir, "") == 0) {
                if (strcmp(direct[i - BIT_SIZE].name, arc) == 0 && direct[i - BIT_SIZE].type == 1) {
                    return i - BIT_SIZE;
                }
            }
        }
    }
    return -1;
}

void touch(char *arc) {
    int fat_pos = find_pos("", arc);
    if (fat_pos == -1) {
        fprintf(stdout, "Arquivo não encontrado.\n");
        int free_fat_pos = find_fat(), data_pos = find_data(0);
        if (free_fat_pos == -1) {
            fprintf(stderr, "Erro! FAT sem espaço livre para criar um novo.\n");
        } else if (data_pos == -1) {
            fprintf(stderr, "Erro! Disco sem espaço livre para criar um novo.\n");
        } else {
            int fath, aux = -1;
            char dir_fath[256];
            for (int i = 0; i < strlen(arc); i++) {
                if (arc[i] == '/') {
                    aux = i;
                }
            }
            if (aux != -1) {
                if (aux == 0) {
                    dir_fath[0] = '/';
                    dir_fath[1] = '\0';
                } else {
                    for (int i = 0; i <= aux; i++) {
                        if (i == aux) {
                            dir_fath[i] = '\0';
                        } else {
                            dir_fath[i] = arc[i];
                        }
                    }
                }
                fath = find_pos(dir_fath, "");
            }
            if (fath == -1 || aux == -1 || direct[fath].type == 1) {
                fprintf(stderr, "Erro! Diretório pai não encontrado.\n");
            } else {
                strcpy(direct[free_fat_pos].name, arc);
                strcpy(direct[free_fat_pos].fath, dir_fath);
                direct[free_fat_pos].size = 0;
                direct[free_fat_pos].son = NULL;
                direct[free_fat_pos].son_qtd = 0;
                direct[free_fat_pos].type = 1;
                set_time(free_fat_pos, 0);
                set_time(free_fat_pos, 1);
                updt_fath(fath, free_fat_pos, 0);
                bitmap[free_fat_pos + BIT_SIZE] = 0;
                save();
                fprintf(stdout, "Novo arquivo criado.\n");
            }
        }
    } else {
        set_time(fat_pos, 2);
        save();
    }
}

void ls(char *dir) {
    int dir_pos = find_pos(dir, "");
    if (dir_pos == -1) {
        fprintf(stderr, "Erro! Diretório não encontrado.\n");
    } else if (direct[dir_pos].son_qtd == 0) {
        fprintf(stdout, "   Diretório %s vazio.\n", direct[dir_pos].name);
    } else {
        fprintf(stdout, "Diretório %s:\n", direct[dir_pos].name);
        for (int i = 0; i < direct[dir_pos].son_qtd; i++) {
            int son_pos = direct[dir_pos].son[i];
            if (direct[son_pos].type == 0) {
                ls(direct[son_pos].name);
            } else {
                fprintf(stdout, "   Arquivo %s com %d bytes e última modificação em %d/%d/%d às %d:%d:%d.\n",
                        direct[son_pos].name, direct[son_pos].size, direct[son_pos].modified.day, direct[son_pos].modified.mon, direct[son_pos].modified.year,
                        direct[son_pos].modified.hour, direct[son_pos].modified.min, direct[son_pos].modified.sec);
            }
        }
    }
}

void find(char *dir, char *arc) {
    int dir_pos = find_pos(dir, "");
    if (dir_pos == -1) {
        fprintf(stderr, "Erro! Diretório não encontrado.\n");
    } else if (direct[dir_pos].son_qtd == 0) {
        fprintf(stdout, "Diretório %s vazio.\n", direct[dir_pos].name);
    } else {
        for (int i = 0; i < direct[dir_pos].son_qtd; i++) {
            int son_pos = direct[dir_pos].son[i];
            if (strcmp(direct[son_pos].name, arc) == 0) {
                fprintf(stdout, "%s\n", direct[son_pos].name);
            }
        }
    }
}

void cat(char *arc) {
    int fat_pos = find_pos("", arc);
    if (fat_pos == -1) {
        fprintf(stderr, "Erro! Arquivo não encontrado.\n");
    } else if (direct[fat_pos].size > 0) {
        double blk_qtd = (double) direct[fat_pos].size / (double) (BLK_SIZE - 2);
        int init = direct[fat_pos].block, aux = blk_qtd;
        if (blk_qtd > aux) {
            aux++;
        }
        for (int i = 0; i < aux; i++) {
            for (int j = 0; j < BLK_SIZE - 2; j++) {
                if (data_v[init].data[j] != '|') {
                    fprintf(stdout, "%c", data_v[init].data[j]);
                }
            }
            init = data_v[init].next;
        }
    }
}

void delete(int fat_pos) {
    for (int i = 0; i < direct[fat_pos].son_qtd; i++) {
        int son_pos = direct[fat_pos].son[i];
        if (direct[son_pos].type == 1) {
            rm(direct[son_pos].name);
            fprintf(stdout, "Arquivo %s apagado.\n", direct[son_pos].name);
        } else {
            if (direct[son_pos].son_qtd > 0) {
                delete(son_pos);
                bitmap[son_pos + BIT_SIZE] = 1;
                fprintf(stdout, "Diretório %s apagado.\n", direct[son_pos].name);
            } else {
                bitmap[son_pos + BIT_SIZE] = 1;
                fprintf(stdout, "Diretório %s apagado.\n", direct[son_pos].name);
            }
        }
    }
}

void rm(char *arc) {
    int fat_pos = find_pos("", arc);
    if (fat_pos == -1) {
        fprintf(stderr, "Erro! Arquivo não encontrado.\n");
    } else if (direct[fat_pos].size > 0) {
        fprintf(stdout, "Removendo o arquivo %s\n", arc);
        double blk_qtd = (double) direct[fat_pos].size / (double) (BLK_SIZE - 2);
        int init = direct[fat_pos].block, aux = blk_qtd;
        if (blk_qtd > aux) {
            aux++;
        }
        for (int i = 0; i < aux; i++) {
            bitmap[init + DIR_SIZE + BIT_SIZE] = 1;
            init = data_v[init].next;
        }
        int fath = find_pos(direct[fat_pos].fath, "");
        updt_fath(fath, fat_pos, 1);
        bitmap[fat_pos + BIT_SIZE] = 1;
        save();
    } else {
        fprintf(stdout, "Removendo o arquivo %s\n", arc);
        int fath = find_pos(direct[fat_pos].fath, "");
        updt_fath(fath, fat_pos, 1);
        bitmap[fat_pos + BIT_SIZE] = 1;
        save();
    }
}

void rmdir_(char *dir) {
    if (strcmp(dir, "/") == 0) {
        fprintf(stderr, "Erro! Não é possível remover o diretório raiz.\n");
    } else {
        int fat_pos = find_pos(dir, "");
        if (fat_pos == -1) {
            fprintf(stderr, "Erro! Diretório não encontrado.\n");
        } else {
            if (direct[fat_pos].son_qtd > 0) {
                delete(fat_pos);
            }
            int fath = find_pos(direct[fat_pos].fath, "");
            updt_fath(fath, fat_pos, 1);
            bitmap[fat_pos + BIT_SIZE] = 1;
            fprintf(stdout, "Diretório %s apagado.\n", direct[fat_pos].name);
            save();
        }
    }
}

int find_fat(void) {
    for (int i = BIT_SIZE; i < DIR_SIZE + BIT_SIZE; i++) {
        if (bitmap[i] == 1) {
            return i - BIT_SIZE;
        }
    }
    return -1;
}

void updt_fath(int fath_pos, int son_pos, int op) {
    if (op == 0) {
        if (direct[fath_pos].son_qtd == 0) {
            direct[fath_pos].son_qtd++;
            direct[fath_pos].son = (int *) malloc(sizeof (int));
            direct[fath_pos].son[0] = son_pos;
        } else {
            direct[fath_pos].son_qtd++;
            int *vet_aux = (int *) malloc(direct[fath_pos].son_qtd * sizeof (int));
            for (int i = 0; i < direct[fath_pos].son_qtd - 1; i++) {
                vet_aux[i] = direct[fath_pos].son[i];
            }
            vet_aux[direct[fath_pos].son_qtd - 1] = son_pos;
            free(direct[fath_pos].son);
            direct[fath_pos].son = (int *) malloc(direct[fath_pos].son_qtd * sizeof (int));
            for (int i = 0; i < direct[fath_pos].son_qtd; i++) {
                direct[fath_pos].son[i] = vet_aux[i];
            }
            free(vet_aux);
        }
    } else {
        direct[fath_pos].son_qtd--;
        int *vet_aux = (int *) malloc(direct[fath_pos].son_qtd * sizeof (int)), pos = 0;
        for (int i = 0; i < direct[fath_pos].son_qtd + 1; i++) {
            if (direct[fath_pos].son[i] != son_pos) {
                vet_aux[pos] = direct[fath_pos].son[i];
                pos++;
            }
        }
        free(direct[fath_pos].son);
        direct[fath_pos].son = (int *) malloc(direct[fath_pos].son_qtd * sizeof (int));
        for (int i = 0; i < direct[fath_pos].son_qtd; i++) {
            direct[fath_pos].son[i] = vet_aux[i];
        }
        free(vet_aux);
    }
}

void mkdir_(char *dir) {
    int fat_pos = find_fat();
    if (fat_pos == -1) {
        fprintf(stderr, "Erro! FAT sem espaço livre.\n");
    } else {
        if (find_pos(dir, "") != -1) {
            fprintf(stderr, "Erro! Já existe um diretório com o mesmo nome no sistema.\n");
        } else {
            int fath, aux = -1;
            char dir_fath[256];
            for (int i = 0; i < strlen(dir); i++) {
                if (dir[i] == '/') {
                    aux = i;
                }
            }
            if (aux != -1) {
                if (aux == 0) {
                    dir_fath[0] = '/';
                    dir_fath[1] = '\0';
                } else {
                    for (int i = 0; i <= aux; i++) {
                        if (i == aux) {
                            dir_fath[i] = '\0';
                        } else {
                            dir_fath[i] = dir[i];
                        }
                    }
                }
                fath = find_pos(dir_fath, "");
            }
            if (fath == -1 || aux == -1 || direct[fath].type == 1) {
                fprintf(stderr, "Erro! Diretório pai não encontrado.\n");
            } else {
                strcpy(direct[fat_pos].name, dir);
                strcpy(direct[fat_pos].fath, dir_fath);
                direct[fat_pos].type = 0;
                direct[fat_pos].size = 0;
                direct[fat_pos].block = -1;
                direct[fat_pos].son = NULL;
                direct[fat_pos].son_qtd = 0;
                set_time(fat_pos, 0);
                set_time(fat_pos, 1);
                updt_fath(fath, fat_pos, 0);
                bitmap[fat_pos + BIT_SIZE] = 0;
                save();
            }
        }
    }
}

int find_data_next(void) {
    for (int i = DIR_SIZE + BIT_SIZE; i < NUM_BLOCK; i++) {
        if (bitmap[i] == 1) {
            return i - (DIR_SIZE + BIT_SIZE);
        }
    }
    return -1;
}

int find_data(int size) {
    int free = 0, pos = -1;
    for (int i = DIR_SIZE + BIT_SIZE; i < NUM_BLOCK; i++) {
        if (bitmap[i] == 1) {
            free++;
            if (pos == -1) {
                pos = i - (DIR_SIZE + BIT_SIZE);
            }
        }
    }
    if ((free * (BLK_SIZE - 2)) >= size) {
        return pos;
    } else {
        return -1;
    }
}

void cp(char *sou, char *tar) {
    int fat_pos = find_fat(), data_pos, size = 0, c_pos = 0;
    char buff;
    if (fat_pos == -1) {
        fprintf(stderr, "Erro! FAT sem espaço livre.\n");
    } else {
        FILE *input = fopen(sou, "r");
        if (input == NULL) {
            fprintf(stderr, "Erro! Arquivo de origem não encontrado.\n");
        } else {
            if (find_pos("", tar) != -1) {
                fprintf(stderr, "Erro! Já existe um arquivo com o mesmo nome no sistema.\n");
            } else {
                while ((buff = getc(input)) != EOF) {
                    size++;
                }
                fclose(input);
                data_pos = find_data(size);
                if (data_pos == -1) {
                    fprintf(stderr, "Erro! Disco sem espaço livre.\n");
                } else {
                    int fath, aux;
                    char dir_fath[256];
                    for (int i = 0; i < strlen(tar); i++) {
                        if (tar[i] == '/') {
                            aux = i;
                        }
                    }
                    if (aux == 0) {
                        dir_fath[0] = '/';
                        dir_fath[1] = '\0';
                    } else {
                        for (int i = 0; i <= aux; i++) {
                            if (i == aux) {
                                dir_fath[i] = '\0';
                            } else {
                                dir_fath[i] = tar[i];
                            }
                        }
                    }
                    fath = find_pos(dir_fath, "");
                    if (fath == -1 || aux == -1 || direct[fath].type == 1) {
                        fprintf(stderr, "Erro! Diretório pai não encontrado.\n");
                    } else {
                        fprintf(stdout, "Copiando o arquivo %s para %s\n", sou, tar);
                        input = fopen(sou, "r");
                        strcpy(direct[fat_pos].name, tar);
                        strcpy(direct[fat_pos].fath, dir_fath);
                        direct[fat_pos].size = size;
                        direct[fat_pos].son = NULL;
                        direct[fat_pos].son_qtd = 0;
                        direct[fat_pos].type = 1;
                        direct[fat_pos].block = data_pos;
                        set_time(fat_pos, 0);
                        set_time(fat_pos, 1);
                        updt_fath(fath, fat_pos, 0);
                        bitmap[fat_pos + BIT_SIZE] = 0;
                        bitmap[data_pos + DIR_SIZE + BIT_SIZE] = 0;
                        while ((buff = getc(input)) != EOF) {
                            if ((c_pos == BLK_SIZE - 3) && size > 1) {
                                int aux = find_data_next();
                                bitmap[aux + DIR_SIZE + BIT_SIZE] = 0;
                                data_v[data_pos].data[c_pos] = buff;
                                data_v[data_pos].next = aux;
                                data_pos = aux;
                                c_pos = 0;
                            } else {
                                data_v[data_pos].data[c_pos] = buff;
                                c_pos++;
                            }
                            size--;
                        }
                        fclose(input);
                        save();
                    }
                }
            }
        }
    }
}

void set_time(int arch, int var) {
    struct tm *now;
    time_t seconds;
    time(&seconds);
    now = localtime(&seconds);
    if (var == 0) {
        direct[arch].created.day = now->tm_mday;
        direct[arch].created.mon = now->tm_mon + 1;
        direct[arch].created.year = now->tm_year + 1900;
        direct[arch].created.hour = now->tm_hour;
        direct[arch].created.min = now->tm_min;
        direct[arch].created.sec = now->tm_sec;
    } else if (var == 1) {
        direct[arch].modified.day = now->tm_mday;
        direct[arch].modified.mon = now->tm_mon + 1;
        direct[arch].modified.year = now->tm_year + 1900;
        direct[arch].modified.hour = now->tm_hour;
        direct[arch].modified.min = now->tm_min;
        direct[arch].modified.sec = now->tm_sec;
    } else {
        direct[arch].accessed.day = now->tm_mday;
        direct[arch].accessed.mon = now->tm_mon + 1;
        direct[arch].accessed.year = now->tm_year + 1900;
        direct[arch].accessed.hour = now->tm_hour;
        direct[arch].accessed.min = now->tm_min;
        direct[arch].accessed.sec = now->tm_sec;
    }
}

void save(void) {
    arch = fopen(path, "wb");
    if (arch == NULL) {
        fprintf(stderr, "Erro! Não foi possível salvar o sistema de arquivos.\n");
    } else {
        fwrite(&bitmap, sizeof (bitmap), 1, arch);
        fwrite(&direct, sizeof (direct), 1, arch);
        for (int i = 0; i < DIR_SIZE; i++) {
            fwrite(&direct[i].son_qtd, sizeof (direct[i].son_qtd), 1, arch);
            for (int j = 0; j < direct[i].son_qtd; j++) {
                fwrite(&direct[i].son[j], sizeof (direct[i].son[j]), 1, arch);
            }
        }
        fwrite(&data_v, sizeof (data_v), 1, arch);
        fclose(arch);
    }
}

void start(void) {
    for (int i = 0; i < NUM_BLOCK; i++) {
        bitmap[i] = 1;
    }
    for (int i = 0; i < DIR_SIZE; i++) {
        direct[i].type = -1;
    }
    for (int i = 0; i < (NUM_BLOCK - DIR_SIZE - BIT_SIZE); i++) {
        for (int j = 0; j < BLK_SIZE - 2; j++) {
            data_v[i].data[j] = '|';
        }
    }
    strcpy(direct[0].name, "/");
    direct[0].type = 0;
    direct[0].size = 0;
    direct[0].block = -1;
    direct[0].son = NULL;
    direct[0].son_qtd = 0;
    set_time(0, 0);
    bitmap[BIT_SIZE] = 0;
    save();
}

void mount_(void) {
    arch = fopen(path, "rb");
    if (arch != NULL) {
        fprintf(stdout, "Sistema de arquivos encontrado.\n");
        fread(&bitmap, sizeof (bitmap), 1, arch);
        fread(&direct, sizeof (direct), 1, arch);
        for (int i = 0; i < DIR_SIZE; i++) {
            fread(&direct[i].son_qtd, sizeof (direct[i].son_qtd), 1, arch);
            direct[i].son = (int *) malloc(direct[i].son_qtd * sizeof (direct[i].son));
            for (int j = 0; j < direct[i].son_qtd; j++) {
                fread(&direct[i].son[j], sizeof (direct[i].son[j]), 1, arch);
            }
        }
        fread(&data_v, sizeof (data_v), 1, arch);
        fclose(arch);
        mounted = 1;
        fprintf(stdout, "Montagem concluída.\n");
    } else {
        fprintf(stdout, "Sistema de arquivos não encontrado. Um novo será criado.\n");
        start();
        mount_();
    }
}

void free_fat(void) {
    for (int i = 0; i < DIR_SIZE; i++) {
        if (direct[i].son != NULL) {
            free(direct[i].son);
        }
    }
}

void loop_commands(char *current) {
    char *sub1, *sub2, *sub3;
    command_buff = readline(current);
    if (strlen(command_buff) > 0) {
        add_history(command_buff);
    }
    sub1 = strtok(command_buff, " ");
    if (sub1 != NULL) {
        if (strcmp(sub1, "mount") == 0) {
            sub2 = strtok(NULL, " ");
            strcpy(path, sub2);
            if (mounted == 0) {
                mount_();
            } else {
                fprintf(stderr, "Sistema de arquivos já montado.\n");
            }
        } else if (strcmp(sub1, "cp") == 0) {
            sub2 = strtok(NULL, " ");
            sub3 = strtok(NULL, "");
            if (mounted == 1) {
                clock_t t = clock();
                cp(sub2, sub3);
                fprintf(stdout, "%.2lf segundos.\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "mkdir") == 0) {
            sub2 = strtok(NULL, "");
            if (mounted == 1) {
                mkdir_(sub2);
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "rmdir") == 0) {
            sub2 = strtok(NULL, "");
            if (mounted == 1) {
                clock_t t = clock();
                rmdir_(sub2);
                fprintf(stdout, "%.2lf segundos.\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "cat") == 0) {
            sub2 = strtok(NULL, "");
            if (mounted == 1) {
                cat(sub2);
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "touch") == 0) {
            sub2 = strtok(NULL, "");
            if (mounted == 1) {
                touch(sub2);
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "rm") == 0) {
            sub2 = strtok(NULL, "");
            if (mounted == 1) {
                clock_t t = clock();
                rm(sub2);
                fprintf(stdout, "%.2lf segundos.\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "ls") == 0) {
            sub2 = strtok(NULL, "");
            if (mounted == 1) {
                ls(sub2);
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "find") == 0) {
            sub2 = strtok(NULL, " ");
            sub3 = strtok(NULL, "");
            if (mounted == 1) {
                find(sub2, sub3);
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "df") == 0) {
            if (mounted == 1) {
                df();
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "umount") == 0) {
            if (mounted == 1) {
                save();
                mounted = 0;
                fprintf(stdout, "Sistema de arquivos desmontado.\n");
            } else {
                fprintf(stderr, "ERRO! É necessário montar o sistema de arquivos antes de fazer essa operação.\n");
            }
        } else if (strcmp(sub1, "sair") == 0 || strcmp(sub1, "exit") == 0 || strcmp(sub1, "sai") == 0) {
            if (mounted == 1) {
                save();
                free_fat();
            }
            exit(EXIT_SUCCESS);
        }
    }
}

char* build_arc(char *size) {
    char *arc = malloc(20 * sizeof (char));
    strcpy(arc, "test_arch_");
    strcat(arc, size);
    strcat(arc, ".txt");
    srand(time(NULL));
    FILE *test = fopen(arc, "w");
    for (int i = 0; i < atoi(size)*1000000; i++) {
        int aux = 97 + (rand() % 25);
        if (i != 0 && (i % 100) == 0) {
            fprintf(test, "\n");
        } else {
            if (aux == 110) {
                fprintf(test, " ");
            } else {
                fprintf(test, "%c", (char) aux);
            }
        }
    }
    fclose(test);
    return arc;
}

void test_mode(int op, int state) {
    char arch[20], arch_a[20];
    if (state == 1) {
        strcpy(path, "test_state_1.fat");
        if (op == 1) {
            strcpy(arch, build_arc("1"));
            for (int i = 0; i < 30; i++) {
                mount_();
                clock_t t = clock();
                cp(arch, "/teste1.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
        } else if (op == 2) {
            strcpy(arch, build_arc("10"));
            for (int i = 0; i < 30; i++) {
                mount_();
                clock_t t = clock();
                cp(arch, "/teste2.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
        } else if (op == 3) {
            strcpy(arch, build_arc("30"));
            for (int i = 0; i < 30; i++) {
                mount_();
                clock_t t = clock();
                cp(arch, "/teste3.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
        } else if (op == 4) {
            strcpy(arch, build_arc("1"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch, "/teste4.txt");
                clock_t t = clock();
                rm("/teste4.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
        } else if (op == 5) {
            strcpy(arch, build_arc("10"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch, "/teste5.txt");
                clock_t t = clock();
                rm("/teste5.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
        } else if (op == 6) {
            strcpy(arch, build_arc("30"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch, "/teste6.txt");
                clock_t t = clock();
                rm("/teste6.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
        } else if (op == 7) {
            for (int i = 0; i < 30; i++) {
                mount_();
                char d[500] = "/dir1";
                mkdir_(d);
                fprintf(stdout, "Montando a hierarquia de diretórios...\n");
                for (int j = 0; j < 30; j++) {
                    char d_aux[10] = "/dir", aux[10];
                    sprintf(aux, "%d", (j + 2));
                    strcat(d_aux, aux);
                    strcat(d, d_aux);
                    mkdir_(d);
                }
                clock_t t = clock();
                rmdir_("/dir1");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
        } else if (op == 8) {
            for (int i = 0; i < 30; i++) {
                mount_();
                char d[500] = "/dir1", arc[500];
                mkdir_(d);
                strcpy(arc, d);
                strcat(arc, "/arch1");
                touch(arc);
                fprintf(stdout, "Montando a hierarquia de diretórios...\n");
                for (int j = 0; j < 30; j++) {
                    char d_aux[10] = "/dir", a_aux[10] = "/arch", aux[10];
                    sprintf(aux, "%d", (j + 2));
                    strcat(d_aux, aux);
                    strcat(d, d_aux);
                    mkdir_(d);
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                    strcpy(a_aux, "/arch");
                    sprintf(aux, "%d", (j + 3));
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                    strcpy(a_aux, "/arch");
                    sprintf(aux, "%d", (j + 4));
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                    strcpy(a_aux, "/arch");
                    sprintf(aux, "%d", (j + 5));
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                }
                clock_t t = clock();
                rmdir_("/dir1");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
        } else {
            fprintf(stderr, "Erro! Opção para o teste inválida.\n");
            exit(EXIT_FAILURE);
        }
    } else if (state == 2) {
        strcpy(path, "test_state_2.fat");
        if (op == 1) {
            strcpy(arch_a, build_arc("10"));
            strcpy(arch, build_arc("1"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado10.txt");
                clock_t t = clock();
                cp(arch, "/teste1.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 2) {
            strcpy(arch_a, build_arc("10"));
            strcpy(arch, build_arc("10"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado10.txt");
                clock_t t = clock();
                cp(arch, "/teste2.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 3) {
            strcpy(arch_a, build_arc("10"));
            strcpy(arch, build_arc("30"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado10.txt");
                clock_t t = clock();
                cp(arch, "/teste3.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 4) {
            strcpy(arch_a, build_arc("10"));
            strcpy(arch, build_arc("1"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado10.txt");
                cp(arch, "/teste4.txt");
                clock_t t = clock();
                rm("/teste4.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 5) {
            strcpy(arch_a, build_arc("10"));
            strcpy(arch, build_arc("10"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado10.txt");
                cp(arch, "/teste5.txt");
                clock_t t = clock();
                rm("/teste5.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 6) {
            strcpy(arch_a, build_arc("10"));
            strcpy(arch, build_arc("30"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado10.txt");
                cp(arch, "/teste6.txt");
                clock_t t = clock();
                rm("/teste6.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 7) {
            strcpy(arch_a, build_arc("10"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado10.txt");
                char d[500] = "/dir1";
                mkdir_(d);
                fprintf(stdout, "Montando a hierarquia de diretórios...\n");
                for (int j = 0; j < 30; j++) {
                    char d_aux[10] = "/dir", aux[10];
                    sprintf(aux, "%d", (j + 2));
                    strcat(d_aux, aux);
                    strcat(d, d_aux);
                    mkdir_(d);
                }
                clock_t t = clock();
                rmdir_("/dir1");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch_a);
        } else if (op == 8) {
            strcpy(arch_a, build_arc("10"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado10.txt");
                char d[500] = "/dir1", arc[500];
                mkdir_(d);
                strcpy(arc, d);
                strcat(arc, "/arch1");
                touch(arc);
                fprintf(stdout, "Montando a hierarquia de diretórios...\n");
                for (int j = 0; j < 30; j++) {
                    char d_aux[10] = "/dir", a_aux[10] = "/arch", aux[10];
                    sprintf(aux, "%d", (j + 2));
                    strcat(d_aux, aux);
                    strcat(d, d_aux);
                    mkdir_(d);
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                    strcpy(a_aux, "/arch");
                    sprintf(aux, "%d", (j + 3));
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                    strcpy(a_aux, "/arch");
                    sprintf(aux, "%d", (j + 4));
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                    strcpy(a_aux, "/arch");
                    sprintf(aux, "%d", (j + 5));
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                }
                clock_t t = clock();
                rmdir_("/dir1");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch_a);
        } else {
            fprintf(stderr, "Erro! Opção para o teste inválida.\n");
            exit(EXIT_FAILURE);
        }
    } else if (state == 3) {
        strcpy(path, "test_state_3.fat");
        if (op == 1) {
            strcpy(arch_a, build_arc("50"));
            strcpy(arch, build_arc("1"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado50.txt");
                clock_t t = clock();
                cp(arch, "/teste1.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 2) {
            strcpy(arch_a, build_arc("50"));
            strcpy(arch, build_arc("10"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado50.txt");
                clock_t t = clock();
                cp(arch, "/teste2.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 3) {
            strcpy(arch_a, build_arc("50"));
            strcpy(arch, build_arc("30"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado50.txt");
                clock_t t = clock();
                cp(arch, "/teste3.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 4) {
            strcpy(arch_a, build_arc("50"));
            strcpy(arch, build_arc("1"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado50.txt");
                cp(arch, "/teste4.txt");
                clock_t t = clock();
                rm("/teste4.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 5) {
            strcpy(arch_a, build_arc("50"));
            strcpy(arch, build_arc("10"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado50.txt");
                cp(arch, "/teste5.txt");
                clock_t t = clock();
                rm("/teste5.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 6) {
            strcpy(arch_a, build_arc("50"));
            strcpy(arch, build_arc("30"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado50.txt");
                cp(arch, "/teste6.txt");
                clock_t t = clock();
                rm("/teste6.txt");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch);
            remove(arch_a);
        } else if (op == 7) {
            strcpy(arch_a, build_arc("50"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado50.txt");
                char d[500] = "/dir1";
                mkdir_(d);
                fprintf(stdout, "Montando a hierarquia de diretórios...\n");
                for (int j = 0; j < 30; j++) {
                    char d_aux[10] = "/dir", aux[10];
                    sprintf(aux, "%d", (j + 2));
                    strcat(d_aux, aux);
                    strcat(d, d_aux);
                    mkdir_(d);
                }
                clock_t t = clock();
                rmdir_("/dir1");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch_a);
        } else if (op == 8) {
            strcpy(arch_a, build_arc("50"));
            for (int i = 0; i < 30; i++) {
                mount_();
                cp(arch_a, "/ocupado50.txt");
                char d[500] = "/dir1", arc[500];
                mkdir_(d);
                strcpy(arc, d);
                strcat(arc, "/arch1");
                touch(arc);
                fprintf(stdout, "Montando a hierarquia de diretórios...\n");
                for (int j = 0; j < 30; j++) {
                    char d_aux[10] = "/dir", a_aux[10] = "/arch", aux[10];
                    sprintf(aux, "%d", (j + 2));
                    strcat(d_aux, aux);
                    strcat(d, d_aux);
                    mkdir_(d);
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                    strcpy(a_aux, "/arch");
                    sprintf(aux, "%d", (j + 3));
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                    strcpy(a_aux, "/arch");
                    sprintf(aux, "%d", (j + 4));
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                    strcpy(a_aux, "/arch");
                    sprintf(aux, "%d", (j + 5));
                    strcpy(arc, d);
                    strcat(a_aux, aux);
                    strcat(arc, a_aux);
                    touch(arc);
                }
                clock_t t = clock();
                rmdir_("/dir1");
                fprintf(stdout, "%.2lf\n", ((double) (clock() - t)) / CLOCKS_PER_SEC);
                remove(path);
            }
            remove(arch_a);
        } else {
            fprintf(stderr, "Erro! Opção para o teste inválida.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Erro! Estado do sistema de arquivos inválido.\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}