#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cd.h"

int cd(exec_params* params){
    printf("Background processes: %d\n", params->background);
    printf("Input file:%s\n", params->input_file);
    printf("Output file:%s\n", params->output_file);
    printf("Clean args:%s\n", params->clean_args);

    if(params->length == 0){
        chdir(getenv("HOME"));
        return 1;
    }
    if(params->clean_args[0] == '/' || params->clean_args[0] == '.'){
        while(params->clean_args[0] == '/' || params->clean_args[0] == '.'){
            params->clean_args++;
        }
        if(chdir(params->clean_args) == -1){
            perror("chdir");
            return 1;
        }
        return 0;
    }
    else
    {
        chdir(getenv("HOME"));
        if(chdir(params->clean_args) == -1){
            perror("chdir");
            return 1;
        }
    }

    return 0;
}