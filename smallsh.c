#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "smallsh.h"
#include "cd.h"
#include "exit.h"
#include "status.h"
#include "structs.h"


#define BUFFER_SIZE 2048

pid_t bg_processes[52] = {0};
pid_t fg_process;
pid_t shell_pid;

int main(){

    shell_pid = getpid();
    shell();
    return 0;
}

void shell(){
    char* input = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char* command = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char* args = (char*)calloc(BUFFER_SIZE, sizeof(char));
    int i_status = 0;
    int sig = 0;
    while(1){
        //these need to be converted later to a non printf version reference 4/24 lecture
        printf("%s", getcwd(NULL, 0));
        printf(" : ");
        fgets(input, BUFFER_SIZE, stdin);

        //TODO: expand && to pid
        int args_len = get_args(input, command, args);

        exec_params* params = parse_args(args, args_len);

        if(strcmp(command, "cd") == 0){
            cd(params);
        } else if(strcmp(command, "exit") == 0){
            smallsh_exit(params);
            return 0;
        } else if(strcmp(command, "status") == 0){
            status(i_status, sig);
        } else if(strcmp(command, "") == 0){
            continue;
        } else if(strcmp(command, "#") == 0){
            continue;
        } else {
            external_command(command, params, &i_status);
        }
    }
}


int get_args(char* input, char* command, char* args){
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

    if(input[i] == '\n'){
        args[0] = '\0';
        return 0;
    }
    i++;
    strcpy(args, input + i);
    int len = strlen(args);
    args[--len] = '\0';
    return len - 1;
}

exec_params* parse_args(char* args, int length){
    exec_params* params = (exec_params*)malloc(sizeof(exec_params));
    params->background = 0;
    params->input_file = NULL;
    params->output_file = NULL;
    params->clean_args = args;
    params->length = length;

    //parse args
    if (length == 0){
        return params;
    }
    if (args[length] == '&'){
        params->background = 1;
        args[length] = '\0';
        length--;
    }

    params->input_file = strchr(args, '<');
    params->output_file = strchr(args, '>');
    if(params->input_file != NULL){
        *params->input_file = '\0';
        params->input_file++;
        while(*params->input_file == ' '){
            params->input_file++;
        }
    }
    if(params->output_file != NULL){
        *params->output_file = '\0';
        params->output_file++;
        while(*params->output_file == ' '){
            params->output_file++;
        }
    }

    return params;
}

void external_command(char* command, exec_params* params, int* status){
    char** args = parse_input(params);
    for(int i = 0; args[i] != NULL; i++){
        if(args[i] != NULL)
            printf("args[%d]: %s\n", i, args[i]);
    }
    pid_t child_pid = fork();
    if(child_pid == -1){
        perror("fork");
        exit(1);
    }
    if(child_pid == 0){
        //child process
        execvp(command, args);
        perror("execvp");
        exit(1);
    }
    else{
        //parent process
        if(params->background == 0){
            //TODO: prevent build up of zombie processes
            fg_process = child_pid;
            waitpid(child_pid, status, 0);
        }
        else{
            int i = 0;
            while(bg_processes[i] != 0){
                i++;
            }
            printf("Background process started with PID %d\n", child_pid);
        }
    }
}

char** parse_input(exec_params* params){
    char** args = (char**)calloc(52, sizeof(char*));
    char* token = strtok(params->clean_args, " ");
    args[0] = params->clean_args;
    int i = 1;
    while(token != NULL){
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    args[i] = NULL;
    return args;
}