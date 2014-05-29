#include "library.h"

#define RET_FAIL -1
#define RET_SUCCESS 1
#define RET_PASS 0

#define USLEEP_TIME 200

#define IS_MEM_OK(M) (M != NULL && M!=(void*)-1)

/* ipcs -m !!!!! Terminal command to show all shared memory segments!!!
 * ipcrm -m <shmid> !!!! Terminal command to destroy segment!! */

#include "debug.h"

int shmid;
int free_sem, full_sem, mutex;
shm_st *shm_segment = NULL;

int buf_init(int n) {
    debug("Key: %d", SHM_KEY);
    debug("Lean struct size: %zu", sizeof(*shm_segment) );
    
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
    
    /* Attach the shared memory to proccess */
    shmid = _shmid;
    shm_segment = (shm_st*)shmat(shmid, NULL, 0);
    if( shm_segment == (void*)-1 ) {
        log_err("shmat");
        return RET_FAIL;
    }
    debug("Attached ID: [%d] at [%p].\n", shmid, shm_segment);

    /* Get the free_sem semaphores */
    free_sem = _semget(SEM_FREE);
    if( free_sem < 0 ) {
        return RET_FAIL;
    }
    
    /* Get the full_sem semaphores */
    full_sem = _semget(SEM_FULL);
    if( full_sem < 0 ) {
        return RET_FAIL;
    }

    /* Get the mutex semaphores */
    mutex = _semget(SEM_MUTEX);
    if( mutex < 0 ) {
        return RET_FAIL;
    }

    /* Increase the proccess count */
    shm_segment->nproc++;

    if (retValue > 0 ){
        /* Init struct */
        shm_segment->size = n;
        shm_segment->in = shm_segment->out = 0;
        shm_segment->nproc = 1;

        /* Init semaphores */
        if( _semctl(free_sem, SETVAL, n) == RET_FAIL )
            return RET_FAIL;
        if( _semctl(full_sem, SETVAL, 0) == RET_FAIL )
            return RET_FAIL;
        if( _semctl(mutex, SETVAL, 1) == RET_FAIL )
            return RET_FAIL;

    }
    return retValue;
}

int buf_destroy(void) {
    if ( !IS_MEM_OK(shm_segment) ){
        log_err("Buffer not initialized.");
        return RET_FAIL;
    }
    /* Decrease the number of proccess */
    int nproc = --(shm_segment->nproc);

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
    
    if( nproc == 0 ) {
        /* Set for destruction the semaphores */
        if( _semctl(free_sem, IPC_RMID, 0) )
            return RET_FAIL;

        if( _semctl(full_sem, IPC_RMID, 0) )
            return RET_FAIL;

        if( _semctl(mutex, IPC_RMID, 0) )
            return RET_FAIL;
    }

    /* NULLize the struct */
    shm_segment = NULL;

    /* Everything went OK */
    return RET_SUCCESS;
}

int buf_put(char c) {
    if ( !IS_MEM_OK(shm_segment) ){
        log_err("Buffer not initialized.");
        return RET_FAIL;
    }
    
    /* Decreasing free_sem spots on shm */
    if( _semdown(free_sem) == RET_FAIL )
        return RET_FAIL;

       
    int nextPos = (shm_segment->in + 1) % shm_segment->size; 
    debug("Trying to put char [%c] in %d, in: %d, out: %d", c,
            shm_segment->in,
            shm_segment->in,
            shm_segment->out
        );
    
    /* Lock the shared memory */
    if( _semdown(mutex) == RET_FAIL)
        return RET_FAIL;

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
    if( _semup(mutex) == RET_FAIL )
        return RET_FAIL;
    
    /* Increase the full_sem spots on shm */
    if( _semup(full_sem) == RET_FAIL )
        return RET_FAIL;
 
    return RET_SUCCESS;
}

int buf_get(char *c){ 
    if ( !IS_MEM_OK(shm_segment) ){
        log_err("Buffer not initialized.");
        return RET_FAIL;
    }
    debug("Trying to get char from buffer, in: %d, out: %d",
            shm_segment->in,
            shm_segment->out
        );
    
    /* Decrease the full_sem spots on shm */
    //debug("full = %d\n", _semprint(full_sem));
    if( _semdown(full_sem) == RET_FAIL )
        return RET_FAIL;
    
       
    /* Lock the shared memory */
    //debug("mutex = %d\n", _semprint(mutex));
    if( _semdown(mutex) == RET_FAIL)
        return RET_FAIL;

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
    if( _semup(mutex) == RET_FAIL )
        return RET_FAIL;
    
    /* Increase the free_sem spots on shm */
    //debug("free = %d\n", _semprint(free_sem));
    if( _semup(free_sem) == RET_FAIL )
        return RET_FAIL;
 
    return RET_SUCCESS;
}

/* Wrapper functions for semaphores */
int _semget(key_t key) {
    int semid = semget(key, 1, IPC_CREAT | IPC_EXCL | S_IRWXU);
    if( semid < 0 ) {
        if( errno == EEXIST) {
            log_info("Semaphone exists!");

            semid = semget(key, 1, 0);
            debug("%d\n", semid);
            if( semid < 0 ) {
                log_err("semget");
                return RET_FAIL;
            }
        }
        else {
            log_err("semget");

            int ret = shmdt(shm_segment);
            if(ret < 0) {
                log_err("shmdt");
            }

            return RET_FAIL;
        }
    }
    return semid;
}

int _semctl(int semid, int cmd, int arg) {
    int ret = semctl(semid, 0, cmd, arg);
    if( ret == -1 ) {
        log_err("semctl");
    }
    return ret;
}

int _semup(int semid) {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;
    
    debug("Increasing semaphore [%d] by 1\n",
            semid);

    if( semop(semid, &op, 1) == RET_FAIL )
        return RET_FAIL;

    return RET_SUCCESS;
}

int _semdown(int semid) {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;
    
    debug("Decreasing sempaphore [%d] by 1\n",
            semid);

    if( semop(semid, &op, 1) == RET_FAIL )
        return RET_FAIL;

    return RET_SUCCESS;
}

int _semprint(int semid) {
    return _semctl(semid, GETVAL, 0);
}

