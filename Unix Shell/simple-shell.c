/**
 * Simple shell interface program.
 *
 * Operating System Concepts - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_LINE		80 /* 80 chars per line, per command */
#define MAX_HISTORY		20 /* total commands that could be stored in history buffer */

int main(void)
{
	char *args[MAX_LINE/2 + 1] ;	/* command line (of 80) has max of 40 arguments */
    int should_run = 1 ;
    char history[MAX_HISTORY][MAX_LINE] ; // 2D array to store 20 most recent commands (history buffer)
    int historyBufferCounter = 0 ; // history buffer commands counter

    while (should_run){

        // allocating buffer for user input
        char buf[MAX_LINE] ;


        printf("osh > ");
        fflush(stdout); // purpose is to clear the output buffer and move buffered data to console


        // Getting user input
        fgets(buf, MAX_LINE, stdin) ;
        buf[strcspn(buf, "\n")] = 0 ; // removing trailing newline if present
        //printf("string is: %s\n", buf) ; // for printing user input (just for debugging)


        // Checking for empty input
        if (strlen(buf) == 0) {
            continue ;
        }


        // Checking if user entered history command i.e !!
        int isHistory = 0 ;
        if (strcmp(buf, "!!") == 0) {

            // Checking if there is any recent command entered in history buffer
            
            if (historyBufferCounter == 0) {
                printf("No commands in history\n") ;
                continue ;
            }

            else {
                isHistory = 1 ;
            }
        }

        // If user entered !! and there is any recent command in history buffer, fetch that command
        if (isHistory == 1) {
            strcpy(buf, history[historyBufferCounter - 1]) ;
            printf("%s\n", history[historyBufferCounter - 1]) ; // echoing command on user screen
        }

        // Otherwise, place the new command entered by user in the history buffer
        else {
            if (historyBufferCounter == MAX_HISTORY) {  // if buffer gets filled, reinitialize
                historyBufferCounter = 0 ; // reset counter
                strcpy(history[historyBufferCounter], buf) ;
                historyBufferCounter = historyBufferCounter + 1 ;
            }

            else {
                strcpy(history[historyBufferCounter], buf) ;
                historyBufferCounter = historyBufferCounter + 1 ;
            }
        }


        // Tokenizing user input
        char *tokens = strtok(buf, " ") ;
        int i = 0;
        int tokenCounter = 0 ;
        while (tokens) {
            //printf("%s\n", tokens) ; // for printing tokens (just for debugging) 
            tokenCounter = tokenCounter + 1 ;
            args[i++] = tokens ;
            tokens = strtok(NULL, " ") ;
        }
        args[i] = NULL ;


        // Checking if user inputted "exit" so shell will be terminated
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0 ;
            continue ;
        }

        // Checking for output redirection i.e. >
        int isRedirection = 0 ; // check for whether > operator found or not
        int isInvalid = 0 ; // check for invalid command with > operator
        int pos ; // position at which > operator detected
        char fileName[MAX_LINE] ; // file name entered by user
        for (int j = 0 ; j < tokenCounter ; j++) {
            if (strcmp(args[j], ">") == 0) {
                if (j - 1 == -1) { // if no system call specified before > operator
                    printf("Invalid command\n") ;
                    isInvalid = 1 ;
                    break ;
                }

                if (j + 1 == tokenCounter) { // if user has not entered any file name after > operator
                    printf("Invalid command\n") ;
                    isInvalid = 1 ;
                    break ;
                }

                else {
                    if (strlen(args[j + 1]) == 4) { // if user only entered the extension but not the file name
                        printf("Invalid file name or extension\n") ;
                        isInvalid = 1 ;
                        break ;
                    }

                    if (strcmp(args[j + 1] + strlen(args[j + 1]) - 4, ".txt") == 0) { // checking file extension
                        strcpy(fileName, args[j + 1]) ; // copy file name after the > operator
                        pos = j ;
                        isRedirection = 1 ;
                        break ;
                    }

                    else { // if any extension other than .txt entered
                        printf("Invalid file name or extension\n") ;
                        isInvalid = 1 ;
                        break ;
                    }
                }
            }
        }

        if (isInvalid == 1)
            continue ;


        // Checking for pipe i.e. |
        int isPipe = 0 ; // pipe command detected or not
        int isInvalid2 = 0 ; // check for invalid command with | operator
        int pos2 ; // position at which | operator detected
        char pipeCommand1[MAX_LINE] ;
        strcpy(pipeCommand1, "") ; // initializing
        char pipeCommand2[MAX_LINE] ;
        strcpy(pipeCommand2, "") ; // initializing
        for (int j = 0 ; j < tokenCounter ; j++) {
            if (strcmp(args[j], "|") == 0) {
                if (j - 1 == -1) { // if no system call specified before | operator
                    printf("Invalid command\n") ;
                    isInvalid2 = 1 ;
                    break ;
                }

                if (j + 1 == tokenCounter) { // if user has not entered any file name after | operator
                    printf("Invalid command\n") ;
                    isInvalid2 = 1 ;
                    break ;
                }

                else {

                    // Now extracting both the commands that are to be executed using pipe

                    pos2 = j ;
                    isPipe = 1 ;
                    for (int k = 0 ; k < pos2 ; k++) {
                        strcat(pipeCommand1, args[k]) ;
                        strcat(pipeCommand1, " ") ;
                    }

                    for (int k = pos2 + 1 ; k < tokenCounter ; k++) {
                        strcat(pipeCommand2, args[k]) ;
                        strcat(pipeCommand2, " ") ;
                    }
                }
            }
        }

        if (isInvalid2 == 1)
            continue ;


        // Checking for ampersand
        int isAmp = 0 ;
        if (strcmp(args[i - 1], "&") == 0) {
            isAmp = 1 ;
            args[i - 1] = NULL ;
            i = i - 1 ;
        }

        // Now concatenating the system call command before the > operator
        char redirCommand[MAX_LINE] ; // redirection system call command name entered by user
        strcpy(redirCommand, "") ; // initializing
        char *args2[MAX_LINE/2 + 1] ; // this args are to be passed to execvp if > is encountered
        if (isRedirection == 1) {
            for (int j = 0 ; j < pos ; j++) {
                strcat(redirCommand, args[j]) ;
                strcat(redirCommand, " ") ;
            }

            // Tokenizing redirCommand
            char *tokens2 = strtok(redirCommand, " ") ;
            int ii = 0;
            while (tokens2) {
                args2[ii++] = tokens2 ;
                tokens2 = strtok(NULL, " ") ;
            }
            args2[ii] = NULL ;
        }


        if (isPipe == 1) {

            char *args3[MAX_LINE/2 + 1] ;
            char *args4[MAX_LINE/2 + 1] ;

            // Tokenizing first command of pipe
            char *tokens3 = strtok(pipeCommand1, " ") ;
            int iii = 0;
            while (tokens3) {
                args3[iii++] = tokens3 ;
                tokens3 = strtok(NULL, " ") ;
            }
            args3[iii] = NULL ;

            // Tokenizing second command of pipe
            char *tokens4 = strtok(pipeCommand2, " ") ;
            int iiii = 0;
            while (tokens4) {
                args4[iiii++] = tokens4 ;
                tokens4 = strtok(NULL, " ") ;
            }
            args4[iiii] = NULL ;

            pid_t childprocess1 ; // to execute first command of pipe
            pid_t childprocess2 ; // to execute second command of pipe

            int myPipe[2] ; // file descriptor array
            if (pipe(myPipe) == -1) {
                printf("Pipe creation failed\n") ;
                continue ;
            }

            if((childprocess1 = fork()) == 0) { // forking child 1
                dup2(myPipe[1], 1) ; // stdout is being redirected to pipe
                close(myPipe[0]) ; // closing read
                execvp(args3[0], args3) ; // executing first command in child 1
                perror("Invalid command") ;
                exit(1) ;
            }

            else if(childprocess1 == -1){ // if forking first child failed
                exit(1) ;
            }

            close(myPipe[1]) ; // close read

            if((childprocess2 = fork()) == 0){ // forking child 2
                dup2(myPipe[0], 0); // stdin is being fetched from pipe
                execvp(args4[0], args4) ; // executing second command in child 2
                perror("Invalid command") ;
                exit(1) ;
            }

            else if(childprocess2 == -1){ // if forking second child failed
                close(myPipe[0]) ; // close read
                wait(NULL); // second child waits for first child to finish execution
                exit(1) ;
            }

            close(myPipe[0]); // close read

            // parent will wait for both the above processes to finish IPC
            wait(NULL) ;
            wait(NULL) ;
        }

        else {
            // (1) forking a child process
            pid_t child_process = fork() ;

            // (2) the child process will invoke execvp()
            if (child_process == 0) {
                if (isRedirection == 1) { // if > operator was entered by user
                    int out = open(fileName, O_WRONLY | O_CREAT) ; // opens file name entered by user. Creats if not exist
                    dup2(out, 1) ;
                    close(out) ;
                    execvp(args2[0], args2) ;
                    printf("%s : No such command found\n", args2[0]) ; // will be printed inside the file content
                    exit(0) ;
                }

                else {
                    execvp(args[0], args) ;
                    printf("%s : No such command found\n", args[0]) ;
                    exit(0) ;
                }
            } 

            else if (child_process < 0) {
                printf("Fork failed!\n") ;
                exit(0) ;
            }

            // (3) if command includes &, parent and child will run concurrently
            else {
                if (isAmp == 0)
                    waitpid(child_process, NULL, 0); ; // parent will wait
            }
        }     
    }
    
	return 0 ;
}