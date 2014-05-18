#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "debug.h"
#include "library.h"

/*#define NDEBUG*/
/* ipcs -m !!!!! Terminal command to show all shared memory segments!!!
 * ipcrm -m <shmid> !!!! Terminal command to destroy segment!! */

int shmid, in, out;
shm_st *shm_segment = NULL;

int buf_init(int n) {
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
    else {
        /* init in,out values */
        in = out = 0;
    }

    debug("_shmid: %d\n", _shmid);

    shmid = _shmid;
    shm_segment = (shm_st*)shmat(shmid, NULL, 0);
    if( shm_segment == (void*)-1 ) {
        log_err("shmat");
        return -1;
    }

    debug("Attached ID: [%d] at [%p].\n", shmid, shm_segment);
    return retValue;
}

int buf_destroy(void) {
    int reValue = shmctl(shmid, IPC_RMID, NULL);
    if ( reValue < 0 ) {
        if ( errno == EIDRM ) {
            log_warn("shmid: %d already removed!", shmid);
            return 0;
        }

        log_err("shmctl");
        return -1;
    }
    debug("[%d] scheduled for deletion.", shmid);
    /* Everything went OK */
    return 1;
}

int buf_put(char c) {
    if ( shm_segment == NULL ){
        log_err("Buffer not initialized.");
        return -1;
    }


}


