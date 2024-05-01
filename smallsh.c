#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cd.h"
#include "exit.h"
#include "status.h"

#define BUFFER_SIZE 2048

int main(){
    char* input = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char* command = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char* args = (char*)calloc(BUFFER_SIZE, sizeof(char));

    while(1){
        printf(": ");
        fgets(input, BUFFER_SIZE, stdin);
        split_args(input, command, args);

        printf("Input: %s\n", input);
        printf("Command: %s\n", command);
        printf("Args: %s\n", args);

        if(strcmp(command, "cd") == 0){
            cd(args);
        } else if(strcmp(command, "exit") == 0){
            smallsh_exit(args);
            return 0;
        } else if(strcmp(command, "status") == 0){
            status(args);
        } else {
            printf("Command not recognized\n");
        }
    }

}


void split_args(char* input, char* command, char* args){
    
    //find first space or \n
    int i = 0;
    while(input[i] != ' ' && input[i] != '\n'){
        i++;
    }
    for(int j = 0; j < i; j++){
        (*command)[j] = input[j];
    }

}
