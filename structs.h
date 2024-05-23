/*
Assignment: smallsh
File: structs.h
Author: Tristan Vosburg
Date: 5/22/2024
Description: The params struct for the shell.
*/

#pragma once

typedef struct s_exec_params{
    int length;
    int background;
    char* input_file;
    char* output_file; 
    char* clean_args;  
} exec_params;