/*
Assignment: smallsh
File: smallsh.c
Author: Tristan Vosburg
Date: 5/22/2024
Description: Main file for a small shell with 3 built in commands, cd, exit, and status,
                as well as the ability to run external commands.
                The shell also has the ability to run commands in the background and to redirect input and output.
                The shell also has the ability to ignore SIGINT and SIGTSTP signals.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>           
#include "smallsh.h"
#include "cd.h"
#include "exit.h"
#include "status.h"
#include "structs.h"


#define BUFFER_SIZE 2048

pid_t bg_processes[52];
pid_t fg_process;
pid_t shell_pid;
int i_status = 0;
int sig = 0;
int background_lock = 0;
int change_lock = 0;
int signal_caught = 0;
int cleanup_pid = 0;

int main(){
    handler_setup();
    shell_pid = getpid();
    //main shell loop
    shell();

    return 0;
}

void shell(){
    //initialize input line
    char* input = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char* command = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char* args = (char*)calloc(BUFFER_SIZE, sizeof(char));
    int run = 1;

    //main shell loop
    while(run){
        run = one_loop(input, command, args);
    }

    return;
}

int one_loop(char* input, char* command, char* args){
        reset_args(input, command, args);

        //handle background lock
        handle_background();
        if(signal_caught == 1){
            if(cleanup_pid != 0)
                clean_terminated();
            signal_caught = 0;
            return 1;
        }

        kill_zombies();
       
       //get input
        printf(" : ");
        fflush(stdout);
        fgets(input, BUFFER_SIZE, stdin);    

        //prevents shell from "munching an input" if a signal is caught in the neutral state
        if(signal_caught == 1){
            signal_caught = 0;
            return 1;
        }

        expand_input(input);    

        //divide input into command and args
        int args_len = get_args(input, command, args);

        //set params
        exec_params* params = set_params(args, args_len);

        //built in command tree
        if(strcmp(command, "cd") == 0){ //change directory
            cd(params);
        } else if(strcmp(command, "exit") == 0){ //terminate shell
            smallsh_exit(bg_processes);
            free(params);
            free(input);
            free(command);
            free(args);
            return 0;
        } else if(strcmp(command, "status") == 0){ //get status of last foreground process
            status(i_status, sig);
            sig = 0;
        } else if(strcmp(command, "") != 0 && strcmp(command, "#") != 0) {    //external or unknown command
            external_command(command, params, &i_status);
        }
        //reset params
        free(params);
        return 1;
}

void clean_terminated(){
    //get the status of the process that was terminated by CTRL-C
    waitpid(cleanup_pid, &i_status, WNOHANG);
    if(WIFEXITED(i_status)){
        printf("Foreground process %d exited with status %d\n", cleanup_pid, WEXITSTATUS(i_status));
    }
    else if(WIFSIGNALED(i_status)){
        printf("Foreground process %d terminated by signal %d\n", cleanup_pid, WTERMSIG(i_status));
    }
    cleanup_pid = 0;
}

void handle_background(){
    if(change_lock == 1){
        change_lock = 0;
        if(background_lock == 0)
            write(1, "Exiting foreground-only mode\n", 30);
        else
            write(1, "Entering foreground-only mode (& is now ignored)\n", 49);
    }
}

void reset_args(char* input, char* command, char* args){
    //set every element of input, command, and args to null
    for(int i = 0; i < BUFFER_SIZE; i++){
        input[i] = '\0';
        command[i] = '\0';
        args[i] = '\0';
    }
}

void kill_zombies(){
    int i = 0;
    //iterate through bg_processes and check if any have terminated
    while(bg_processes[i] != 0){
        int status;
        pid_t pid = waitpid(bg_processes[i], &status, WNOHANG);
        if(pid > 0){
            if(WIFEXITED(status)){
                printf("Background process %d exited with status %d\n", bg_processes[i], WEXITSTATUS(status));
            }
            else if(WIFSIGNALED(status)){
                printf("Background process %d terminated by signal %d\n", bg_processes[i], WTERMSIG(status));
            }
            bg_processes[i] = 0;
        }
        i++;
    }
}

int get_args(char* input, char* command, char* args){
    //check for comments or empty lines
    if(input[0] == '#' || input[0] == '\n'){
        command[0] = '\0';
        return 0;
    }
    //find first space or \n
    int i = 0;
    while(input[i] != ' ' && input[i] != '\n'){
        i++;
    }

    //copy command
    strncpy(command, input, i);
    command[i] = '\0';

    //if there are no args, set args to null and return 0
    if(input[i] == '\n'){
        args[0] = '\0';
        return 0;
    }
    i++;
    //copy args
    strcpy(args, input + i);
    int len = strlen(args);
    args[--len] = '\0';
    return len - 1;
}

exec_params* set_params(char* args, int length){
    //create params struct
    exec_params* params = (exec_params*)calloc(1, sizeof(exec_params));
    params->length = length;
    params->clean_args = args;

    //if the length of args is 0, there is nothing to parse
    if (length == 0){
        return params;
    }

    //check for background process
    if (args[length] == '&'){
        if(background_lock == 0)
            params->background = 1;
        args[length] = '\0';
        length--;
    }

    //get input char
    params->input_file = strchr(args, '<');

    //get output char
    params->output_file = strchr(args, '>');

    //remove trailing spaces
    char* seeker = NULL;

    //if there is an input file...
    if(params->input_file != NULL){
        //... then remove the symbol ...
        *params->input_file = '\0';
        params->input_file++;
        //... and find the start of the file name ...
        while(*params->input_file == ' '){
            params->input_file++;
        }
        // ... and make sure the file name is null terminated
        seeker = params->input_file;
        while(*seeker != ' ' && *seeker != '\0'){
            seeker++;
        }
        *seeker = '\0';
    }
    //if there is an output file...
    if(params->output_file != NULL){
        //... then remove the symbol ...
        *params->output_file = '\0';
        params->output_file++;
        //... and find the start of the file name ...
        while(*params->output_file == ' '){
            params->output_file++;
        }
        // ... and make sure the file name is null terminated
        seeker = params->output_file;
        while(*seeker != ' ' && *seeker != '\0'){
            seeker++;
        }
        *seeker = '\0';
    }

    return params;
}

int redirect_input(exec_params* params){
    int fd = -2;
    //if the process is a background process and there is no input file, redirect to /dev/null
    if(params->background == 1 && params->input_file == NULL )
        fd = open("/dev/null", O_RDONLY, 0644);
    //if there is an input file, open it
    else if(params->input_file != NULL)
        fd = open(params->input_file, O_RDONLY);
    //if the file could not be opened, return 1
    if(fd == -1){
        return 1;
    }
    //if there is no file to open, return 0
    if(fd == -2){
        return 0;
    }
    //redirect input
    int new_fd = dup2(fd, 0);
    //if the file could not be redirected, exit
    if(new_fd == -1){
        exit(1);
    }
    //close the file
    close(fd);
    return 0;
}

int redirect_output(exec_params* params){
    int fd = -2;
    //if the process is a background process and there is no output file, redirect to /dev/null
    if(params->background == 1 && params->output_file == NULL)
        fd = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    //if there is an output file, open it
    else if(params->output_file != NULL)
        fd = open(params->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    //if the file could not be opened, return 1
    if(fd == -1){
        return 1;
    }
    //if there is no file to open, return 0
    if(fd == -2){
        return 0;
    }
    //redirect output
    int new_fd = dup2(fd, 1);
    //if the file could not be redirected, exit
    if(new_fd == -1){
        exit(1);
    }
    //close the file
    close(fd);
    return 0;
}


void external_command(char* command, exec_params* params, int* status){
    //get args in char** form
    char** args = parse_input(params, command);
    //fork
    pid_t child_pid = fork();
    //if fork failed, exit
    if(child_pid == -1){
        exit(1);
    }
    if(child_pid == 0){  //child process
        //redirect input
        int input_status = redirect_input(params);

        //redirect output
        int output_status = redirect_output(params);

        //if either redirect failed, exit
        if(output_status == 1){
            write(1, "Error redirecting output\n", 26);
            exit(1);
        }
        else if (input_status == 1){
            write(1, "Error redirecting input\n", 25);
            exit(1);
        }
        else{
            struct sigaction SIGINT_ignore, SIGTSTP_ignore;

            //if the process is a background process, ignore SIGINT
            if(params->background == 1){

                SIGINT_ignore.sa_handler = SIG_IGN;
                sigfillset(&SIGINT_ignore.sa_mask);
                SIGINT_ignore.sa_flags = 0;
                sigaction(SIGINT, &SIGINT_ignore, NULL);
            }
            //ignore SIGTSTP
            SIGTSTP_ignore.sa_handler = SIG_IGN;
            sigfillset(&SIGTSTP_ignore.sa_mask);
            SIGTSTP_ignore.sa_flags = 0;
            sigaction(SIGTSTP, &SIGTSTP_ignore, NULL);

            //execute command
            execvp(command, args);

            //if execvp failed, exit
            write(1, "Invalid command\n", 16);
            exit(1);
        }
    }
    else{   //parent process
        //if the process is not a background process, wait for it to finish
        if(params->background == 0){
            //update fg_process if there is a signal
            fg_process = child_pid;

            //wait for child process to finish and get status
            waitpid(child_pid, status, 0);
            *status = WEXITSTATUS(*status);
            fg_process = 0;
        }
        //if the process is a background process, add it to bg_processes
        else{
            int i = 0;
            //look for first empty spot in bg_processes
            while(bg_processes[i] != 0){
                i++;
            }
            //add child_pid to bg_processes
            bg_processes[i] = child_pid;
            printf("Background process started with PID %d\n", child_pid);
        }
    }
}

char** parse_input(exec_params* params, char* command){
    //create 52 args
    char** args = (char**)calloc(52, sizeof(char*));
    char* token = strtok(params->clean_args, " ");
    //set args[0] to command
    args[0] = command;
    int i = 1;
    //set args[i] to the char after every space
    while(token != NULL){
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    args[i] = NULL;
    return args;
}

void expand_input(char* input){
    int i = 0;
    while(1){
        //break if at end of line
        if(input[i] == '\n')
            break;
        if(input[i] == '$'){
            //check if & is at the end of the line
            if(input[i + 1] == '\n'){
                break;
            }
            //check if & is followed by &
            if(input[i + 1] == '$'){
                input[i] = '\0';
                input[i + 1] = '\0';
                //get pid length
                char pid[10];
                sprintf(pid, "%d", shell_pid);
                int pid_len = strlen(pid);
                char* next_char = input + i + 2;
                //shift next_char and everything after to the right by pid_len - 2
                char* seeker = next_char;
                while(*seeker != '\n'){
                    seeker++;
                }
                while(seeker != next_char - 1){
                    char* destination = seeker + pid_len - 2;
                    *destination = *seeker;
                    seeker--;
                }
                //copy pid into input
                for(int j = 0; j < pid_len; j++){
                    input[i + j] = pid[j];
                }
            }
        }
        i++;
    }
}

void handler_setup(){
    struct sigaction SIGTSTP_action, SIGINT_action;

    //use custom handlers for SIGTSTP and SIGINT
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);   

    SIGINT_action.sa_handler = handle_SIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);
}

void handle_SIGINT(int signo){
    signal_caught = 1;
    sig = signo;
    cleanup_pid = fg_process;
}

void handle_SIGTSTP(int signo){
    signal_caught = 1;
    change_lock = 1;
    if(background_lock == 0){
        background_lock = 1;
    }
    else{
        background_lock = 0;
    }
    waitpid(fg_process, &i_status, 0);
    fg_process = 0;

}