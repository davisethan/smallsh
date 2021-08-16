#include "signal.h"


typedef struct command{
    char cmd[CHAR_MAX];
    char args[ARG_MAX][CHAR_MAX];
    char inrdr[2];
    char infile[CHAR_MAX];
    char outrdr[2];
    char outfile[CHAR_MAX];
    char amp[2];
}commandStruct;


void commandPrint(commandStruct *command);
void commandClear(commandStruct *command);
void commandBuild(commandStruct *command, char *cmdline);
bool commandSkipLine(commandStruct *command);
void commandExpandPID(commandStruct *command);
void strExpandPID(char *newstr, char *str);
bool commandIsBuiltin(commandStruct *command);
void commandBuiltin(commandStruct *command, linkedlistStruct *linkedlist);
void commandNonBuiltin(commandStruct *command, linkedlistStruct *linkedlist);
void commandVector(char **newargv, commandStruct *command);


void commandPrint(commandStruct *command){
    /**
     * Print command struct to stdin
     * Usage non-reentrant debugging
     * :param command: Command struct
     * :return: Null
     */
    if(strlen_r(command->cmd) > 0){
        printf("cmd:\"%s\"\n", command->cmd);
    }
    for(int i=0; strlen_r(command->args[i]) > 0; i++){
        printf("args[%d]:\"%s\"\n", i, command->args[i]);
    }
    if(strlen_r(command->inrdr) > 0){
        printf("inrdr:\"%s\"\n", command->inrdr);
        printf("infile:\"%s\"\n", command->infile);
    }
    if(strlen_r(command->outrdr) > 0){
        printf("outrdr:\"%s\"\n", command->outrdr);
        printf("outfile:\"%s\"\n", command->outfile);
    }
    if(strlen_r(command->amp) > 0){
        printf("amp:\"%s\"\n", command->amp);
    }
}

void commandClear(commandStruct *command){
    /**
     * Clear command struct memory for reuse
     * :param command: Command struct
     * :return: Null
     */
    if(strlen_r(command->cmd) > 0){
        memset(command->cmd, 0, CHAR_MAX);
    }
    for(int i=0; strlen_r(command->args[i]) > 0; i++){
        memset(command->args[i], 0, CHAR_MAX);
    }
    if(strlen_r(command->inrdr) > 0){
        memset(command->inrdr, 0, 2);
        memset(command->infile, 0, CHAR_MAX);
    }
    if(strlen_r(command->outrdr) > 0){
        memset(command->outrdr, 0, 2);
        memset(command->outfile, 0, CHAR_MAX);
    }
    if(strlen_r(command->amp) > 0){
        memset(command->amp, 0, 2);
    }
}

void commandBuild(commandStruct *command, char *cmdline){
    /**
     * Build command struct from command line string
     * :param cmdline: Command line string
     * :return: Command struct
     */
    char *saveptr;
    char *token = strtok_r(cmdline, " ", &saveptr);
    int commandargc = 0;
    while(token != NULL){
        if(strlen_r(command->cmd) == 0){ // Set cmd
            removeNewline(&token);
            strncpy(command->cmd, token, strlen_r(token));
        }else if(strncmp(token, "<", 1) == 0){ // Set input file
            removeNewline(&token);
            strncpy(command->inrdr, "<", 2);
            token = strtok_r(NULL, " ", &saveptr);
            removeNewline(&token);
            strncpy(command->infile, token, strlen_r(token));
        }else if(strncmp(token, ">", 1) == 0){ // Set output file
            removeNewline(&token);
            strncpy(command->outrdr, ">", 2);
            token = strtok_r(NULL, " ", &saveptr);
            removeNewline(&token);
            strncpy(command->outfile, token, strlen_r(token));
        }else if(strncmp(token, "&\n", 2) == 0){ // Set background
            removeNewline(&token);
            strncpy(command->amp, "&", 2);
        }else{ // Set argument
            removeNewline(&token);
            strncpy(command->args[commandargc], token, strlen_r(token));
            commandargc = commandargc+1;
        }
        token = strtok_r(NULL, " ", &saveptr);
    }
}

bool commandSkipLine(commandStruct *command){
    /**
     * Command struct cmd is comment or blank line
     * :param cmd: Command struct cmd
     * :return: Command struct cmd is comment or blank line
     */
    return strncmp(command->cmd, "#", 1) == 0 || strncmp(command->cmd, "\n", 1) == 0;
}

void commandExpandPID(commandStruct *command){
    /**
     * Command struct expand PID
     * :param command: Command struct
     * :return: Null
     */
    // Command struct cmd expand PID
    char newcmd[CHAR_MAX];
    memset(newcmd, 0, CHAR_MAX);
    strExpandPID(newcmd, command->cmd);
    strncpy(command->cmd, newcmd, strlen_r(newcmd));

    // Command struct args expand PID
    char newarg[ARG_MAX][CHAR_MAX];
    for(int i=0; i < ARG_MAX; i++){
        memset(newarg[i], 0, CHAR_MAX);
        strExpandPID(newarg[i], command->args[i]);
        strncpy(command->args[i], newarg[i], strlen_r(newarg[i]));
    }

    // Command struct infile expand PID
    char newinfile[CHAR_MAX];
    memset(newinfile, 0, CHAR_MAX);
    strExpandPID(newinfile, command->infile);
    strncpy(command->infile, newinfile, strlen_r(newinfile));

    // Command struct outfile expand PID
    char newoutfile[CHAR_MAX];
    memset(newoutfile, 0, CHAR_MAX);
    strExpandPID(newoutfile, command->outfile);
    strncpy(command->outfile, newoutfile, strlen_r(newoutfile));
}

void strExpandPID(char *newstr, char *str){
    /**
     * String expand PID
     * `commandExpandPID` helper function
     * :param newstr: New string with expanded PID
     * :param str: Current string with un-expanded PID
     * :return: Null
     */
    char *delim = "$$";
    pid_t pid = getpid();
    char spid[10];
    memset(spid, 0, 10);
    sprintf(spid, "%d", pid);

    char *end = strstr(str, delim);
    while(end != NULL){
        strncat(newstr, str, end-str);
        strncat(newstr, spid, strlen_r(spid));
        str = end+2;
        end = strstr(str, delim);
    }
    strncat(newstr, str, strlen_r(str));
}

bool commandIsBuiltin(commandStruct *command){
    /**
     * Command struct cmd is in {exit, cd, status}
     * :param command: Command struct
     * :return: Command struct cmd is in {exit, cd, status}
     */
    return strncmp(command->cmd, "exit", 4) == 0 || strncmp(command->cmd, "cd", 2) == 0 || strncmp(command->cmd, "status", 6) == 0;
}

void commandBuiltin(commandStruct *command, linkedlistStruct *linkedlist){
    /**
     * Command struct cmd some {exit, cd, status}
     * :param command: Command struct
     * :param status: Status struct
     * :return: Null
     */
    if(strncmp(command->cmd, "exit", 4) == 0){
        // Terminate all children processes
        killChildren();
        // Terminate self
        exit(EXIT_SUCCESS);
    }else if(strncmp(command->cmd, "cd", 2) == 0){
        char *path;
        if(strlen_r(command->args[0]) > 0){
            path = command->args[0];
        }else{
            path = getenv("HOME");
        }
        chdir(path);
    }else if(strncmp(command->cmd, "status", 6) == 0){
        linkedlistRemoveForegroundTail(linkedlist);
        linkedlistPrintForegroundStatusSignal(linkedlist);
    }
}

void commandNonBuiltin(commandStruct *command, linkedlistStruct *linkedlist){
    /**
     * Command struct cmd not some {exit, cd, status}
     * :param command: Command struct
     * :param status: Status struct
     * :return: Null
     */
    char **newargv = malloc((ARG_MAX+6)*sizeof(char));
    commandVector(newargv, command);

    int childstatus;
    pid_t childpid = fork();
    switch(childpid){
    case -1: // Fork error
        perror("Error: fork()");
        exit(errno);
        break;
    case 0: // Child process
        if(strlen_r(command->inrdr) > 0){ // Redirect input
            int infilefd = open(command->infile, O_RDONLY, 0640);
            if(infilefd == -1){
                perror("Error: open()");
                exit(errno);
            }
            int inres = dup2(infilefd, STDIN_FILENO);
            if(inres == -1){
                perror("Error: dup2()");
                exit(errno);
            }
        }

        if(strlen_r(command->outrdr) > 0){ // Redirect output
            int outfilefd = open(command->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if(outfilefd == -1){
                perror("Error: open()");
                exit(errno);
            }
            int outres = dup2(outfilefd, STDOUT_FILENO);
            if(outres == -1){
                perror("Error: dup2()");
                exit(errno);
            }
        }

        if(strlen_r(command->amp) > 0 && shouldamp == true){ // Background process
            if(strlen_r(command->inrdr) == 0){ // Redirect input
                int infilefd = open("/dev/null", O_RDONLY, 0640);
                if(infilefd == -1){
                    perror("Error: open()");
                    exit(errno);
                }
                int inres = dup2(infilefd, STDIN_FILENO);
                if(inres == -1){
                    perror("Error: dup2()");
                    exit(errno);
                }
            }
            if(strlen_r(command->outrdr) == 0){ // Redirect output
                int outfilefd = open("/dev/null", O_WRONLY, 0640);
                if(outfilefd == -1){
                    perror("Error: open()");
                    exit(errno);
                }
                int outres = dup2(outfilefd, STDOUT_FILENO);
                if(outres == -1){
                    perror("Error: dup2()");
                    exit(errno);
                }
            }
        }else{ // Foreground process
            // Signal handlers
            SIGINT_default_handler();
        }

        execvp(newargv[0], newargv);
        perror("Error: execvp()");
        exit(errno);
        break;
    default: // Parent process
        if(strncmp(command->amp, "&", 1) == 0 && shouldamp == true){
            // Print background PID
            char schildpid[10];
            memset(schildpid, 0, 10);
            sprintf(schildpid, "%d", childpid);
            write(STDOUT_FILENO, "pid: ", 5);
            fflush(stdout);
            write(STDOUT_FILENO, schildpid, strlen_r(schildpid));
            fflush(stdout);
            write(STDOUT_FILENO, "\n", 1);
            fflush(stdout);

            waitpid(childpid, &childstatus, WNOHANG);
            linkedlistAdd(linkedlist, childpid, childstatus, true);
        }else{
            waitpid(childpid, &childstatus, 0);
            linkedlistAdd(linkedlist, childpid, childstatus, false);
        }
        break;
    }

    free(newargv);
}

void commandVector(char **newargv, commandStruct *command){
    /**
     * Command struct to execvp vector
     * :param newargv: execvp vector
     * :param command: Command struct
     * :return: Null
     */
    int newargc = 0;
    if(strlen_r(command->cmd) > 0){ // Set cmd
        newargv[newargc] = command->cmd;
        newargc = newargc+1;
    }
    for(int i=0; i < ARG_MAX; i++){ // Set argument
        if(strlen_r(command->args[i]) > 0){
            newargv[newargc] = command->args[i];
            newargc = newargc+1;
        }
    }
    newargv[newargc] = NULL;
}
