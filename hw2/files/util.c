#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include "util.h"

#define OPERATOR_MAX_LEN 4
#define EOM '\n'

#define SLEEP_TIME (useconds_t)400000 /* 0.4 sec*/


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

int FileSeek(int fd, int offset, int whence){
    int code = lseek(fd, ((off_t)offset), whence);
    if( code == -1 ){
        perror("lseek");
        exit(EXIT_FAILURE);
    }
    return code;
}

char FileEmpty(int fd) {
    return (FileSeek(fd, 0, SEEK_END) == 0);
}

void TruncateFile(int fd) {
    int ret = ftruncate(fd, 0);
    if(ret == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
}

char ReadChar(int fd){
    ssize_t bytesRead;
    char c;

    do {
        bytesRead = read(fd, &c, 1);
        if (bytesRead == -1){
            perror("read");
            exit(EXIT_FAILURE);
        }

     } while(bytesRead == 0);

    return c;
}

void ReadLine(int fd, char *line) {
    FileSeek(fd, 0, SEEK_SET);
    int position = 0;

    do {
        line[position] = ReadChar(fd);

        if( line[position] == '\n' ){
            line[position + 1] = '\0';
            break;
        }
        position++;
    } while ( 1 );
}

void sWrite(int fd, void *buf, size_t count){
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

void WriteLine(int fd, char *line, int count){
    FileSeek(fd, 0, SEEK_SET);
    sWrite(fd, line, count);
}

void SendMessage(int fd, char *message){
    while ( ! FileEmpty(fd) ){
        /*wait until file is empty */
       usleep (SLEEP_TIME);
    }

    /*file is empty we can write */
    int len = strlen(message);

    message[len]     = '\n';
    message[len + 1] = '\0';
    WriteLine(fd, message, len + 1);
}
void ReceiveMessage(int fd, char *bf){
    while ( FileEmpty(fd) ){
        /* Wait for the file not to be empty */
        usleep (SLEEP_TIME);
     }
    /* Read the file from the beginning until read string contains
     * EOM (end of message */
    char *lineFeed;
    do {
        FileSeek(fd, 0, SEEK_SET);
        ReadLine(fd, bf);
    } while ( (lineFeed = strchr(bf, EOM)) == NULL);

    /* Remove the line feed */
    *lineFeed = '\0';

    /* Empty the file */
    TruncateFile(fd);
}

void Close(int fd){
    int code = close(fd);
    if( code < 0 ){
        perror("close");
        exit(EXIT_FAILURE);
    }
}

