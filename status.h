#include "smallsh.h"
#include "stringr.h"


typedef struct status{
    pid_t pid;
    int wstatus;
    bool amp;
}statusStruct;

typedef struct linkedlistNode{
    statusStruct *data;
    struct linkedlistNode *next;
}linkedlistNodeStruct;

typedef struct linkedlist{
    linkedlistNodeStruct *head;
}linkedlistStruct;


void linkedlistBuild(linkedlistStruct *linkedlist);
void linkedlistPrintForegroundStatusSignal(linkedlistStruct *linkedlist);
void linkedlistPrintBackgroundStatusSignal(linkedlistStruct *linkedlist);
void linkedlistAdd(linkedlistStruct *linkedlist, pid_t pid, int wstatus, bool amp);


void linkedlistPrint(linkedlistStruct *linkedlist){
    /**
     * Print linked list of status structs to stdin
     * Usage non-reentrant debugging
     * :param linkedlist: Linked list of status structs
     * :return: Null
     */
    linkedlistNodeStruct *cur = linkedlist->head->next;
    while(cur != NULL){
        printf("status::pid:\"%d\",wstatus:\"%d\",amp:\"%d\"\n", cur->data->pid, cur->data->wstatus, cur->data->amp);
        cur = cur->next;
    }
}

void linkedlistBuild(linkedlistStruct *linkedlist){
    /**
     * Create new linked list struct with head sentinel
     * :param linkedlist: Linked list struct
     * :return: Null
     */
    linkedlistNodeStruct *linkedlistNode = malloc(sizeof(linkedlistNodeStruct));
    linkedlistNode->data = NULL;
    linkedlistNode->next = NULL;
    linkedlist->head = linkedlistNode;
}

void linkedlistFree(linkedlistStruct *linkedlist){
    /**
     * Free linked list struct memory
     * :param linkedlist: Linked list struct
     * :return: Null
     */
    linkedlistNodeStruct *cur = linkedlist->head->next;
    while(cur != NULL){
        free(cur->data);
        linkedlist->head->next = cur->next;
        free(cur);
        cur = linkedlist->head->next;
    }
    free(linkedlist->head);
    free(linkedlist);
}

void linkedlistPrintForegroundStatusSignal(linkedlistStruct *linkedlist){
    /**
     * Print linked list first foreground process exit status or termination signal
     * :param linkedlist: Linked list
     * :return: Null
     */
    linkedlistNodeStruct *cur = linkedlist->head->next;
    bool allamp = true;

    // Filter foreground processes
    while(cur != NULL){
        if(cur->data->amp == false){
            allamp = false;
            // Print wstatus
            if(WIFEXITED(cur->data->wstatus)){
                char wexitstatus[16];
                memset(wexitstatus, 0, 16);
                sprintf(wexitstatus, "%d", WEXITSTATUS(cur->data->wstatus));
                char *msg = "exitstatus: ";
                write(STDOUT_FILENO, msg, strlen_r(msg));
                fflush(stdout);
                write(STDOUT_FILENO, wexitstatus, strlen_r(wexitstatus));
                fflush(stdout);
                write(STDOUT_FILENO, "\n", 1);
                fflush(stdout);
            }else{ // WIFSIGNALED
                char wtermsig[16];
                memset(wtermsig, 0, 16);
                sprintf(wtermsig, "%d", WTERMSIG(cur->data->wstatus));
                char *msg = "termsig: ";
                write(STDOUT_FILENO, msg, strlen_r(msg));
                fflush(stdout);
                write(STDOUT_FILENO, wtermsig, strlen_r(wtermsig));
                fflush(stdout);
                write(STDOUT_FILENO, "\n", 1);
                fflush(stdout);
            }
        }
        cur = cur->next;
    }

    // No past foreground processes
    if(allamp == true){
        char *msg = "exitstatus: 0\n";
        write(STDOUT_FILENO, msg, strlen_r(msg));
        fflush(stdout);
    }
}

void linkedlistRemoveForegroundTail(linkedlistStruct *linkedlist){
    /**
     * Remove linked list tail foreground processes after first foreground process
     * :param linkedlist: Linked list
     * :return: Null
     */
    linkedlistNodeStruct *prev = linkedlist->head;
    linkedlistNodeStruct *cur = linkedlist->head->next;
    bool allamp = true;
    while(cur != NULL){
        if(cur->data->amp == false){
            if(allamp == false){ // Past first foreground process
                prev->next = cur->next;
                free(cur->data);
                free(cur);
                cur = prev->next;
            }else{ // First foreground process
                allamp = false;
                prev = cur;
                cur = cur->next;
            }
        }else{ // Background process
            prev = cur;
            cur = cur->next;
        }
    }
}

void linkedlistPrintBackgroundStatusSignal(linkedlistStruct *linkedlist){
    /**
     * Print linked list finished background processes exit status or termination signal
     * :param linkedlist: Linked list
     * :return: Null
     */
    linkedlistNodeStruct *cur = linkedlist->head->next;
    linkedlistNodeStruct *prev = linkedlist->head;
    while(cur != NULL){
        if(cur->data->amp == true && waitpid(cur->data->pid, &(cur->data->wstatus), WNOHANG) != 0){
            // Print PID
            char spid[10];
            memset(spid, 0, 10);
            sprintf(spid, "%d", cur->data->pid);
            char *msg = "pid: ";
            write(STDOUT_FILENO, msg, strlen_r(msg));
            fflush(stdout);
            write(STDOUT_FILENO, spid, strlen_r(spid));
            fflush(stdout);
            write(STDOUT_FILENO, "\n", 1);
            fflush(stdout);

            // Print wstatus
            if(WIFEXITED(cur->data->wstatus)){
                char wexitstatus[16];
                memset(wexitstatus, 0, 16);
                sprintf(wexitstatus, "%d", WEXITSTATUS(cur->data->wstatus));
                char *msg = "exitstatus: ";
                write(STDOUT_FILENO, msg, strlen_r(msg));
                fflush(stdout);
                write(STDOUT_FILENO, wexitstatus, strlen_r(wexitstatus));
                fflush(stdout);
                write(STDOUT_FILENO, "\n", 1);
                fflush(stdout);
            }else{ // WIFSIGNALED
                char wtermsig[16];
                memset(wtermsig, 0, 16);
                sprintf(wtermsig, "%d", WTERMSIG(cur->data->wstatus));
                char *msg = "termsig: ";
                write(STDOUT_FILENO, msg, strlen_r(msg));
                fflush(stdout);
                write(STDOUT_FILENO, wtermsig, strlen_r(wtermsig));
                fflush(stdout);
                write(STDOUT_FILENO, "\n", 1);
                fflush(stdout);
            }

            // Remove finished background process linked list node
            prev->next = cur->next;
            free(cur->data);
            free(cur);
            cur = prev->next;
        }else{
            prev = cur;
            cur = cur->next;
        }
    }
}

void linkedlistAdd(linkedlistStruct *linkedlist, pid_t pid, int wstatus, bool amp){
    /**
     * Linked list add status node
     * :param linkedlist: Linked list
     * :param pid: Status PID
     * :param wstatus: Status wstatus
     * :param amp: Status process foreground or background
     * :return: Null
     */
    statusStruct *status = malloc(sizeof(statusStruct));
    status->pid = pid;
    status->wstatus = wstatus;
    status->amp = amp;

    linkedlistNodeStruct *linkedlistNode = malloc(sizeof(linkedlistNodeStruct));
    linkedlistNode->data = status;
    linkedlistNode->next = linkedlist->head->next;
    linkedlist->head->next = linkedlistNode;
}
