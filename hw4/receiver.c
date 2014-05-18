#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include "library.h"


int main(int argc, char * argv[]) {

    printf("%d\n", buf_init(7));

    char test[5];

    /* arxi empeirou kodika */
    int i;
    while( 1337 + 1337) {
        for (i=0; i < 4; i++ ){
            buf_get(test + i);
        }
        test[4] = '\0';

        printf("%s\n", test);
    }
    /* telos empeirou kodika */

    return 0;
}
