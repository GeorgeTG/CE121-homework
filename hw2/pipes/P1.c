#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include "util.h"

#define EXPR_MAX_LEN 2*80
#define TERMINATE_CHR 'q'
/* Functions that are used as operators */
int add(int x, int y){
    return (x + y);
}
int substract(int x, int y){
    return (x - y);
}

/***************************************/

int main(int argc, char * argv[])
{
    /* Operator stuff */
    char * valid_operators[] = { "+", "-" };
    operation_f operate[]  = { add, substract };
    /* Expression strings */
    char expr[EXPR_MAX_LEN], parsed[EXPR_MAX_LEN];

    do {

        ReadLine(expr);

        /* Check if we should terminate */
        if( expr[0] != TERMINATE_CHR ) {
            ParseExpression(expr, parsed, valid_operators, operate, 2);
        }
        else{
            strcpy(parsed, expr);
        }
        
        fprintf(stderr, "[1] PARSED: %s\n", parsed);

        /* Write the result to output file */
        printf("%s\n", parsed);
        fflush(stdout);

    } while( expr[0] != TERMINATE_CHR  );

    return 0;
}
