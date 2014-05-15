#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "util.h"

#define EXPR_MAX_LEN 2*82
#define ESCAPE_CHR 'q'

int GetExpresssion(char *str, size_t size);
void AddSpaces(char *oldExpr, char *newExpr);

int main(int argc, char *argv[]){

    int commFiles[4]; /* fds for our communication files */
    char filenames[4][10]; /* commincation files filenames */
    char *child[] = {"./P1", "./P2", "./P3" };/* children's filenames vector */

    pid_t pid[3]; /* forked pids */

    char input[EXPR_MAX_LEN * 2], expr[EXPR_MAX_LEN], output[EXPR_MAX_LEN];

    int i,j;

/*******************************Create files**********************************/
    for (i = 0; i < 4; i++){
        sprintf( filenames[i], ".com%d", i);
        commFiles[i] = open( filenames[i], O_CREAT | O_RDWR, S_IRWXU);

        /* error checking */
        if (commFiles[i] < 0) {
            fprintf( stderr, "Cannot create file %s!\n", filenames[i]);
            perror(NULL);

            /*close previous files!*/
            for (j = 0; j < i; j++){
                close( commFiles[j] );
            }

            exit(EXIT_FAILURE);
        }
    }

/**********************Create the 'children' proccesses***********************/
    for(i = 0; i < 3; i++){
        pid[i] = fork();

        if( pid[i] == 0 ) {
            execlp(child[i],
                    child[i],
                    filenames[i],
                    filenames[ (i+1) % 4 ],
                    (char *)NULL );

            /* This code is excecuted only if execlp fails! */
            perror("Exec failed");
            exit(EXIT_FAILURE);
        }
        else if( pid[i] < 0 ) {
            /* We kill the previous children */
            for(j = 0; j < i; j++) {
                kill(pid[j], SIGKILL);
            }

            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

/*******************************Main loop*************************************/
    while ( 1 ) {
        printf( "Enter an expression up to %d characters long: ",
                EXPR_MAX_LEN  / 2 - 2);

        while ( GetExpresssion(expr, EXPR_MAX_LEN / 2 ) == 1 ){
            printf( "Warning: Maximum expression length is %d!\n",
                    EXPR_MAX_LEN );

            printf("Please re-try: ");
        }
        
        /* Add spaces when needed */
        if( expr[0] != ESCAPE_CHR )
            AddSpaces(expr, input);
        else
            strcpy(input, expr);

        SendMessage(commFiles[0], input);
        ReceiveMessage(commFiles[3], output);

        if (expr[0] != ESCAPE_CHR ) {
            printf("Result: %s\n", output);
        }
        else break;

    }

/*******************************Clean Up**************************************/

    /* Wait for children to quit */
    int childStatus;
    for(i = 0; i < 3; i++){
        waitpid(pid[i], &childStatus, 0);
    }

    /* Close files */
    for(i = 0; i < 4; i++) {
        Close( commFiles[i] );
        if ( remove( filenames[i] ) < 0 ){
            perror("Delete file");
        }
    }

    printf("Quitting...Bye!\n");

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
    while( isspace(nExpr[j-1]) ) j--;
    nExpr[j] = '\n';
    nExpr[j+1] = '\0';
}
