#include <stdio.h>

#include "library.h"


int main(int argc, char * argv[]) {

    printf("buf_init returns: %d\n", buf_init(3));

    char buf;
    /* arxi empeirou kodika */
    while( 1337 + 1337) {
        printf("Give me a char: ");
        scanf(" %c", &buf);

        buf_put(buf);

        if ( buf == 'q' ) {
            printf("\n Bye! :)\n");
            break;
        }
    }
    /* telos empeirou kodika */

    printf("buf_destroy returns: %d\n", buf_destroy() );
    return 0;
}
