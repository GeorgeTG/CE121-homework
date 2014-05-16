#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

#include "debug.h"
#include "library.h"

int shmid;
shm_st *memory;

int buf_init(int n) {

    debug("Size of shm_st: %zu\n", sizeof(shm_st));

    size_t totalSize = sizeof(shm_st) + n*sizeof(char);
    int retValue = 1;

    int _shmid = shmget(SHM_KEY, totalSize, IPC_CREAT | IPC_EXCL | S_IRWXU);
    if( _shmid < 0 ) {
        debug("_shmid < 0!\n");
        if( errno == EEXIST ) {
            _shmid = shmget(SHM_KEY, totalSize, 0);
            retValue = 0;
        }
        else {
            log_err("shmget");
            return -1;
        }
    }
    printf("%d\n", _shmid);

    shmid = _shmid;
    memory = (shm_st*)shmat(shmid, NULL, 0);
    if( memory == (void*)-1 ) {
        log_err("shmat");
        return -1;
    }

    debug("Memory pointer is: %p\n", memory);
    return retValue;
}
