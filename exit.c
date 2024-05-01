#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exit.h"

int smallsh_exit(char* path){
    printf("exit: %s\n", path);
    return 1;
}