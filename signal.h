#define _POSIX_C_SOURCE 200809L
#include "status.h"


bool shouldamp = true;
void SIGINT_ignore_handler();
void SIGINT_default_handler();
void SIGTSTP_handler();
void SIGTSTP_sa_handler(int signo);


void SIGINT_ignore_handler(){
    /**
     * Signal SIGINT ignore handler
     * :return: Null
     */
    struct sigaction ignore_action = {0};
    // struct sigaction ignore_action = malloc(sizeof(struct sigaction));
    ignore_action.sa_handler = SIG_IGN;
    sigfillset(&ignore_action.sa_mask);
    ignore_action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &ignore_action, NULL);
}

void SIGINT_default_handler(){
    /**
     * Signal SIGINT default handler
     * :return: Null
     */
    struct sigaction default_action = {0};
    default_action.sa_handler = SIG_DFL;
    sigfillset(&default_action.sa_mask);
    default_action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &default_action, NULL);
}

void SIGTSTP_handler(){
    /**
     * Signal SIGTSTP allow background/foreground processes handler
     * :return: Null
     */
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = SIGTSTP_sa_handler;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}

void SIGTSTP_sa_handler(int signo){
    /**
     * Signal SIGTSTP set allow background/foreground processes
     * :param signo: Signal number
     * :return: Null
     */
    if(shouldamp == true){
        char *msg = "Notice: New background processes not allowed\n";
        write(STDOUT_FILENO, msg, strlen_r(msg));
        shouldamp = false;
    }else{ // false
        char *msg = "Notice: New background processes allowed\n";
        write(STDOUT_FILENO, msg, strlen_r(msg));
        shouldamp = true;
    }
}

void killChildren(){
    /**
     * Terminate all children processes
     * Solution from StackOverflow
     * :return: Null
     */
    pid_t pid = getpid();
    signal(SIGTERM, SIG_IGN);
    kill(-pid, SIGTERM);
}
