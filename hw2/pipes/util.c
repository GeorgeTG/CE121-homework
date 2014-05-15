#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

#define OPERATOR_MAX_LEN 4
#define EOM '\n'

#define POLL_TIMEOUT 2000

void ParseExpression(char *expr,
        char *parsed,
        char* operators[],
        operation_f do_operation[],
        int count )
{
    int valuesRead, charsRead, charsWritten;
    int exprPosition = 0, parsedPosition = 0;

    char validOperator;
    int i, a, b;
    char operator[OPERATOR_MAX_LEN];
    
    /* Read the first number */
    sscanf(expr, "%d %n", &a, &charsRead);
    exprPosition += charsRead;

    while( 1 ) {
        /* Read operator & number until there is no more */
        valuesRead = sscanf(expr + exprPosition,
                "%s %d %n",
                operator,
                &b,
                &charsRead );
        exprPosition += charsRead;
        
        if( valuesRead < 2 ) { 
            charsWritten = sprintf(parsed + parsedPosition, "%d", a);
            parsedPosition += charsWritten;
            break;
        }
        
        /* Check if an operation must be performed */
        validOperator = 0;
        for(i = 0; i < count; i++){
            if( strcmp(operator, operators[i]) == 0 ){
                a = do_operation[i](a, b);
                validOperator = 1;
                break;
            }
        }

        /* If no operation has to be performed, append the number & operator */
        if( !validOperator ) {
            charsWritten = sprintf(parsed + parsedPosition,
                    "%d %s ",
                    a,
                    operator );
            parsedPosition += charsWritten;

            a = b;
        }
    }

}

void Write(int fd, char *buf, size_t count){
    ssize_t bytesWritten;
    ssize_t bytesLeft = count;

    do {
        bytesWritten = write(fd, buf, bytesLeft);
        if( bytesWritten == -1 ){
            perror("write");
            exit(EXIT_FAILURE);
        }
        else{
            bytesLeft -= bytesWritten;
            buf += bytesWritten;
        }
    } while( bytesLeft > 0 );

}


void Read(int fd, char *buf) {
    ssize_t bytesRead;

    do {
        bytesRead = read(fd, buf, 1);
        
        if( bytesRead == -1 ) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        else if( *buf == '\n' ){
            *buf = '\0';
            break;
        }
        else {
            buf++;
        }
    }
    while( 1 );

}

void ReadLine(char *buf) {
    int i = 0;

    do {
        scanf("%c", &buf[i]);
        i++;
    } while( buf[i-1] != '\n' );
    
    buf[i] = '\0';
}

void Redirect(int oldfd, int newfd) {
    int code = dup2(oldfd, newfd);
    if( code < 0 ) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
}

void Close(int fd){
    int code = close(fd);
    if( code < 0 ){
        perror("close");
        exit(EXIT_FAILURE);
    }
}

