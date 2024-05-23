/*
Assignment: smallsh
File: exit.c
Author: Tristan Vosburg
Date: 5/22/2024
Description: Exits the shell, killing all background processes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "exit.h"

int smallsh_exit(pid_t* background_processes){
    //kill all background processes
    int i = 0;
    for(i = 0; i < 52; i++){
        if(background_processes[i] != 0){
            kill(background_processes[i], SIGKILL);
        }

    }

    return 0;
}