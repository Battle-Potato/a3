#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smallsh.h"
#include "cd.h"
#include "exit.h"
#include "status.h"
#include "structs.h"


#define BUFFER_SIZE 2048

int main(){
    char* input = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char* command = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char* args = (char*)calloc(BUFFER_SIZE, sizeof(char));

    while(1){
        //these need to be converted later to a non printf version reference 4/24 lecture
        printf(": ");
        fgets(input, BUFFER_SIZE, stdin);
        int args_len = get_args(input, command, args);

        exec_params* params = parse_args(args, args_len);

        if(strcmp(command, "cd") == 0){
            cd(params);
        } else if(strcmp(command, "exit") == 0){
            smallsh_exit(params);
            return 0;
        } else if(strcmp(command, "status") == 0){
            status(params);
        } else if(strcmp(command, "") == 0){
            continue;
        } else if(strcmp(command, "#") == 0){
            continue;
        } else {
            printf("Command not recognized\n");
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
    return len;
}

exec_params* parse_args(char* args, int length){
    exec_params* params = (exec_params*)malloc(sizeof(exec_params));
    params->background = 0;
    params->input_file = NULL;
    params->output_file = NULL;
    params->args = NULL;

    //parse args
    if (length == 0){
        return params;
    }
    if (args[length - 1] == '&'){
        params->background = 1;
        args[length - 1] = '\0';
        length--;
    }
    
    //until > or < is found, do nothing.
    
    //if either are found, null the space before the char

    //if < is found, copy the next word into input_file

    //if > is found, copy the next word into output_file

    return params;
}
