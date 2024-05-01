# Program Functionality
 - A colon symbolizes the start of a smallsh prompt.
 - Support the characters '<', '>', and '&'
    * & as the last character in the prompt means to run the program asynchronously, and return control to the user.  A & anywhere else should be treated as regular text.
    * '>' and '<' for input and output files must be the last arguments.  There will be no arguments that come after them (other than &).  The input and output for the program are from the file not the user if the operators are used.  This must be done using `dup2()`
 - Must support command lines with a max length of **2048 chars and 512 args.**
 - No error checking needed
 - Any line that begins with a '#' should not be executed.
 - Blank line should do nothing.
 - "$$" should be expanded to the PID of smallsh.  This is the only string expansion that is required.

# Built-in Commands
 - For all built in commands, there is no need to support input output redirection.
 - No need to set exit status
 - Ignores trailing '&'
 - Exit
    - Kills children then terminates itself.
 - cd
    - With no parameters: changed CWD to HOME env var
    - With 1 arg: path to change CWD to.  Support absolute and relative paths
 - status
    - If this is run before any foreground command is run, return 0
    - The three built in commands do not count as foreground commands.
 - Others
    - For any non-built in command, run for the appropriate variant of exec.
    - Your shell should use the PATH variable to look for non-built in commands, and it should allow shell scripts to be executed
    - If a command fails because the shell could not find the command to run, then the shell will print an error message and set the exit status to 1
    - A child process must terminate after running a command (whether the command is successful or it fails).

# Background Commands
 - Control must be returned to the user for non built-in functions that have the trailing '&'.
 - Print the PID of the background process when it starts.
 - If the user doesn't redirect the standard input for a background command, then standard input should be redirected to /dev/null
 - If the user doesn't redirect the standard output for a background command, then standard output should be redirected to /dev/null

# CTRL + C
 - smallsh must ignore CTRL + C.
 - Any background processes must ignore it.
 - Only current foreground processes must terminate.
 - A child running as a foreground process must terminate itself when it receives SIGINT
    - The parent must not attempt to terminate the foreground child process; instead the foreground child (if any) must terminate itself on receipt of this signal.
    - If a child foreground process is killed by a signal, the parent must immediately print out the number of the signal that killed it's foreground child process (see the example) before prompting the user for the next command.

# CTRL + Z
 - Toggles between allowing background execution and not.
    - Ignore trailing '&' or not.

# Hints
 - Use `fflush()` after each and every time text is output.
 - Do built ins first. 
 - Use `getcwd()` for getting CWD
 - Be careful which version of `exec()` is used. `execlp()` or `execvp()` is recommended.
 - `printf()` family of functions is NOT reentrant. This means if ctrl+z or crtl+c will interfere with execution.  