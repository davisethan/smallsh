#include "command.h"


int main(int argc, char **argv){
    /**
     * General command line syntax
     * `: command [arg1 arg2 ...] [< input_file] [> output_file] [&]`
     */
    // Signal handlers
    SIGINT_ignore_handler();
    SIGTSTP_handler();

    // Open memory
    commandStruct *command = malloc(sizeof(commandStruct));
    linkedlistStruct *linkedlist = malloc(sizeof(linkedlistStruct));
    linkedlistBuild(linkedlist);

    bool loop = true;
    while(loop){
        // Clear memory
        commandClear(command);
        
        // Read stdin
        char cmdline[CHAR_MAX];
        memset(cmdline, 0, CHAR_MAX);
        write(STDOUT_FILENO, PROMPT, strlen_r(PROMPT));
        fflush(stdout);
        fgets(cmdline, CHAR_MAX, stdin);
        
        // Fill memory
        commandBuild(command, cmdline);
        if(commandSkipLine(command)){
            continue;
        }
        commandExpandPID(command);

        // Task memory
        if(commandIsBuiltin(command)){
            commandBuiltin(command, linkedlist);
        }else{
            commandNonBuiltin(command, linkedlist);
        }

        // Display background memory
        linkedlistPrintBackgroundStatusSignal(linkedlist);
    }

    // Clean memory
    free(command);
    linkedlistFree(linkedlist);
    exit(EXIT_SUCCESS);
}
