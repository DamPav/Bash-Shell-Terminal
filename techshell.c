// Name(s): Christian Smith, Damjan Pavlovic
// Description: Creates a shell that takes inputs and executes commands such as cd and ls.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

//////// Some function ideas: ////////////
// Note: Some code is reflected in main that represents these functions,
// but it is up to you to determine how you want to organize your code.

/* 
    A function that causes the prompt to 
    display in the terminal
*/

void displayPrompt()
{
    char cwd[PATH_MAX]; //gathers an array of chars that is the current working directory

    if (getcwd(cwd, sizeof(cwd)) != NULL) //if cwd isn't nothing
    {
        printf("%s$ ", cwd); //prints cwd string
    }
    else
    {
        perror("getcwd"); //prints error "getcwd"
    }

    fflush(stdout); //prints the output immediately into terminal
}


/*
    A function that takes input from the user.
    It may return return the input to the calling statement or 
    store it at some memory location using a pointer.
*/ 
char* getInput(){ // * is pointer that gets an input string
    char buffer[1024]; //array of empty characters
    char* input; //pointer to char
    //call fgets() stores char array buffer as stdin (keyboard input)
    if(fgets(buffer, sizeof(buffer), stdin) == NULL){ // if the buffer array equals NULL, return NULL
        return NULL;
    }
    //strcspn returns number that specified character is located at
    buffer[strcspn(buffer, "\n")] = '\0'; //replaces "\n" with '\0' (NULL)
    input = malloc(strlen(buffer) + 1); //makes input pointer equal the address of malloc
    if(input == NULL){ //if input address == NULL
        perror("malloc error"); //show error
        exit(1); //exit
    }

    strcpy(input, buffer); //copies buffer array string into input (has smaller size)

    return input; //returns input array
}

/*
    A function that parses through the user input.
    Consider having this function return a struct that stores vital
    information about the parsed instruction such as:
    - The command itself
    - The arguments that come after the command 
        Hint: When formatting your data, 
        look into execvp and how it takes in args.
    - Information about if a redirect was detected such as >, <, or |
    - Information about whether or not a new file 
        needs to be created and what that filename may be.
    

    Some helpful functions when doing this come from string.h and stdlib.h, such as
    strtok, strcmp, strcpy, calloc, malloc, realloc, free, and more

    Be sure to consider/test for situations when a backslash is used to escape the space char
    and when quotes are used to group together various tokens.
*/

typedef struct ShellCommand{
    char* command; //pointer to command like cd
    char** args; //argument array for first command
    char* inputFile; //after <
    char* outputFile; // after >
    int append; //tells if there is > or >> etc.
} ShellCommand;

ShellCommand parseInput(char* input){
    ShellCommand Sc;

    //Initialize variables
    Sc.command = NULL;
    Sc.args = NULL;
    Sc.inputFile = NULL;
    Sc.outputFile = NULL;
    Sc.append = 0;
    char* tokenInput = strdup(input);
    char* originalInput = strdup(input);

    int capacity = 0; //starts capacity counting
    char* incrementToken = strtok(tokenInput, " "); 
    while(incrementToken != NULL){ //increments to see capacity size
        capacity++;
        incrementToken = strtok(NULL, " ");
    }
    Sc.args = malloc((capacity+1) * sizeof(char*)); //sets memory size for args
    if(!Sc.args){
        perror("malloc");
        exit(1);
    }
    int i = 0;
    char* token = strtok(originalInput, " "); //gets the first token
    if(token != NULL) {Sc.command = token;} //sets token value as command if there is token
    while(token != NULL){
        if(strcmp(token, "<") != 0 && strcmp(token, ">") != 0 && strcmp(token, "|") != 0){
            Sc.args[i++] = token; //appends token value to args
        }
        if(strcmp(token, "<") == 0){ //if token is < make next equal inputFile
            Sc.inputFile = strtok(NULL, " ");
        }
        else if(strcmp(token, ">") == 0){ //if token is > make next equal outputFile
            Sc.outputFile = strtok(NULL, " ");
            Sc.append = 0; //redirects output into file
        }
        else if(strcmp(token, ">>") == 0){ //if token is >> make next equal outputFile
            Sc.outputFile = strtok(NULL, " ");
            Sc.append = 1; //appends existing file
        }
        else if(strcmp(token, "|") == 0){ //skips over pipes
            perror("No Pipes allowed"); 
        token = strtok(NULL, " ");
    }
    return Sc;
}


/*
    A function that executes the command. 
    This function might take in a struct that represents the shell command.

    Be sure to consider each of the following:
    1. The execvp() function. 
        This can execute commands that already exist, that is, 
        you don't need to recreate the functionality of 
        the commands on your computer, just the shell.
        Keep in mind that execvp takes over the current process.
    2. The fork() function. 
        This can create a process for execvp to take over.
    3. cd is not a command like ls and mkdir. 
        cd is a toold provided by the shell, 
        so you WILL need to recreate the functionality of cd.
    4. Be sure to handle standard output redirect and standard input redirects here 
        That is, there symbols: > and <. 
        Pipe isn't required but could be a nice addition.
*/
void executeCommand(ShellCommand command){
    if(command.command == NULL){ //if there is no command, return nothing
        return;
    }
    //built in cd
    if(strcmp(command.command, "cd") == 0){
        if(command.args[1] == NULL){
            fprintf(stderr, "cd: expected argument\n");
        }
        else{
            if(chdir(command.args[1]) != 0){
                perror("cd fail");
            }
        }
        return;
    }
    //fork
    pid_t pid = fork();
    if(pid < 0){
        perror("fork failed");
        return;
    }

    //child process
    if(pid == 0){
        if(command.inputFile != NULL){
            int fd = open(command.inputFile, O_RDONLY);
            if(fd < 0){
                perror("input redirection failed");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if(command.outputFile != NULL)
        {
            int fd;
            if(command.append){
                fd = open(command.outputFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
            }
            else{
                fd = open(command.outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if(fd < 0){
                perror("output redirection failed");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execvp(command.command, command.args);
        perror("exec failed");
        exit(1);
        }
        else{
            wait(NULL);
        }
    }
}

int main() // MAIN
{
	char* input;
	struct ShellCommand command;
		
	// repeatedly prompt the user for input
    for (;;)
	{
        // display the prompt
        displayPrompt();

	    // get the user's input
	    input = getInput();
	    
	    // parse the command line
	    command = parseInput(input);
	    
	    // execute the command
        /*printf("You typed: %s\n", input);
        if(command.command != NULL){
            printf("Command: %s\n", command.command);
            printf("Args:");
            for (int i = 0; command.args[i] != NULL; i++) {
                printf(" %s", command.args[i]);
            }
            printf("\n");
            printf("Output: %s\n", command.outputFile);
            printf("Input: %s\n", command.inputFile);

        }
        free(input);
        */
	    executeCommand(command);
	}
	exit(0);
}

