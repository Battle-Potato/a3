#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "status.h"

int status(int i_status, int sig){
    if(sig == 0){
        printf("Exit value: %d\n", i_status);
    } else {
        printf("Terminated by signal: %d\n", sig);
    }
    return 0;
}