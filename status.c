/*
Assignment: smallsh
File: status.c
Author: Tristan Vosburg
Date: 5/22/2024
Description: prints the exit status of a process or the signal that terminated it.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "status.h"

int status(int i_status, int sig){
    //if the process exited normally, print the exit value
    if(sig == 0){
        printf("Exit value: %d\n", i_status);
    }
    //otherwise the process was terminated by a signal, print
    else {
        printf("Terminated by signal: %d\n", sig);
    }
    return 0;
}