#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>           
#include "smallsh.h"
#include "cd.h"
#include "exit.h"
#include "status.h"
#include "structs.h"


#define BUFFER_SIZE 2048

pid_t bg_processes[52] = {0};
pid_t fg_process;
pid_t shell_pid;
int i_status = 0;
int sig = 0;
int background_lock = 0;
int change_lock = 0;
int signal_caught = 0;

int main(){

    handler_setup();
    shell_pid = getpid();
    // printf("Shell PID: %d\n", shell_pid);
    shell();
    return 0;
}

void shell(){
    char* input = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char** input_adr = &input;
    char* command = (char*)calloc(BUFFER_SIZE, sizeof(char));
    char* args = (char*)calloc(BUFFER_SIZE, sizeof(char));
    size_t max_size = BUFFER_SIZE;

    while(1){
        reset_args(input, command, args);
        if(change_lock == 1){
            change_lock = 0;
            if(background_lock == 0)
                write(1, "Exiting foreground-only mode\n", 30);
            else
                write(1, "Entering foreground-only mode (& is now ignored)\n", 49);
        }

        if(signal_caught == 1){
            // printf("Caught signal %d\n", sig);
            signal_caught = 0;
            continue;
        }

        kill_zombies();
       
        //these need to be converted later to a non printf version reference 4/24 lecture
        // char* path = getcwd(NULL, 0); //TODO: dont print this in final version
        // printf("%s", path);
        printf(" : ");
        fflush(stdout);
        fgets(input, BUFFER_SIZE, stdin);        input_adr = &input;

        if(signal_caught == 1){
            // printf("Caught signal %d\n", sig);
            signal_caught = 0;
            continue;
        }

        // if (input[0] == '\n' || input[0] == '#' || input[0] == '\0')
        // {
        //     continue;
        // }
        

        expand_input(input);    
        int args_len = get_args(input, command, args);

        exec_params* params = parse_args(args, args_len);

        if(strcmp(command, "cd") == 0){
            cd(params);
        } else if(strcmp(command, "exit") == 0){
            smallsh_exit(bg_processes);
            // free(path);
            free(params);
            free(input);
            free(command);
            free(args);
            return;
        } else if(strcmp(command, "status") == 0){
            status(i_status, sig);
            sig = 0;
        } else if(strcmp(command, "") == 0){
            continue;
        } else if(strcmp(command, "#") == 0){
            continue;
        } else {
            external_command(command, params, &i_status);
        }
        free(params);
        // free(path);
    }
}

void reset_args(char* input, char* command, char* args){

    for(int i = 0; i < BUFFER_SIZE; i++){
        input[i] = '\0';
        command[i] = '\0';
        args[i] = '\0';
    }
}

void kill_zombies(){
    // printf("Checking for zombies\n");

    int i = 0;
    while(bg_processes[i] != 0){
        int status;
        // printf("Checking for zombie process %d\n", bg_processes[i]);
        pid_t pid = waitpid(bg_processes[i], &status, WNOHANG);
        if(pid > 0){
            if(WIFEXITED(status)){
                printf("Background process %d exited with status %d\n", bg_processes[i], WEXITSTATUS(status));
            }
            else if(WIFSIGNALED(status)){
                printf("Background process %d terminated by signal %d\n", bg_processes[i], WTERMSIG(status));
            }
            bg_processes[i] = 0;
        }
        i++;
    }
}

int get_args(char* input, char* command, char* args){
    if(input[0] == '#' || input[0] == '\n'){
        command[0] = '\0';
        return 0;
    }
    //find first space or \n
    int i = 0;
    while(input[i] != ' ' && input[i] != '\n'){
        i++;
    }

    //copy command
    strncpy(command, input, i);
    command[i] = '\0';

    if(input[i] == '\n'){
        args[0] = '\0';
        return 0;
    }
    i++;
    strcpy(args, input + i);
    int len = strlen(args);
    args[--len] = '\0';
    return len - 1;
}

exec_params* parse_args(char* args, int length){
    exec_params* params = (exec_params*)malloc(sizeof(exec_params));
    params->background = 0;
    params->input_file = NULL;
    params->output_file = NULL;
    params->clean_args = args;
    params->length = length;

    //parse args
    if (length == 0){
        return params;
    }
    if (args[length] == '&'){
        if(background_lock == 0)
            params->background = 1;
        args[length] = '\0';
        length--;
    }

    //get input char
    params->input_file = strchr(args, '<');

    //get output char
    params->output_file = strchr(args, '>');

    //remove trailing spaces
    char* seeker = NULL;


    if(params->input_file != NULL){
        *params->input_file = '\0';
        params->input_file++;
        while(*params->input_file == ' '){
            params->input_file++;
        }
        seeker = params->input_file;
        while(*seeker != ' ' && *seeker != '\0'){
            seeker++;
        }
        *seeker = '\0';
    }
    if(params->output_file != NULL){
        *params->output_file = '\0';
        params->output_file++;
        while(*params->output_file == ' '){
            params->output_file++;
        }
        seeker = params->output_file;
        while(*seeker != ' ' && *seeker != '\0'){
            seeker++;
        }
        *seeker = '\0';
    }

    return params;
}

int redirect_input(exec_params* params){
    int fd = -2;
    if(params->background == 1 && params->input_file == NULL )
        fd = open("/dev/null", O_RDONLY, 0644);
    else if(params->input_file != NULL)
        fd = open(params->input_file, O_RDONLY);
    if(fd == -1){
        perror("open");
        return 1;
    }
    if(fd == -2){
        return 0;
    }
    int new_fd = dup2(fd, 0);
    if(new_fd == -1){
        perror("dup2");
        exit(1);
    }
    close(fd);
    return 0;
}

int redirect_output(exec_params* params){
    int fd = -2;
    if(params->background == 1 && params->output_file == NULL)
        fd = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    else if(params->output_file != NULL)
        fd = open(params->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
        perror("open");
        return 1;
    }
    if(fd == -2){
        return 0;
    }
    int new_fd = dup2(fd, 1);
    if(new_fd == -1){
        perror("dup2");
        exit(1);
    }
    close(fd);
    return 0;
}

void external_command(char* command, exec_params* params, int* status){
    char** args = parse_input(params);
    pid_t child_pid = fork();
    if(child_pid == -1){
        perror("fork");
        exit(1);
    }
    if(child_pid == 0){
        //child process


        //redirect input
        int input_status = 0;
        input_status = redirect_input(params);

        //redirect output
        int output_status = 0;
        output_status = redirect_output(params);

        if(output_status == 1){
            write(1, "Error redirecting output\n", 26);
            exit(1);
        }
        else if (input_status == 1){
            write(1, "Error redirecting input\n", 25);
            exit(1);
        }
        else{
            struct sigaction SIGINT_ignore, SIGTSTP_ignore = {0};

            if(params->background == 1){

                SIGINT_ignore.sa_handler = SIG_IGN;
                sigfillset(&SIGINT_ignore.sa_mask);
                SIGINT_ignore.sa_flags = 0;
                sigaction(SIGINT, &SIGINT_ignore, NULL);
            }
            SIGTSTP_ignore.sa_handler = SIG_IGN;
            sigfillset(&SIGTSTP_ignore.sa_mask);
            SIGTSTP_ignore.sa_flags = 0;
            sigaction(SIGTSTP, &SIGTSTP_ignore, NULL);
            execvp(command, args);
            write(1, "Invalid command\n", 16);
            exit(1);
        }
    }
    else{
        //parent process
        if(params->background == 0){
            //TODO: prevent build up of zombie processes
            fg_process = child_pid;
            waitpid(child_pid, status, 0);
            *status = WEXITSTATUS(*status);
            fg_process = 0;
        }
        else{
            int i = 0;
            while(bg_processes[i] != 0){
                i++;
            }
            bg_processes[i] = child_pid;
            printf("Background process started with PID %d\n", child_pid);
        }
    }
}

char** parse_input(exec_params* params){
    char** args = (char**)calloc(52, sizeof(char*));
    char* token = strtok(params->clean_args, " ");
    args[0] = params->clean_args;
    int i = 1;
    while(token != NULL){
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    args[i] = NULL;
    return args;
}


void expand_input(char* input){
    int i = 0;
    //TODO: REMOVE INTERNAL '&' AS WILL CONFUSE BASH (maybe ignore?)
    while(1){
        if(input[i] == '\n')
            break;
        if(input[i] == '$'){
            //check if & is at the end of the line
            if(input[i + 1] == '\n'){
                break;
            }
            //check if & is followed by &
            if(input[i + 1] == '$'){
                input[i] = '\0';
                input[i + 1] = '\0';
                //get pid length
                char pid[10];
                sprintf(pid, "%d", shell_pid);
                int pid_len = strlen(pid);
                char* next_char = input + i + 2;
                //shift next_char and everything after to the right by pid_len - 2
                char* seeker = next_char;
                while(*seeker != '\n'){
                    seeker++;
                }
                while(seeker != next_char - 1){
                    char* destination = seeker + pid_len - 2;
                    *destination = *seeker;
                    seeker--;
                }
                //copy pid into input
                for(int j = 0; j < pid_len; j++){
                    input[i + j] = pid[j];
                }
            }
        }
        i++;
    }
}

void handler_setup(){
    struct sigaction SIGTSTP_action, SIGINT_action = {0};

    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);   

    SIGINT_action.sa_handler = handle_SIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);
}

void handle_SIGINT(int signo){
    // char* message = "Caught SIGINT\n\n";
    // write(1, message, 15);
    signal_caught = 1;
    sig = signo;
    // write(1, message, 15);
}


void handle_SIGTSTP(int signo){
    // char* message = "Caught SIGTSTP\n\n";
    // write(1, message, 18);
    signal_caught = 1;
    change_lock = 1;
    if(background_lock == 0){
        background_lock = 1;
    }
    else{
        background_lock = 0;
    }
    waitpid(fg_process, &i_status, 0);
    fg_process = 0;
    // write(1, message, 18);
}