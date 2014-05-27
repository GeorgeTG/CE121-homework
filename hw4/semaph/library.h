#ifndef SHM_LIBRARY
#define SHM_LIBRARY

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define SHM_KEY 1337
#define SEM_FREE 10021995
#define SEM_FULL 17071995
#define SEM_MUTEX 14041879

typedef struct {
    int size;
    int in, out;
    char buf[];
} shm_st;

extern shm_st *shm_segment;
extern int shmid;
extern int free_sem, full_sem, mutex;

int buf_init(int n);
int buf_destroy();
int buf_put(char c);
int buf_get(char *c);

int _semget(key_t key);
int _semctl(int semid, int cmd, int arg);
int _semup(int semid);
int _semdown(int semid);

#endif
