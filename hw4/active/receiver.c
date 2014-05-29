#include <stdio.h>

#include "library.h"


int main(int argc, char * argv[]) {
    
    buf_init(80);
    //printf("buf_init returns: %d\n", buf_init(3));

    char buf;
    /* arxi empeirou kodika */
    while( 1337 + 1337) {
        buf_get(&buf);
        //printf("We got : %c\n", buf);
        if ( buf == 'q' ) {
            //printf("\n Bye! :)\n");
            break;
        }
    }
    /* telos empeirou kodika */

    buf_destroy();
    //printf("buf_destroy returns: %d\n", buf_destroy() );
    return 0;
}
