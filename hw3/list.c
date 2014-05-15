#include <stdio.h>
#include <stdlib.h>

#include "list.h"

void l_init(process_t **HEAD) {
    *HEAD = (process_t *)malloc(sizeof(**HEAD));
    if (*HEAD == NULL){
        fprintf( stderr, "Memory allocation failure!\n");
        exit( LIST_FAIL );
    }

    (*HEAD)->next = *HEAD;
    (*HEAD)->prev = *HEAD;
    (*HEAD)->pid = TERMINAL_PID;
}

void l_add(process_t *HEAD, process_t data_node){
    process_t *new_node;
    new_node = (process_t *)malloc( sizeof(process_t) );
    if (new_node == NULL){
        fprintf( stderr, "Memory allocation failure!\n");
        exit( LIST_FAIL );
    }

    *new_node = data_node;

    new_node->next = HEAD->next;
    new_node->prev = HEAD;
    new_node->next->prev = new_node;
    new_node->prev->next = new_node;

}

process_t *l_search(process_t *HEAD, pid_t pid){
    process_t *cur_node = HEAD->next;

    for(; (cur_node->pid) != TERMINAL_PID; cur_node = cur_node->next) {
        if (cur_node->pid == pid){
            return cur_node;
        }
    }

    return NULL;
}

void l_print_node(process_t *node){
    int i;

    printf("pid: %d\nstatus: %d\nname: %s\nargs:\n",
        node->pid,
        node->status,
        node->progname
      );
     for (i=0; (node->args)[i] != NULL; i++ ){
            printf("\t%s\n", (node -> args)[i] );
     }
}

void l_print(process_t *HEAD){
    process_t *cur_node = HEAD->next;

    for(; (cur_node->pid) != TERMINAL_PID; cur_node = cur_node->next) {
        l_print_node(cur_node);
    }
}

void free_node(process_t *node){
    int i;

    /* free args */
    for (i=0; (node->args)[i] != NULL; i++ ){
        free( (node->args)[i] );
    }

    /* free node */
    free( node );
}

int l_del_node(process_t *d_node){
    if( d_node == NULL){
        return -1;
    }

    d_node->next->prev = d_node->prev;
    d_node->prev->next = d_node->next;

    free( d_node );
    d_node = NULL;

    return 0;
}

int l_delete(process_t *HEAD, pid_t pid){
    process_t *s_node = l_search(HEAD, pid);

    return l_del_node(s_node);
}

void l_destroy(process_t **HEAD){
    process_t *cur_node = (*HEAD)->next;

    for(; (cur_node->pid) != TERMINAL_PID; cur_node = cur_node -> next) {
        l_del_node(cur_node);
    }

    free ( *HEAD );
    *HEAD = NULL;
}

