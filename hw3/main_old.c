#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>

#include "list.h"

#define COMMAND_MAX_LEN 101
#define ARGUMENT_MAX_NUMBER 101
#define ARGUMENT_MAX_LEN (4096 + 1)

/*typedef struct _process {
    pid_t pid;
    int status;
    char *progname;
    char **args;
    struct _process *next;
    struct _process *prev;
} process_t;*/

void _exec();
void _kill();
void _stop();
void _cont();
void _list();
void _info();

/* Head of the list */
process_t * l_head;

void _exec() {
    char *progname, **args;
    char buffer[ARGUMENT_MAX_NUMBER * ARGUMENT_MAX_LEN];
    char argument[ARGUMENT_MAX_LEN];
    int nArguments, bufferLen;
    
    /* Read the progname */
    scanf("%s", buffer);
    progname = (char *)malloc( strlen(buffer) * sizeof(char) );
    strcpy(progname, buffer);

    printf("%s\n", progname);
    
    /* Read the rest of the arguments */
    int i = 0;
    do {
        buffer[i] = getchar();
        i++;
    } while( buffer[i-1] != '\n' );
    buffer[i] = '\0';
    bufferLen = i;

    /* Count the number of arguments */
    /* Ignore whitespaces in the beggining */
    while( i < bufferLen && isspace(buffer[i]) )
        i++;
    
    nArguments = 0;
    for(i = 0; i < bufferLen; i++) { 
        nArguments++;
        
        /* Pass the argument */
        while( i < bufferLen && isgraph(buffer[i]) )
            i++;

        /* Ignore whitespaces */
        while( i < bufferLen && isspace(buffer[i]) )
            i++;
    }
    //printf("%d\n", nArguments);
    
    /* Allocate space for args array */
    args = (char **)malloc( (nArguments + 2) * sizeof(char *) );
    
    /* First argument is always the filename */
    args[0] = (char *)malloc( strlen(progname) * sizeof(char) );
    strcpy(args[0], progname);
    
    //printf("%s\n", buffer);

    /* Add the rest of the arguments to args array */
    int position = 0, curArgument = 1;
    int charsRead, valuesRead;
    while( position < bufferLen ) {
        valuesRead = sscanf(buffer + position, "%s %n",
                argument, &charsRead);
        position += charsRead;

        //printf("[%d], [%d], '%s'\n", valuesRead, charsRead, argument); 
        
        if( valuesRead < 1 )
            break;
        
        args[curArgument] = (char *)malloc( strlen(argument) * sizeof(char) );
        strcpy( args[curArgument], argument);
        curArgument++;
    }

    /* Last argument must be a NULL pointer */
    args[nArguments + 1] = NULL;
  

    /* Initialize struct */
    process_t process;
    process.progname = progname;
    process.args = args;
    process.next = process.prev = NULL;
    
    /* Run the process */ 
    process.pid = fork();
    if( process.pid < 0 ) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if( process.pid == 0 ) {
        execv( process.progname, process.args );

        perror("execv");
        exit(EXIT_FAILURE);
    }
    
    /* Add the process to list */
    l_add(l_head, process);
}

int main(int argc, char *argv[])
{
    char command[COMMAND_MAX_LEN];
    
    l_init(&l_head);
    while( 1 ) {
        printf(">> ");
        scanf("%s", command);

        if( strcmp(command, "exec") == 0 )
            _exec();
    }
    
    return 0;
}




