/*
 * * * * * * * * * * * * * *
 *  Giorgos Gkougkoudis    *
 *  Kostantinos Kanellis   *
 *  hw3-CE121, May 2014    *
 * * * * * * * * * * * * * *
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>

#include "list.h"

#define COMMAND_MAX_LEN 101
#define ARGUMENT_MAX_NUMBER 101
#define ARG_MAX_LEN (4096 + 1)

#define STAT_NORMAL 0
#define STAT_STOP 1
#define STAT_CONT 2
#define STAT_DEAD 3

#define USLEEP_TIME 500
#define TRIES_MAX 4000

#define COMMAND_IS(y) (strcmp(command, y) == 0)

void _exec();
void _kill();
void _stop();
void _cont();
void _list();
void _info();

void send_signal(pid_t pid, int sig);
int parse_args(char ***argvect);

char *status_to_str(int status);
void refresh_status(process_t *proc);
void refresh_all();
int wait_status(process_t *proc, int status);

/* Head of the list */
process_t * l_head;

int main(int argc, char *argv[])
{
    char command[COMMAND_MAX_LEN];

    l_init(&l_head);

    while( 1 ) {
        printf("\n>> ");
        scanf("%s", command);

        /* Check for status changes (external) */
        refresh_all();

        if( COMMAND_IS( "exec" ) ) {
            _exec();
        }
        else if( COMMAND_IS ( "list" ) ) {
            _list();
        }
        else if( COMMAND_IS( "kill") ) {
            _kill();
        }
        else if( COMMAND_IS( "stop") ) {
            _stop();
        }
        else if( COMMAND_IS( "cont") ) {
            _cont();
        }
        else if( COMMAND_IS( "info") ) {
            _info();
        }
        else if ( COMMAND_IS( "exit") ) {
           break;
        }
        else{
            fprintf(stderr, "\n**Unknown command: \"%s\"!!!\n", command);
        }
    }

    printf("\t**Killing all processes...\n");
    /* Kill all process */
    process_t *cur_node;
    for( cur_node = l_head->next; cur_node != l_head; cur_node = cur_node->next ) {
        if ( cur_node->status == STAT_STOP ){
            send_signal(cur_node->pid, SIGCONT);
        }

        send_signal(cur_node->pid, SIGTERM);

        int w_ret = wait_status(cur_node, STAT_DEAD);
        if (w_ret < 0) {
            send_signal(cur_node->pid, SIGKILL);
        }
    }

    l_destroy(&l_head);
    printf("Bye!\n");

    return 0;
}

void send_signal(pid_t pid, int sig){
    int ret;

    ret = kill(pid, sig);
    if( ret < 0 && errno != ESRCH) {
        perror( "kill" );
        exit( EXIT_FAILURE );
    }
}

void refresh_all() {
    process_t *c_node, *del_node;

    for(c_node = l_head->next; c_node != l_head; c_node=c_node->next) {
        refresh_status(c_node);

        if ( c_node->status == STAT_DEAD ){
            del_node = c_node;
            c_node = c_node->next;
            l_del_node( del_node );
        }
    }
}

char *status_to_str(int status){
    switch (status){
        case STAT_NORMAL:
            return "Normal";
        case STAT_STOP:
            return "Stopped";
        case STAT_CONT:
            return "Continued";
        case STAT_DEAD:
            return "Dead";
    }
    /* Normally this will never be reached */
    return "Unknown";
}

void refresh_status(process_t *proc) {
    /* Refreshes a process' status */
    int status, ret;

    ret = waitpid(proc->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
    if ( ret < 0 ) {
        if (errno == ECHILD){
            proc->status = STAT_DEAD;
        }/*
        else {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }*/
    }
    else if ( ret > 0 ){
        /* if ret==0 waitpid retrun because of W options but status is
         * unchanged! */
        if( WIFEXITED(status) || WIFSIGNALED(status) ){
            proc->status = STAT_DEAD;
        }
        else if( WIFSTOPPED(status) ){
            proc->status =  STAT_STOP;
        }
        else if( WIFCONTINUED(status) ){
            proc->status = STAT_CONT;
        }
        else {
            proc->status = STAT_NORMAL;
        }
    }
}

int wait_status(process_t *proc, int status){
    /* Waits for a process to be in a desired state */
    long i;

    for(i=0; i < TRIES_MAX; i++){
        refresh_status( proc );

        if( proc->status == status){
            return 0;
        }

        /* Wait for a small interval */
        usleep(USLEEP_TIME);
    }

    /* Timed out */
    return -1;
}

void _list() {
    process_t *cur_node = l_head->next;

    printf("\n* Running processes:");

    if ( cur_node != l_head ) {
        /* List is empty */
        for (; cur_node != l_head; cur_node = cur_node->next){
            refresh_status( cur_node );

            printf("\n\t* Progname: %s\n\t  Pid: %d\n\t  Status: %s\n",
                    cur_node->progname,
                    cur_node->pid,
                    status_to_str( cur_node->status )
                  );
        }
    }
    else {
        printf( "\n\tNone.\n" );
    }
}

void _stop() {
    pid_t pid;
    process_t *s_node;
    int w_ret;

    /* Get pid from user and search for process in list */
    scanf(" %d", &pid);
    s_node = l_search(l_head, pid);

    if (s_node == NULL){
        fprintf(stderr, "\n **Pid: [%d] not found!\n", pid);
        return;
    }

    refresh_status( s_node );
    if ( s_node->status == STAT_STOP ){
        fprintf(stderr, "\n*P:[%d] Already stopped!\n", pid);
        return;
    }

    send_signal(s_node->pid, SIGSTOP);

    w_ret = wait_status(s_node, STAT_STOP);
    if( w_ret < 0 ) {
        /* Waiting for status change timed out */
        fprintf(stderr, "\n **P:[%d] Process not responding to signal!\n", pid);
    }
}

void _cont() {
    pid_t pid;
    process_t *s_node;
    int w_ret;

    /* Get pid from user and search for process in list */
    scanf(" %d", &pid);
    s_node = l_search(l_head, pid);

    if (s_node == NULL){
        fprintf(stderr, "\n **Pid: [%d] not found!\n", pid);
        return;
    }

    refresh_status( s_node );

    if ( s_node->status != STAT_STOP ){
        fprintf(stderr, "\n*P:[%d] Is not stopped!\n", pid);
        return;
    }

    send_signal(s_node->pid, SIGCONT);

    w_ret = wait_status(s_node, STAT_CONT);
    if( w_ret < 0 ) {
        /* Waiting for status change timed out */
        fprintf(stderr, "\n **P:[%d] Process not responding to signal!\n", pid);
    }
}

void _kill() {
    pid_t pid;
    process_t *s_node;
    int w_ret;

    /* Get pid from user and search for process in list */
    scanf(" %d", &pid);
    s_node = l_search(l_head, pid);

    if (s_node == NULL){
        fprintf(stderr, "\n **Pid: [%d] not found!\n", pid);
        return;
    }

    refresh_status( s_node );

    if ( s_node->status == STAT_STOP ){
        /* Process is stopped, continue and then send SIGTERM */
        send_signal(s_node->pid, SIGCONT);
    }

    send_signal(s_node->pid, SIGTERM);

    w_ret = wait_status(s_node, STAT_DEAD);
    if (w_ret < 0){
        /* Failed to kill gracefully. Force */
        fprintf(stderr, "\n**P[%d]: Failed to exit gracefully. Will now force exit!\n", pid);
        send_signal(pid, SIGKILL);
    }

    /* Remove dead process from the list */
    l_del_node( s_node );
}

void _exec() {
    char *progname, **argv;
    int p_ret;

    /* Parse given arguments */
    p_ret = parse_args(&argv);
    if ( p_ret < 0 ) {
        fprintf(stderr, "/n *Failed to parse arguments due to memory error!\n");
        return;
    }

    /* Add progname */
    progname = malloc(strlen(argv[0]));
    if ( progname == NULL ){
        exit(EXIT_FAILURE);
    }
    strcpy(progname, argv[0]);

    /* Initialize struct */
    process_t process;
    process.progname = progname;
    process.status = STAT_NORMAL;
    process.args = argv;
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

    /* Print confirmation message */
    printf("\n\t* Successfully started \"%s\" with pid:[%d]\n",
            process.progname,
            process.pid
        );
}

void _info() {
    pid_t pid;
    process_t *s_node;

    /* Get pid from user and search for process in list */
    scanf(" %d", &pid);
    s_node = l_search(l_head, pid);

    if (s_node == NULL){
        fprintf(stderr, "\n **Pid: [%d] not found!\n", pid);
        return;
    }

    refresh_status( s_node );

    send_signal(s_node->pid, SIGUSR1);
}

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
    /* Skip initial whitespace/s */
    while( buf == ' '){
        buf = getchar();
    }
    /* No arguments given special case */
    if( buf == '\n' ){
        argv[0] = NULL;
        *argvect = argv;
        return 0;
    }
    while ( 1 ){
        argv[argc] = malloc(ARG_MAX_LEN * sizeof(char) );
        /* getchar While the char we got is valid */
        for(i=0; buf != ' ' && buf != '\n'; i++){
            argv[argc][i] = buf;
            buf=getchar();
        }

        /* Skip whitespace between args */
        while( buf == ' '){
            buf = getchar();
        }

        /* Realloc str(trim), and null terminate */
        argv[argc] = realloc(argv[argc], i + 1);
        argv[argc][i] = '\0';

        /* Increment arg count, realloc arguments' vector size*/
        argc ++;
        argv = realloc(argv, sizeof(char*) * (argc + 1) );

        /* end of buffer */
        if(buf=='\n'){
            argv[argc] = NULL;
            break;
        }

    }

    *argvect = argv;
    return argc + 1;
}
