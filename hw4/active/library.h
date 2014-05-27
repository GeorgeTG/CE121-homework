#ifndef SHM_LIBRARY
#define SHM_LIBRARY

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define SHM_KEY 1337

typedef struct {
    int size;
    int in,out;
    char mutex;
    char buf[];
} shm_st;

extern shm_st *shm_segment;
extern int shmid;

int buf_init(int n);
int buf_destroy();
int buf_put(char c);
int buf_get(char *c);

#endif
