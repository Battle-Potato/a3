#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "status.h"

int status(char* path){
    printf("status: %s\n", path);
    return 1;
}