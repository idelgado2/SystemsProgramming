/****************************************************************************************
 * Systems Programming (HomeWork_2)
 * Isaac Delgado
 * COSC_4348
 * 2/22/2017
 * Purpose: This program will execute a shell by executing commands thorugh
 *          a child process and utilizing the execvp()function.
 *
 * Input:   The input of the program will be shell commands (ex. ls,
 *          ls -l, cal, ps -ael, etc.) and to terminate the shell simply type 
 *			"quit" (without quotes).
 *
 * Output:  The output of this program will be the execution of the specific Unix system
 *          commands
 ***************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 80 				//80 chars per line, per command

int makeargv(char **MyArgument, char *MyCommand, char *delimiter){
    int i = 0;
    char *temp = NULL;
    temp = strtok(MyCommand, delimiter);	//extract inital token from user's input
    while(temp != NULL){
        MyArgument[i] = temp;				//append individual token it argument array of tokens
        i++;
        temp = strtok(NULL, delimiter);		//fetch next token
    }
    
    MyArgument[i] = NULL;			//append NULL at the end of Argument array, needs this for execvp function
    
    return i;						//return number of arguments in argument array
}

int main(int argc, char *argv[]){
    
    char delim[] = " \n";			//delimiter --- characters to strip from user input
    int numberOfTokens = 0;			//number of arguments passed by user
    
    char *myargv[MAX_LINE/2 + 1];	//character array of character arrays to hold user's ARGUMENTS
    char command[MAX_LINE];			//character array to hold direct input from user
    
    pid_t pid;			//process ID data type
    
    while(1){				//while true -- infinite loop
        printf("Shell>");
        fflush(stdout);
        
        if(fgets(command, sizeof(command), stdin) == NULL){		//get user input
            perror("Failed to get User Input");
        } 
        
        if((numberOfTokens = makeargv(&myargv, command, delim)) < 0){		//transform user's input into an argument array of tokens
            perror("Failed to Construct an Argument Array");
        }else{
        	
        	if(strcmp(command, "quit") == 0){	//Terminate shell if user inputs 'quit', check before fork to avoid execcesive fork
        		break;
    		}
        
        	pid = fork();					//Create a child process to run command given by user
        	
        	if(pid < 0){					//If Failed to fork a child process, error
        		perror("Failed to Fork");
        		return -1;
        	}
        	if(pid == 0){				/**child process**/
        		if (execvp(*myargv, myargv) == -1) {			//run system call given by User
            		perror("Failed to Execute System Command");
        		}
        		break;						//let child process break from while loop to return
        	}else{						/**parent process**/
        		wait(NULL);					//Parent will wait for child to finish
        	}
        }
    }
    
	return 0;
}
