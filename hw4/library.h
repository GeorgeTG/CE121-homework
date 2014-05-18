#ifndef SHM_LIBRARY

#define SHM_LIBRARY

#define SHM_KEY 1337

typedef struct {
    int in,out;
    char buf[];
} shm_st;

extern shm_st *shm_segment;
extern int shmid, in, out;

int buf_init(int n);
int buf_destroy();
int buf_put(char c);
int buf_get(char *c);

#endif
