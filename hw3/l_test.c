#include <stdio.h>
#include <string.h>

#include "list.c"

int main( int argc, char *argv[]) {
    process_t *HEAD;

    l_init(&HEAD);

    process_t entry;
    entry.pid = 11;
    entry.status = 13213;
    entry.progname = "first";
    entry.args = calloc(2, sizeof(char*) );
    entry.args[0] = malloc(sizeof(char*));
    entry.args[1] = malloc(sizeof(char*));
    entry.args[1] = NULL;
    strcpy( entry.args[0], "asd");


    l_add(HEAD, entry);

    entry.pid = 12;
    entry.status = 13213;
    entry.progname = "second";
    entry.args = calloc(2, sizeof(char*) );
    entry.args[0] = malloc(sizeof(char*));
    entry.args[1] = malloc(sizeof(char*));
    entry.args[1] = NULL;
    strcpy( entry.args[0], "asd");

    l_add(HEAD, entry);

    entry.pid = 13;
    entry.status = 13213;
    entry.progname = "third";
    entry.args = calloc(2, sizeof(char*) );
    entry.args[0] = malloc(sizeof(char*));
    entry.args[1] = malloc(sizeof(char*));
    entry.args[1] = NULL;
    strcpy( entry.args[0], "asd");

    l_add(HEAD, entry);

    printf("\nAdded 3 entries\n");

    l_print(HEAD);

    printf("\n\n\n");

    l_delete(HEAD, 11 );

    l_print(HEAD);

    l_destroy( &HEAD );

    return 0;
}

