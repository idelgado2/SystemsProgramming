/****************************************************************************************
 * Systems Programming (HomeWork_5)
 * Isaac Delgado
 * COSC_4348
 * 3/7/2017
 * Purpose: This is one of three programs in a pingpong scheme between programs sending
 *			signals to each other. There three programs are (ping.c, pong.c, and pingpong.c)
 *			ping and pong will send signals to each other in a ping pong game manner, while
 *			at the same time sending signals to pingpong.c to print out wither "ping" or 
 *			"pong accordingly". ----Please read README.txt file for execution directions
 *
 * Input:   This program will not take any argument inputs but must run in the background
 *			PLEASE SEE README file execution directions
 *
 * Output:  This porgram will not have an explicit output, but will be sending a signal
 *			to pingpong to print out "ping", and sending a signal to pong.c to invoke it's
 *			functionality.
 ***************************************************************************************/
 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

const char* stoargeFile = "mypid.txt";
long int pong;			//to hold pong.c's PID
long int pingpong;		//to hold ping.c's PID

void addPID(const char* file_name)		//this function will add the current process's PID to storage file
{
  	FILE* file = fopen (file_name, "w");	//important that we write, so that we can create a blank file after multiple calls
  	
  	if(!file)
  		printf("failed to open file");
  		
	fprintf(file, "%ld\n", (long)getpid());
  	fclose (file);        
}

void readPIDs(const char* file_name)	//This function will read all PID's from storage and save them accourdingly
{
	FILE* file = fopen(file_name, "r");
	
	if(!file)
		printf("failed to open file");
	
	long int pid;
	int i;
	for(i = 0; !feof (file); i++)
	{
		fscanf (file, "%ld", &pid);
		if(i == 1)					//second stored process ID
		{
			pong = pid;
		}
		else if(i == 2)				//third stored process ID		
		{
			pingpong = pid;
		}
	}
	
}

void my_handler(int signum)			//handler for signal catching
{
	if (signum == SIGUSR1)
	{
		readPIDs(stoargeFile);		//read pids from storage file
	}
	else if(signum == SIGUSR2)
	{
		if(kill(pingpong, SIGUSR1) == -1)	//send signal to pingpong
		{
			perror("Failed to send SIGUSR1 to pingpong or pingpong process was interrupted");
			if(kill(pong, SIGINT) == -1)		//kill pong process to terminate program
				perror("Failed to kill pong process OR ping Process is already dead");
			if (raise(SIGINT) != 0)				//kill this process to terminate program
   				perror("Failed to raise SIGUSR1");
		}
		
		if(kill(pong, SIGUSR2) == -1)		//send signal to pong
			perror("Failed to send SIGUSR2 to pong");
	}
}

int main(int argc, char *argv[])
{	
	addPID(stoargeFile);			//add PID to storage text file
	sigset_t maskall, maskold;		//signal sets
	
	struct sigaction myAction;			//struct to define handler
	myAction.sa_handler = my_handler;
	myAction.sa_flags = 0;
	
	if((sigemptyset(&myAction.sa_mask) == -1) || (sigaction(SIGUSR1, &myAction, NULL)) == -1)	//intialize signal mask and associate SIGUSR1 with handler
	{
		perror("Failed to install SIGUAR1 signal handler");
		return -1;
	}
	
	sigfillset(&myAction.sa_mask);					//fill our structure's mask with all signals
	sigfillset(&maskall);							//fill mask with all signals
	sigdelset(&myAction.sa_mask, SIGUSR1);			//delete SIGUSR1 from the set
	sigdelset(&myAction.sa_mask, SIGINT);			//delete SIGINT from set to make sure we can kill our processes
	sigprocmask(SIG_BLOCK, &maskall, &maskold); 	//have all signals blocking

	sigsuspend(&myAction.sa_mask);					//block all signals except those we deleted from the set (SIGUSR1 & SIGINT)
	
	sigprocmask(SIG_BLOCK, &maskold, NULL);			//reset blocked signals

	
	if(sigaction(SIGUSR2, &myAction, NULL) == -1)	//Associate SIGUSR2 with our handler now
	{
		perror("Failed to associate SIGUAR2 with signal handler");
		return -1;
	}
	
	if(kill(pingpong, SIGUSR1) == -1)				//send signal to pingpong to print out inital "ping"
	{
		perror("Failed to send SIGUSR1 to pingpong");
		return -1;
	}
	
	if(kill(pong, SIGUSR2) == -1)					//serve the ball to pong (send signal to pong)
	{
		perror("Failed to send SIGUSR1 to pingpong");
		return -1;
	}
	
	sigaddset(&myAction.sa_mask, SIGUSR1);			//make sure SIGUSR1 is blocked now
	sigdelset(&myAction.sa_mask, SIGUSR2);			//delete SIGUSR2 (SIGUSR2 and SIGINT are not blocked)
	
	for(;;)
	{
		sigsuspend(&myAction.sa_mask);				//block all signals except those we deleted from the set (SIGUSR2 & SIGINT)
	}
	
	return 0;
}















