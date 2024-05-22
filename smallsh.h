#pragma once
#include "structs.h"

void reset_args(char*, char*, char*);

int get_args(char*, char*, char*);

exec_params* parse_args(char*, int);

void external_command(char*, exec_params*, int*);

char** parse_input(exec_params*);

int redirect_input(exec_params*);

int redirect_output(exec_params*);

void shell();

void expand_input(char*);

void kill_zombies();


void handler_setup();

void handle_SIGTSTP(int);

void handle_SIGINT(int);