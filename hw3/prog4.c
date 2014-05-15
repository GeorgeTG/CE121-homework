#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t report;

static void handler(int sig) {
    report = 1;
}

int main(int argc, char * argv[]) {

    struct sigaction act = { {0} };
    sigset_t sa_mask = { {0} };

    report = 0;

    /* Block USR1 while we are in handler execution */
    sigaddset(&sa_mask, SIGUSR1);
    act.sa_mask = sa_mask;

    act.sa_flags |= SA_RESTART;
    act.sa_handler = handler;

    int ret = sigaction(SIGUSR1, &act, NULL);
    if( ret < 0 ) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    int i;
    pid_t pid = getpid();
    for(i = 0; i < 100; i++ ) {
        if (report == 1 ){
            printf("\n*P[%d] says: We love pizza!\nWe ate %d%% of pizza!\n", pid, i);
            report = 0;
        }

        sleep(1);
        if(i==100) {
            i = 0;
        }
    }

    return 0;
}
