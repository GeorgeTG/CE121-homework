#ifndef SHM_LIBRARY

#define SHM_LIBRARY

#define SHM_KEY 1337
#define SEM_GP 10021995
#define SEM_IO 17071995

typedef struct {
    int size;
    int in, out;
    char buf[];
} shm_st;

extern shm_st *shm_segment;
extern int shmid;
extern int gp_semid, io_semid;

int buf_init(int n);
int buf_destroy();
int buf_put(char c);
int buf_get(char *c);

#endif
