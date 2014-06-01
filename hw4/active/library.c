#include "library.h"

#define RET_FAIL -1
#define RET_SUCCESS 1
#define RET_PASS 0

#define USLEEP_TIME 200

#define is_mem_ok(M) (M != NULL && M!=(void*)-1)

#define NDEBUG

#include "debug.h"

int shmid;
shm_st *shm_segment = NULL;

int buf_init(int n) {
    debug("Key: %d", SHM_KEY);
    debug("Lean struct size: %zu", sizeof(*shm_segment) );
    /* we need n+1 size since we have to sacriffice 1 spot in the
     * array in order to use circular-buffer related modulo calculations
     * properly */
    n++;
    size_t totalSize = sizeof(shm_st) + n*sizeof(char);
    debug("Total size: %zu", totalSize);

    int retValue = RET_SUCCESS;
    /* Try to create a NEW shared memory segment */
    int _shmid = shmget(SHM_KEY, totalSize, IPC_CREAT | IPC_EXCL | S_IRWXU);
    if( _shmid < 0 ) {
        log_warn("_shmid < 0!\n");
        if( errno == EEXIST ) {
            log_info("Segment already exists!");

            /* Call without options to get existant segment ID */
            _shmid = shmget(SHM_KEY, totalSize, 0);
            if (_shmid < 0) {
                log_err("shmget");
                return RET_FAIL;
            }

            /* segment already existed and we got ID successfully */
            retValue = RET_PASS;
        }
        else {
            log_err("shmget");
            return RET_FAIL;
        }
    }
    debug("_shmid: %d\n", _shmid);

    /* attach segment */
    shmid = _shmid;
    shm_segment = (shm_st*)shmat(shmid, NULL, 0);
    if( shm_segment == (void*)-1 ) {
        log_err("shmat");
        return RET_FAIL;
    }

    debug("Attached ID: [%d] at [%p].\n", shmid, shm_segment);

    if (retValue > 0 ) {
        /* Initialize structure */
        shm_segment->size = n;
        shm_segment->in = shm_segment->out = 0;
        shm_segment->mutex = 0;
    }
    return retValue;
}

int buf_destroy(void) {
    int retValue = shmctl(shmid, IPC_RMID, NULL);
    if ( retValue < 0 ) {
        if ( errno == EIDRM ) {
            log_warn("shmid: %d already removed!", shmid);
            return RET_PASS;
        }

        log_err("shmctl");
        return RET_FAIL;
    }
    debug("segment: [%d] scheduled for deletion.", shmid);

    retValue = shmdt(shm_segment);
    if ( retValue < 0 ){
        log_err("shmdt");
        return RET_FAIL;
    }

    /* NULLize the struct */
    shm_segment = NULL;

    /* Everything went OK */
    return RET_SUCCESS;
}

int buf_put(char c) {
    /* This kind of Mutex technique is STILL NOT thread SAFE!!! */
    if ( !is_mem_ok(shm_segment) ){
        log_err("Buffer not initialized.");
        return RET_FAIL;
    }

    /* Busy loop, waiting for buffer to get an empty slot */
    int nextPos = (shm_segment->in + 1) % shm_segment->size;
    while ( shm_segment->out == nextPos ){
        usleep( USLEEP_TIME );
    }

    debug("Trying to put char [%c] in %d, in: %d, out: %d", c,
            shm_segment->in,
            shm_segment->in,
            shm_segment->out
        );

    /* Dirty semaphore to limit simulaneous function calls */
    while( shm_segment->mutex == 1) {
        usleep( USLEEP_TIME );
    }
    shm_segment->mutex = 1;

    /**** Critical Section ****/
    (shm_segment->buf)[shm_segment->in] = c;

    debug("We just put char [%c] in buffer @ pos: %d", c, nextPos);

    shm_segment->in = nextPos;

    debug("buf_put: positions state--> in: %d, out: %d",
            shm_segment->in,
            shm_segment->out
        );
    /************************/
    /* Unlock the shared memory */
    shm_segment->mutex = 0;

    return RET_SUCCESS;
}

int buf_get(char *c){
    if ( !is_mem_ok(shm_segment) ){
            log_err("Buffer not initialized.");
            return RET_FAIL;
    }
    debug("Trying to get char from buffer, in: %d, out: %d",
            shm_segment->in,
            shm_segment->out
        );

    /* Busy loop, waiting for buffer to fill so we can read */
    while( shm_segment->in - shm_segment->out == 0){
        usleep( USLEEP_TIME );
    }

    /* Dirty semaphore to limit simulaneous function calls */
    while( shm_segment->mutex == 1) {
        usleep( USLEEP_TIME );
    }
    shm_segment->mutex = 1;

    /**** Critical Section ****/
    /* Extract next character from buffer */
    *c = (shm_segment->buf)[shm_segment->out];

    /* Increment out */
    shm_segment->out = (shm_segment->out + 1) % shm_segment->size;

    debug("Got char %c from buffer, in: %d, out: %d", *c,
        shm_segment->in,
        shm_segment->out
    );

    /*************************/
    /* Unlock the shared memory */
    shm_segment->mutex = 0;

    return RET_SUCCESS;
}


