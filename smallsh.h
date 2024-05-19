#pragma once
#include "structs.h"


int get_args(char*, char*, char*);

exec_params* parse_args(char*, int);

void external_command(char*, exec_params*, int*);

char** parse_input(exec_params*);

int redirect_input(char*);

int redirect_output(char*);

void shell();

void expand_input(char*);