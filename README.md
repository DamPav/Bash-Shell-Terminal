# Bash-Shell-Terminal

// Names: Damjan Pavlovic & Christian Smith

// Responsibilites: We worked together in person on each and every feature of the code stimultaneusly , so I'm not exactly sure how else to answer

// Description: This program implements a simplified Unix-style shell similar to Bash. It repeatedly displays a command prompt, accepts user input from the terminal, and interprets that input as a command to be executed. The program tokenizes the input string into individual components (the command and its arguments), parses the tokens to detect special operators such as input/output redirection symbols (<, >), and organizes the data into a struct. It then uses fork() function to create a child process and calls execvp() function to execute the requested command with the parsed arguments. If redirection is detected, the program adjusts the appropriate file descriptors before execution. Built in commands such as cd are handled directly by the shell rather than passed to execvp() function.
