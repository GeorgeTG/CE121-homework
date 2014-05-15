#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define ARG_MAX_LEN 15

int parse_args(char ***argvect){
    int i, argc=0;
    char **argv, buf;

    /* init arguments vector */
    argv = malloc( sizeof(char*) );
    if (argv == NULL){
        return -1;
    }

    /* argv[0] should be the program's name */

    buf = getchar();
    /* skip initial whitespace */
    while( buf == ' '){
        buf = getchar();
    }
    /* no arguments given special case */
    if(buf=='\n'){
        argv[0] = NULL;
        *argvect = argv;
        return -1;
    }
    while ( 1 ){
        argv[argc] = malloc(ARG_MAX_LEN * sizeof(char) );
        /* getchar while the char we got is valid */
        for(i=0; buf != ' ' && buf != '\n'; i++){
            argv[argc][i] = buf;
            buf=getchar();
        }

        /* skip whitespace between args */
        while( buf == ' '){
            buf = getchar();
        }

        /* realloc str, and null terminate */
        argv[argc] = realloc(argv[argc], i + 1);
        argv[argc][i] = '\0';

        /* increment arg count, realloc arguments' vector size*/
        argc ++;
        argv = realloc(argv, sizeof(char*) * (argc + 1) );

        /* end of buffer */
        if(buf=='\n'){
            argv[argc] = NULL;
            break;
        }

    }
    *argvect = argv;
    return argc;
}

int main(int argc, char *argv[]){
    int i;
    char **argvect;

    parse_args(&argvect);

    for(i=0; argvect[i] != NULL; i++){
        printf("arg%d: %s\n", i, argvect[i]);
    }
    return 0;
}

