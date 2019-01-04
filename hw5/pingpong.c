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
 * Input:   This program will not take any argument inputs but this program will invoke
 *			the signal pingpong match between programs ping.c and pong.c. PLEASE READ README
 *			FILE.
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
#include<signal.h>

const char* stoargeFile = "mypid.txt";
long int ping;			//to hold ping.c's PID
long int pong;			//to hold ping.c's PID

void addPID(const char* file_name)		//this function will add the current process's PID to storage file
{
  	FILE* file = fopen (file_name, "a");	//we use append flag in order not delete the other PIDs
  	
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
		if(i == 0)			//first stored process ID
		{
			ping = pid;
		}
		else if(i == 1)		//second stored process ID		
		{
			pong = pid;
		}
	}
	
}

void my_handler(int signum)		//handler for signal catching
{
	if(signum == SIGUSR1)			//signal recieved from ping
	{
		printf("Ping\n");
	}	
	else if(signum == SIGUSR2)		//signal revieved from pong
	{
		printf("Pong\n");
	}
}

int main(int argc, char *argv[]){
	
	addPID(stoargeFile);		//add PID to storage text file
	readPIDs(stoargeFile);		//read PID's from storage
	
	sigset_t maskall, maskold;	//signal sets
	
	struct sigaction myAction;			//struct to define handler
	myAction.sa_handler = my_handler;
	myAction.sa_flags = 0;
	
	if(sigemptyset(&myAction.sa_mask) == -1)	//initialize signal mask 
	{
		perror("Failed to initials signalset");
		return -1;
	}
	
	sigfillset(&maskall);							//fill mask with all signals
	sigfillset(&myAction.sa_mask);					//fill our structure's mask with all signals
	sigdelset(&myAction.sa_mask, SIGUSR1);			//delete SIGUSR1 from the set
	sigdelset(&myAction.sa_mask, SIGINT);			//delete SIGINT from set to make sure we can kill our processes
	sigprocmask(SIG_SETMASK, &maskall, &maskold); 	//have all signals blocking
	
	if(kill(ping, SIGUSR1) == -1)		//send a SIGUSR1 to invoke ping process
	{
		perror("Failed to send the SIGUSR1 signal to PING");
		return -1;
	}
	
	if(kill(pong, SIGUSR1) == -1) 		//send a SIGUSR1 to invoke pong process
	{
		perror("Failed to send the SIGUSR1 signal to PONG");
		return -1;
	}
	
	for(;;)
	{
		if(sigaction(SIGUSR1, &myAction, NULL) == -1)			//Associate SIGUSR1 with our handler 
		{
			perror("Failed to associate SIGUSR1 with handler");
		}
		sigsuspend(&myAction.sa_mask);							//block all signals except those we deleted from the set (SIGUSR1 & SIGINT)
		sigaddset(&myAction.sa_mask, SIGUSR1);					//add SIGUSR1 back to the list of blocked signals
		
		if(sigaction(SIGUSR2, &myAction, NULL) == -1)			//Associate SIGUSR2 with our handler 
		{
			perror("Failed to associate SIGUSR2 with handler");
		}
		
		sigdelset(&myAction.sa_mask, SIGUSR2);					//delete SIGUSR2 from the set so that it is not blocked				
		
		sigsuspend(&myAction.sa_mask);							//block all signals except those we deleted from the set (SIGUSR2 & SIGINT)
		
		sigaddset(&myAction.sa_mask, SIGUSR2);					//add SIGUSR2 back to the list of blocked signals
		sigdelset(&myAction.sa_mask, SIGUSR1);					//delete SIGUSR1 from the set so that it is not blocked again
	}
	
	return 0;
}













