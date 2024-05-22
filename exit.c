#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "exit.h"

int smallsh_exit(pid_t* background_processes){
    int i = 0;
    for(i = 0; i < 52; i++){
        if(background_processes[i] != 0){
            kill(background_processes[i], SIGKILL);
        }

    }

    return 0;
}