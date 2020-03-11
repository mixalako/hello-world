#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"

/* Compile-time parameters. */
#define TQ 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */
static void
signals_disable(void);
static void
signals_enable(void);


struct Node{
        int number;
        int pid;
        int prio;
        char *name;
        struct Node *next;
};

struct Queue{
        struct Node *front;
        struct Node *last;
        unsigned int size;
};

void init(struct Queue *q) {
        q->front = NULL;
        q->last = NULL;
        q->size = 0;
}


void push(struct Queue *q, char *name, int number, int pid, int prio){
        q->size ++;
        if(q->front == NULL){
                q->front = (struct Node *) malloc(sizeof(struct Node));
                q->front->number = number;
                q->front->pid = pid;
                q->front->name = name;
                q->front->prio = prio;
                q->front->next = NULL;
                q->last = q->front;
        }
        else{
                q->last->next = (struct Node *) malloc(sizeof(struct Node));
                q->last->next->name = name;
				 q->last->next->number = number;
                q->last->next->pid = pid;
                q->last->next->prio = prio;
                q->last = q->last->next;
        }
}


void pop(struct Queue *q) {
        q->size--;

        struct Node *tmp = q->front;
        q->front = q->front->next;
        free(tmp);
}

int front_pid(struct Queue *q) {
        return q->front->pid;
}
int front_number(struct Queue *q){
        return q->front->number;
}
char *front_name(struct Queue *q){
        return q->front->name;
}
int front_prio(struct Queue *q){
        return q->front->prio;
}
int size(struct Queue *q){
        return q->size;
}
void reduce_size(struct Queue *q){
        q->size--;
}
struct Node* front(struct Queue *q){
        return q->front;
}
//Finds the nearest to the top High Process and Makes it first
void ifHigh(struct Queue *q){
        struct Node *temp = q->front;
        int i;

        if(tmp->prio == 1) return;
        for(i=0; i< q->size - 1; i++){
                if(tmp->next->prio == 1){
                        struct Node *newFirst = tmp->next;
                        tmp->next = tmp->next->next;
                        newFirst->next = q->front;
                        q->front = newFirst;
                }
                else tmp = tmp->next;
        }
}
//Queue, nhigh = #High priority Processes, flag is an index that we have kill from shell process
struct Queue q;
int nhigh = 0;
bool flag=false;
/* Print a list of all tasks currently being scheduled.  */
static void
sched_print_tasks(void)
{
        printf("\n==============================\n");
        printf("You chose Printing Queue with size = %d\n", size(&q));
        struct Node *tmp = front(&q);
        int i;

        for( i=0; i<size(&q) ; i++){
                printf("Task named %s with No %d, PID %d and Priority(HI=1, LO=0) %d \n", tmp->name, tmp->number, tmp->pid, tmp->prio);
                tmp = tmp -> next;
        }
        printf("Success of the shell\n\n");
}

/* Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
static int
sched_kill_task_by_id(int id)
{
        printf("\n******************************\n");
        printf("You chose Killing a child with ID %d\n", id);
        struct Node *tmp = front(&q);
        int i;

        for( i=0; i<size(&q)-1 ; i++){
                if( tmp->next->number == id){
                        printf("second\n");
                        struct Node *del = tmp->next;
                        tmp -> next = tmp -> next ->next;
                        if(del->prio == 1) nhigh--;
                        reduce_size(&q);
                        flag = true;
                        kill( del->pid, SIGKILL );
                        break;
                }
                else tmp = tmp -> next;
        }
        printf("Success of the shell\n\n");
        return 0;
}


/* Create a new task.  */
static void
sched_create_task(char *executable)
{
        printf("\n++++++++++++++++++++++++++++++\n");
        printf("You are a god damn creator my DUDE\n");
        pid_t p;
        int No = size(&q);

        p=fork();
        if(p == 0){
                char *newargv[] = { executable , NULL, NULL, NULL };
                //char *newenviron[] = { NULL };
                raise(SIGSTOP); execve(executable, newargv,NULL);
        }
        else if( p > 0){
                printf("Create child with PID = %d\n", p);
                push(&q, executable, No, p, 0);
        }
        else printf("Problem with Fork");

        printf("Success of the shell\n\n");
}

static void
sched_high_task(int id)
{
        printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printf("Child with id %d changing his Priority to 1 \n", id);
        struct Node *tmp = front(&q);
        int i;

        if(size(&q) == 1 && tmp->number == id && tmp->prio == 0){ tmp->prio = 1; nhigh++; return; }
        for( i=0; i<size(&q)-1 ; i++){
                if( i==0 && tmp -> number == id ){
                        tmp->prio = 1;
                        nhigh ++;
                        break;
                }
                else if( tmp -> next -> number == id ){
                        tmp -> next -> prio = 1;
                        nhigh ++;
                        break;
                }
                else
                        tmp = tmp -> next;
        }
        printf("Success of the shell\n\n");
}
static void
sched_low_task(int id)
{
        printf("\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
        printf("Child with id %d changing his Priority to 0 \n", id);
        struct Node *tmp = front(&q);
        int i;

        if(size(&q) == 1 && tmp->number == id && tmp->prio == 1){ tmp->prio = 0; nhigh--; return; }
        for( i=0; i<size(&q)-1 ; i++){
                if( i==0 && tmp -> number == id ){
                        tmp->prio = 0;
                        nhigh --;
                        if( nhigh < 0 )
                                nhigh = 0 ;
                        break;
                }
                else if( tmp -> next -> number == id ){
                        tmp -> next -> prio = 0;
                        nhigh --;
                        if( nhigh < 0 )
                               nhigh = 0 ;
                                            execve(executable, newargv,NULL);
        }
        else if( p > 0){
                printf("Create child with PID = %d\n", p);
                push(&q, executable, No, p, 0);
        }
        else printf("Problem with Fork");

        printf("Success of the shell\n\n");
}

static void
sched_high_task(int id)
{
        printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printf("Child with id %d changing his Priority to 1 \n", id);
        struct Node *tmp = front(&q);
        int i;

        if(size(&q) == 1 && tmp->number == id && tmp->prio == 0){ tmp->prio = 1; nhigh++; return; }
        for( i=0; i<size(&q)-1 ; i++){
                if( i==0 && tmp -> number == id ){
                        tmp->prio = 1;
                        nhigh ++;
                        break;
                }
                else if( tmp -> next -> number == id ){
                        tmp -> next -> prio = 1;
                        nhigh ++;
                        break;
                }
                else
                        tmp = tmp -> next;
        }
        printf("Success of the shell\n\n");
}
static void
sched_low_task(int id)
{
        printf("\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
        printf("Child with id %d changing his Priority to 0 \n", id);
        struct Node *tmp = front(&q);
        int i;

        if(size(&q) == 1 && tmp->number == id && tmp->prio == 1){ tmp->prio = 0; nhigh--; return; }
        for( i=0; i<size(&q)-1 ; i++){
                if( i==0 && tmp -> number == id ){
                        tmp->prio = 0;
                        nhigh --;
                        if( nhigh < 0 )
                                nhigh = 0 ;
                        break;
                }
                else if( tmp -> next -> number == id ){
                        tmp -> next -> prio = 0;
                        nhigh --;
                        if( nhigh < 0 )
                               nhigh = 0 ;
                                            break;
                }
                else
                        tmp = tmp -> next;
        }
        printf("Success of the shell\n\n");
}

static int
process_request(struct request_struct *rq)
{
        switch (rq->request_no) {
                case REQ_PRINT_TASKS:
                        sched_print_tasks();
                        return 0;

                case REQ_KILL_TASK:
                        return sched_kill_task_by_id(rq->task_arg);

                case REQ_EXEC_TASK:
                        sched_create_task(rq->exec_task_arg);
                        return 0;

                case  REQ_HIGH_TASK:
                        sched_high_task(rq->task_arg);
                        return 0;

                case  REQ_LOW_TASK:
                        sched_low_task(rq->task_arg);
                        return 0;

                default:
                        return -ENOSYS;
        }
}


/*
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum)
{
        if (signum != SIGALRM) {
                fprintf(stderr, "Internal error: Called for signum %d, not SIGALRM\n",
                        signum);
                exit(1);
        }

        printf("Quantum time of %d seconds has expired --> Next Proocedure \n",TQ);

        printf("PID stopped = %d\n", front_pid(&q));
        kill(front_pid(&q), SIGSTOP);
}

/*
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum)
{
        pid_t p;
        int status;
        if (signum != SIGCHLD) {
                fprintf(stderr, "Internal error: Called for signum %d, not SIGCHLD\n",
                        signum);
                exit(1);
        }

        for (;;) {
                p = waitpid(-1, &status, WUNTRACED | WNOHANG);
                if (p < 0) {
                        perror("waitpid");
                        exit(1);
                }
                if (p == 0)
                        break;

                explain_wait_status(p, status);

                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                        /* A child has died */
                        printf("Parent: Received SIGCHLD, child is dead. Exiting.\n");

                        //IF SIGKILL BUT NOT COMING FROM OUR SHELL COMMAND
                        if(!flag && WIFSIGNALED(status)){
                                        struct Node *tmp = front(&q);
                                        int i;
                                        for(i=0; i<size(&q)-1;i++){
                                                if( tmp -> next -> pid == p ){
                                                       struct Node *delete = tmp->next;
                                                       tmp->next=tmp->next->next;
                                                       free(delete);
                                                       return;
                                                }
                                                else tmp = tmp -> next;
                                        }
                        }

                        //IF A HIGH PROCESS EXITS
                        if(front_prio(&q) && WIFEXITED(status)) nhigh--;

                        //IF SIGKILL COMING FROM OUR SHELL(CANT KILL SHELL IN THIS IMPLEMENTATION AS LONG AS THERE IS Q)
                        if(flag){
                                push(&q, front_name(&q), front_number(&q), front_pid(&q), front_prio(&q));
                                flag = false;
                        }
                        pop(&q);
                        if(size(&q) == 0){

                                fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
                                exit(1);
                        }
                        else{
                                if(nhigh>0) ifHigh(&q);
   if (alarm(TQ) < 0) {
                                        perror("alarm");
                                        exit(1);
                                }
                                kill(front_pid(&q), SIGCONT);
                        }
                }
                if (WIFSTOPPED(status)) {
                        /* A child has stopped due to SIGSTOP/SIGTSTP, etc... */
                        if( p != front_pid(&q) ) return; //SIGSTOP COMES FROM CREATING KID
                        printf("Parent: Child has been stopped. Moving right along...\n");
                        push(&q, front_name(&q), front_number(&q), front_pid(&q), front_prio(&q));
                        pop(&q);

                        if(nhigh>0) ifHigh(&q);

                        if (alarm(TQ) < 0) {
                                perror("alarm");
                                exit(1);
                        }
                        kill(front_pid(&q), SIGCONT);
                }
        }
}

/* Disable delivery of SIGALRM and SIGCHLD. */
static void
signals_disable(void)
{
        sigset_t sigset;

        sigemptyset(&sigset);
        sigaddset(&sigset, SIGALRM);
        sigaddset(&sigset, SIGCHLD);
        if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
                perror("signals_disable: sigprocmask");
                exit(1);
        }
}

/* Enable delivery of SIGALRM and SIGCHLD.  */
static void
signals_enable(void)
{
        sigset_t sigset;

        sigemptyset(&sigset);
        sigaddset(&sigset, SIGALRM);
        sigaddset(&sigset, SIGCHLD);
        if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
                perror("signals_enable: sigprocmask");
                exit(1);
        }
}


/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
static void
install_signal_handlers(void)
{
        sigset_t sigset;
        struct sigaction sa;

        sa.sa_handler = sigchld_handler;
        sa.sa_flags = SA_RESTART;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGCHLD);
        sigaddset(&sigset, SIGALRM);
        sa.sa_mask = sigset;
        if (sigaction(SIGCHLD, &sa, NULL) < 0) {
                perror("sigaction: sigchld");
                exit(1);
        }

        sa.sa_handler = sigalrm_handler;
        if (sigaction(SIGALRM, &sa, NULL) < 0) {
                perror("sigaction: sigalrm");
                exit(1);
        }

        /*
         * Ignore SIGPIPE, so that write()s to pipes
         * with no reader do not result in us being killed,
         * and write() returns EPIPE instead.
         */
        if (signal(SIGPIPE, SIG_IGN) < 0) {
                perror("signal: sigpipe");
                exit(1);
        }
}

static void
do_shell(char *executable, int wfd, int rfd)
{
        char arg1[10], arg2[10];
        char *newargv[] = { executable, NULL, NULL, NULL };
        char *newenviron[] = { NULL };

        sprintf(arg1, "%05d", wfd);
        sprintf(arg2, "%05d", rfd);
        newargv[1] = arg1;
        newargv[2] = arg2;

        raise(SIGSTOP);
        execve(executable, newargv, newenviron);

        /* execve() only returns on error */
        perror("scheduler: child: execve");
        exit(1);
}

/* Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
static void
sched_create_shell(char *executable, int *request_fd, int *return_fd)
{
        pid_t p;
        int pfds_rq[2], pfds_ret[2];

        if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0) {
                perror("pipe");
                exit(1);
        }

        p = fork();
        if (p < 0) {
                perror("scheduler: fork");
                exit(1);
        }

        if (p == 0) {
                /* Child */
                close(pfds_rq[0]);
                close(pfds_ret[1]);
                do_shell(executable, pfds_rq[1], pfds_ret[0]);
                assert(0);
        }
        /* Parent */
        printf("CCreate child with PID = %d \n", p);
        push(&q, executable, 10000, p, 0);
        printf("Name is %s\n", front_name(&q));
        close(pfds_rq[1]);
        close(pfds_ret[0]);
        *request_fd = pfds_rq[0];
        *return_fd = pfds_ret[1];
}

static void
shell_request_loop(int request_fd, int return_fd)
{
        int ret;
        struct request_struct rq;

        /*
         * Keep receiving requests from the shell.
         */
        for (;;) {
                if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
                        perror("scheduler: read from shell");
                        fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
                        break;
                }

        }
}

int main(int argc, char *argv[])
{
        /* Two file descriptors for communication with the shell */
        static int request_fd, return_fd;

        /* Create the shell. */
        //sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);
        /* TODO: add the shell to the scheduler's tasks */

        /*
         * For each of argv[1] to argv[argc - 1],
         * create a new child process, add it to the process list.
         */
        int nproc = argc-1;
        init(&q);
        pid_t pid[nproc+1];
        int i=0;



        for(i=0; i<nproc ; i++){
                pid[i] = fork();
                if(pid[i] == 0){
                        char *newargv[] = { argv[i+1] , NULL, NULL, NULL};
                        //char *newenviron[] = { NULL };
                        raise(SIGSTOP);
                        execve(argv[i+1], newargv,NULL);
                        //for(;;){
                        //      printf("My No = %d\n", i+1);
                        //      sleep(1);
                        //}
                }
                else if( pid[i] > 0){
                        printf("Create child with PID = %d\n", pid[i]);
                        push(&q, argv[i+1], i+1, pid[i],0);
                }
                else printf("Problem with Fork");
        }
        sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);


        /* Wait for all children to raise SIGSTOP before exec()ing. */
        wait_for_ready_children(nproc);

        /* Install SIGALRM and SIGCHLD handlers. */
        install_signal_handlers();

        alarm(TQ);
        //kill(front_pid(&q), SIGCONT);

        nproc += 1;
        if (nproc == 0) {
                fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
                exit(1);
 }

        shell_request_loop(request_fd, return_fd);

        /* Now that the shell is gone, just loop forever
         * until we exit from inside a signal handler.
         */
        while (pause())
                ;

        /* Unreachable */
        fprintf(stderr, "Internal error: Reached unreachable point\n");
        return 1;

