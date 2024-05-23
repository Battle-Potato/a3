/*
Assignment: smallsh
File: cd.c
Author: Tristan Vosburg
Date: 5/22/2024
Description: Changes directory based on the arguments given.
                If no arguments are given, changes to the home directory.
                If an argument is given, changes to that directory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cd.h"

int cd(exec_params* params){
    //if no arguments are given, change to the home directory
    if(params->length == 0){
        chdir(getenv("HOME"));
        return 1;
    }
    //otherwise, change to the directory specified
    if(chdir(params->clean_args) == -1){
        perror("chdir");
        return 1;
    }

    return 0;
}