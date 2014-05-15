#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef SHM_LIBRARY
#define SHM_LIBRARY

#define SHM_KEY 1337

typedef struct {
    int in,out;
    char buf[];
} shm_st;

extern shm_st *memory;
extern int shmid;

int buf_init(int n);
int buf_destroy();
int buf_put(char c);
int buf_get(char *c);

#endif
