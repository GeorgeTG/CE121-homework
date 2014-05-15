#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "util.h"

#define EXPR_MAX_LEN 2*82
#define TERMINATE_CHR 'q'

/* Functions that are used as operators */
int and(int x, int y){
    return (x && y);
}
/***************************************/

int main(int argc, char * argv[])
{
    if( argc < 3 ) {
        printf("ERROR: Not enough arguments\n");
        exit(EXIT_FAILURE);
    }

    /* Handling file descriptors */
    int in_fd, out_fd;

    in_fd = open(argv[1], O_RDWR, S_IRWXU);
    if( in_fd < 0 ) {
        perror("Error while opening file!\n");
        exit(EXIT_FAILURE);
    }

    out_fd = open(argv[2], O_WRONLY, S_IWUSR);
    if( out_fd < 0 ) {
        perror("Error while opening file\n");
        exit(EXIT_FAILURE);
    }
    /* ****************************** */

    /* Operator stuff */
    char * valid_operators[] = { "&&" };
    operation_f operate[] = { and };
    /* Expression strings */
    char expr[EXPR_MAX_LEN], parsed[EXPR_MAX_LEN];

    do {

        ReceiveMessage(in_fd, expr);

        /* Check if we should terminate */
        if( expr[0] != TERMINATE_CHR ) {
            ParseExpression(expr, parsed, valid_operators, operate, 1);
        }
        else{
            strcpy(parsed, expr);
        }

        printf("[3] PARSED: %s\n", parsed);

        /* Write the result to output file */
        SendMessage(out_fd, parsed);

    } while( expr[0] != TERMINATE_CHR  );

    close(in_fd);
    close(out_fd);

    return 0;
}
