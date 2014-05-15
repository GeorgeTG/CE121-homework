#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/wait.h>
#include "util.h"

#define EXPR_MAX_LEN 82
#define ESCAPE_CHR 'q'


int GetExpresssion(char *str, size_t size);
void AddSpaces(char *oldExpr, char *newExpr);

int main(int argc, char *argv[]){
    int fd[4][2]; /* pipes array */
    char *child[] = { "./P1", "./P2", "./P3" }; /* child binaries */
    pid_t pid[3]; /* children pids*/

    char input[EXPR_MAX_LEN * 2], expr[EXPR_MAX_LEN], output[EXPR_MAX_LEN];

    int i, j, code;

    /* Initialize pipes */
    for(i = 0; i < 4; i++) {
        code = pipe(fd[i]);

        if( code < 0 ) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

/****************************Spawn child processes****************************/

    for(i = 0; i < 3; i++) {
        /* Fork the proccess */
        pid[i] = fork();

        if( pid[i] == 0 ) {
            /* Child code */
            /*rPipe: Read Pipe, wPipe: Write pipe*/
            int rPipe = i, wPipe = i+1;

            /* Close the unsused pipes */
            for(j = 0; j < 4; j++) {
                if( j != rPipe )
                    Close( fd[j][0] );
                if( j != wPipe )
                    Close( fd[j][1] );
            }

            /* Redirect stdin & stdout */
            Redirect( fd[rPipe][0], 0);
            Redirect( fd[wPipe][1], 1);

            /* Execute the child code */
            execlp( child[i] , child[i], NULL);

            /* If we get here we got an eror */
            perror("execlp");
            exit(EXIT_FAILURE);
        }
        else if ( pid[i] < 0 ) {
            /* Kill the previous children */
            for(j = 0; j < i; j++) {
                kill(pid[j], SIGKILL);
            }

            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

    }

    /* Close the unused pipes in main */
    for(j = 0; j < 4; j++) {
        if( j != 3 )
            Close(fd[j][0]);
        if( j != 0 )
            Close(fd[j][1]);
    }

    /* Initializing pollfd for poll-ing */
    struct pollfd fdinfo[2];

    fdinfo[0].fd = fd[3][0];
    fdinfo[0].events = POLLIN;
    fdinfo[1].fd = fd[0][1];
    fdinfo[1].events = POLLOUT;

/********************************************************************
 * ***** ****** MAIN LOOP *******************************************
 *******************************************************************/
    while ( 1 ) {
        printf( "Enter an expression up to %d characters long: ",
                EXPR_MAX_LEN - 2);

        while ( GetExpresssion(expr, EXPR_MAX_LEN) == 1 ){
            printf( "Warning: Maximum expression length is %d!\n",
                    EXPR_MAX_LEN );

            printf("Please re-try: ");
        }
        
        /* Add spaces when needed */
        if( expr[0] != ESCAPE_CHR )
            AddSpaces(expr, input);
        else
            strcpy(input, expr);
       
        /* Write to pipe */
        WaitForPipe( &fdinfo[1] );
        Write(fdinfo[1].fd, input, strlen(input));
        
        /* Read from pipe */
        WaitForPipe( &fdinfo[0] );
        Read(fdinfo[0].fd, output);

        if (expr[0] != ESCAPE_CHR ) {
            printf("Result: %s\n", output);
        }
        else break;
    }

    /* Clean up */
    int childStatus;
    for(i = 0 ; i < 3; i++){
        waitpid(pid[i], &childStatus, 0);
    }

    /* Close the pipes */
    Close(fdinfo[0].fd);
    Close(fdinfo[1].fd);

    return 0;
}
/*****************************************************************************/

int GetExpresssion(char *str, size_t size){
    /* gets the expression from stdin. If more than size chars are entered
     * then they are thrown away to clear stdin. The string is
     * null-terminated*/

    int i = 0;
    if ( (str[i] = getchar() ) != '\n' ) {
        i++; /* if this is not a linefeed go on, otherwise replace */
    }

    for (; i <= size; i++){
        str[i] = getchar();

        if (str[i] == '\n'){
            /* we got line feed(end of input) */
            str[i + 1] = '\0';
            return 0;
        }
    }

    /*if we got here we have exceeded maximum size */
    while ( (str[i] = getchar() ) != EOF && (str[i] != '\n') ) {
                /*dump the rest of the stdin*/
                }
    str[  i  ] = '\n';
    str[i + 1] = '\0';

    return 1;
}

void AddSpaces(char * expr, char * nExpr) {
    int i, p, j, len = (int)strlen(expr);
    j = 0,p = 0;
    
    char charType = 0;
    for(i = 0; i < len; i++) {
        if ( (charType ^ (isdigit(expr[i]) != 0)) == 0 ) { 
            /* Add the interval */
            for( ; p < i; p++)
                nExpr[j++] = expr[p];

            nExpr[j++] = ' ';
            charType ^= 1;
        }
    }
    /* Add the last interval */
    for( ; p < i; p++)
        nExpr[j++] = expr[p];
    
    /* Trim the nExpr */
    while( nExpr[j-1] == ' ' ) j--;
    nExpr[j] = '\0';
}
