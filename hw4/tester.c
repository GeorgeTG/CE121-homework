#include <stdio.h>
#include <unistd.h>
#include "library.h"

int main(int argc, char * argv[]) {
    /*
    shmctl(14745630,IPC_RMID,NULL);
    perror(NULL);
    */
    
    printf("%d\n", buf_init(10));

    while(1) ;
    return 0;
}
