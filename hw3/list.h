#ifndef LIST_H

#define LIST_H

typedef struct process {
    pid_t pid;
    int status;
    char *progname;
    char **args;
    struct process *next;
    struct process *prev;
} process_t;

void l_init(process_t **HEAD);
void l_add(process_t *HEAD, process_t data_node);
process_t *l_search(process_t *HEAD, pid_t pid);
void l_print_node(process_t *node);
void l_print(process_t *HEAD);
int l_del_node(process_t *d_node);
int l_delete(process_t *HEAD, pid_t pid);
void l_destroy(process_t **HEAD);

#define LIST_FAIL -1
#define LIST_SUCCESS 0

#define TERMINAL_PID -1337

#endif
