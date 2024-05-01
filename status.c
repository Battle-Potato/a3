#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "status.h"

int status(exec_params* params){
    printf("Background processes: %d\n", params->background);
    printf("Input file: %s\n", params->input_file);
    printf("Output file: %s\n", params->output_file);
    printf("Clean args: %s\n", params->clean_args);
    return 0;
}