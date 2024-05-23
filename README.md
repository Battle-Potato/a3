# Execution Instruction
To compile: `$ make`

To run: `$ smallsh`

# Info
- Project Name: smallsh
- Author: Tristan Vosburg
- Description: A small shell with limited built in features including: 
    - 3 built in commands
        - `cd`
            - Changes directory to home with no parameters
            - Changes directory to appropriate directory with parameters
        - `status`
            - Returns the exit status or terminating signal of the last foreground process
        - `exit`
            - Exits the shell after ending all background processes
    - File IO redirection
    - Background Processes
    - Signal Handling for `SIGINT` and `SIGTSTP`