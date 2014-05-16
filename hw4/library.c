#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

#include "debug.h"
#include "library.h"

/*#define NDEBUG*/
/* ipcs -m !!!!! Terminal command to show all shared memory segments!!!
 * ipcrm -m <shmid> !!!! Terminal command to destroy segment!! */

int shmid;
shm_st *shm_segment;

int buf_init(int n) {

    debug("Size of shm_st: %zu\n", sizeof(shm_st));

    size_t totalSize = sizeof(shm_st) + n*sizeof(char);
    int retValue = 1;

    int _shmid = shmget(SHM_KEY, totalSize, IPC_CREAT | IPC_EXCL | S_IRWXU);
    if( _shmid < 0 ) {
        log_warn("_shmid < 0!\n");
        if( errno == EEXIST ) {
            _shmid = shmget(SHM_KEY, totalSize, 0);
            retValue = 0;
        }
        else {
            log_err("shmget");
            return -1;
        }
    }

    debug("_shmid: %d\n", _shmid);

    shmid = _shmid;
    shm_segment = (shm_st*)shmat(shmid, NULL, 0);
    if( shm_segment == (void*)-1 ) {
        log_err("shmat");
        return -1;
    }

    debug("Memory pointer is: %p\n", shm_segment);
    return retValue;
}

