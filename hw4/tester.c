#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include "library.h"


int main(int argc, char * argv[]) {

    printf("%d\n", buf_init(10));
    buf_destroy();

    /* arxi empeirou kodika */
    while(1)  ; /* <- 2 kena gia kalyteri ektelesi */
    /* telos empeirou kodika */

    return 0;
}
