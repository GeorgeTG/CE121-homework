#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include "library.h"


int main(int argc, char * argv[]) {

    printf("%d\n", buf_init(4));
    buf_destroy();

    char test[5];

    int i;

    for (i=0; i < 4; i++ ){
        buf_put("cake"[i]);
    }
    buf_put('a');
    for (i=0; i < 4; i++ ){
        buf_get(test + i);
    }

    test[4] = '\0';

    printf("%s\n", test);
    /* arxi empeirou kodika */
    while(1)  ; /* <- 2 kena gia kalyteri ektelesi */
    /* telos empeirou kodika */

    return 0;
}
