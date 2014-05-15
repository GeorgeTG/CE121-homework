#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

#include "library.h"

int shmid;
shm_st *memory;

int buf_init(int n) {
    printf("%u\n", sizeof(shm_st));

    size_t totalSize = sizeof(shm_st) + n*sizeof(char);
    int retValue = 1;

    int _shmid = shmget(SHM_KEY, totalSize, IPC_CREAT | IPC_EXCL | S_IRWXU);
    if( _shmid < 0 ) {
        printf("ERROR\n");
        if( errno == EEXIST ) {
            _shmid = shmget(SHM_KEY, totalSize, 0);
            retValue = 0;
        }
        else {
            perror("shmget");
            return -1;
        }
    }
    printf("%d\n", _shmid);
    
    shmid = _shmid;
    memory = (shm_st*)shmat(shmid, NULL, 0);
    if( memory == (void*)-1 ) {
        perror("shmat");
        return -1;
    }
    
    printf("%p\n", memory);
    return retValue;
}
