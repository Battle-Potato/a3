#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cd.h"

int cd(exec_params* params){

    if(params->length == 0){
        chdir(getenv("HOME"));
        return 1;
    }
    if(chdir(params->clean_args) == -1){
        perror("chdir");
        return 1;
    }

    return 0;
}