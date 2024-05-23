#pragma once
#include "structs.h"

void reset_args(char*, char*, char*);

int get_args(char*, char*, char*);

exec_params* set_params(char*, int);

void external_command(char*, exec_params*, int*);

char** parse_input(exec_params*, char*);

int redirect_input(exec_params*);

int redirect_output(exec_params*);

void handle_background();

void clean_terminated();

void shell();

void expand_input(char*);

void kill_zombies();

int one_loop(char*, char*, char*);

void handler_setup();

void handle_SIGTSTP(int);

void handle_SIGINT(int);