#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static void handler(int sig) {
    write(1, "Got SIGTERM! Ignore it!\n", 31);
}

int main(int argc, char * argv[]) {

    struct sigaction act = { {0} };
    act.sa_handler = handler;

    int ret = sigaction(SIGTERM, &act, NULL);
    if( ret < 0 ) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    while( 1 ) {
        sleep(1);
    }

    return 0;
}
