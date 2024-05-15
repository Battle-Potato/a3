#pragma once

typedef struct s_exec_params{
    int length;
    int background;
    char* input_file;
    char* output_file; 
    char* clean_args;  
} exec_params;