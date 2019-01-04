/****************************************************************************************
 * Systems Programming (HomeWork_7)
 * Isaac Delgado
 * COSC_4348
 * 4/28/2017
 * Purpose: The purpose of the this program is to determine the prime numbers through a range
 *          that is predetermined through command line arguments. This program utilizez the
 *          pthread library to utilize threads to determine the listing of primes. Mutex locks
 *          are used to manage these threads accordingly. In addition this program will be using
 *			IPC pipes to send the total number of primes to the parent process. The ultimate
 *			parent process will add all these primes for a total.
 *
 * Input:   the input for this program are four arguments including the executatble as follows
 *          ./executable [begging range][end range][# of processes][# of threads per process].
 *           example exection is as follows ./prime 1 1000 3 2 --- This execution specifies
 *			there be 3 processes (INCLUDEING THE MAIN RPPROCESS) and each process (EXCLUDING MAIN PROCESS)
 *			will have 2 threads. The main thread will take the totals and create the real Total.
 *
 * Output: 	The result of this execution will be a lisiting of output of total primes per process.
 *			THe last output will be the main process displayin the total primes output of all processes
 *			example ouput: 
 *				The sum of the prime numbers computed by process pid: 22507 is :84
 *				The sum of the prime numbers computed by process pid: 22510 is :72
 *				The total sum of the prime numbers in the range 1-1000 is:156
 *
 *          *IMPORTANT* ---- to compile on joker utilize -pthread option-
 *				example:
 *                  g++ -pthread primethread_delgado.cpp -o primethread
 ***************************************************************************************/
#include <sys/msg.h>    //for Message Queue
#include <pthread.h>    //Threads
#include <sys/stat.h>   //for IPC Permissions
#include <string.h>     // messages
#include <unistd.h>     //fork()
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <iostream>

#define PERM (S_IRUSR | S_IWUSR)
#define READ_END 0
#define WRITE_END 1
#define MAXLINE 1024
#define MAXSIZE 4096

using namespace std;

static int queueid;
static int globalerror = 0;         //global error to allow any thread to set error
static pthread_mutex_t errorlock = PTHREAD_MUTEX_INITIALIZER;               //mutex lock strictly for setting errors of threads
static pthread_mutex_t threadfunctionlock = PTHREAD_MUTEX_INITIALIZER;      //mutex lock for anipulating thread local varibales
static int processPrimeTotal = 0;       //total number of primes in given process

typedef struct {
    long mtype;
    char mtext[1];
} mymsg_t;

struct arg_struct {
    int BeginThreadPoint;
    int Range;
    int iteration;
};

int initqueue(int key);
int msgwrite(void *buf, int len);
int msgprintf(char *fmt, ...);
int remmsgqueue(void);
void threadMachine(int numberOfThreads);
void *threadFunction(void *argument);
bool isPrime(int value);
int geterror(int *error);       //thread error getting
int seterror(int error);        //thread error setting



void *msgreceiver(void *arg)
{
 pthread_exit((void *)0);
} // msgreceiver()



void *msgtransmitter(void *arg)
{
    pthread_exit((void *)0);
} // msgtransmitter()


//pthread_mutex_lock(&msgmutex);
//pthread_mutex_unlock(&msgmutex);

int main(int argc, char **argv)
{
    int numberOfProcesses = 0;
    int numberOfThreadsPerProcess = 0;
    int numberOfIntegers = 0;
    int beginRangePoint = 0;
    int beginRangePoint_Thread = 0;
    int rangForEachProcess = 0;
    int rangForEachThread = 0;
    int temp = 0; //to hold total primes for each process -- to total primes
    int TotalPrimes = 0;
    
    int error;
    int status;
    pid_t pid;
    int fd[2];
    
    if(argc != 5){				//command line arguments must be exactly 4 (this includes the executable - see header comments)
        perror("Incorret number of passed arguments");
        return 1;
    }
    
    numberOfProcesses = atoi(argv[3]);
    numberOfThreadsPerProcess = atoi(argv[4]);
    beginRangePoint = beginRangePoint_Thread = atoi(argv[1]);
    numberOfIntegers = (atoi(argv[2]) - atoi(argv[1]) + 1);   //get number of integers within the range specified
    
    pthread_t thr[numberOfThreadsPerProcess]; //create array of threads to be used per process
    pipe(fd);                       // create pipe descriptors
    
    if((numberOfIntegers % (numberOfProcesses - 1)) != 0){             //if range does not equally divide to number of threads
        perror("Uneven processes to range, please adjust process count-or-range to have equal ranges for all process(es)\n");
        return 1;
    }else if(((numberOfIntegers/(numberOfProcesses - 1)) % numberOfThreadsPerProcess) != 0){
        perror("Uneven threads to range, please adjust thread count-or-range to have equal ranges for all threads\n");
        return 1;
    }
    
    rangForEachProcess = (numberOfIntegers / (numberOfProcesses - 1));
    rangForEachThread = ((numberOfIntegers / (numberOfProcesses - 1))/numberOfThreadsPerProcess);
    
    for(int i = 0; i < (numberOfProcesses - 1); i++){             //parent process creates all child process -- FAN
        pid = fork();
        
        if(pid == -1){
            perror("Fork Failed");
            break;
        }
        if(pid == 0){              	//child process(es)
            struct arg_struct args;
            args.Range = rangForEachThread;
            
            beginRangePoint_Thread = ((rangForEachThread * numberOfThreadsPerProcess)*i); //set thread beign point accroding to process
            for (int k = 0; k < numberOfThreadsPerProcess; k++) {               //make 'numberOfThreadPerProcess' threads
                beginRangePoint_Thread += (rangForEachThread * k);
                args.BeginThreadPoint = beginRangePoint_Thread;
                args.iteration = k;
                if(pthread_create(&thr[k], NULL, threadFunction, (void*)&args)){
                    cout << argv[0] << " cannot make thread\n";
                    exit(1);
                }
                //sleep(2);
            }
            
            for(i = 1; i < numberOfThreadsPerProcess; i++){
                if ((error = pthread_join(thr[i], NULL))) {       //join all threads as they finish executing
                    fprintf(stderr, "Failed to join thread %d:%s\n", i, strerror(error));
                    return 1;
                }
            }
            
            printf("The sum of the prime numbers computed by process pid: %d is :%d\n", getpid(), processPrimeTotal);
            //close(fd[READ_END]);
            
            // send the value on the write-descriptor.
            write(fd[WRITE_END], &processPrimeTotal, sizeof(processPrimeTotal));
            
            // close the write descriptor
            //close(fd[WRITE_END]);
            
        /*send prime total for this process to message queue here*/
            
            
            return 0;
        }
        else{
            waitpid(pid, &status, 0);       //wait for last process child process to finish
            //close(fd[WRITE_END]);
            
            // now read the data (will block)
            read(fd[READ_END], &temp, sizeof(temp));
            TotalPrimes += temp;
            
            // close the read-descriptor
            //close(fd[READ_END]);

        }
    }
    
    waitpid(pid, &status, 0);       //wait for last process child process to finish
    
    cout << "The total sum of the prime numbers in the range " << argv[1] << "-" << argv[2] <<" is:" << TotalPrimes << endl;
    
    pthread_mutex_destroy(&threadfunctionlock);     //destroy our mutex locks
    
    return 0;

}

void *threadFunction(void *arguments){
    struct arg_struct *args = (struct arg_struct *)arguments;
    int localThreadCounter = 0;
    int error = 0;

    /***find total number of primes here***/
    int start = (args -> BeginThreadPoint);
    int end = (start + (args -> Range));
    while (start <= end) {
        if(isPrime(start)){
            localThreadCounter++;
        }
        start++;
    }
    
    if((error = pthread_mutex_lock(&threadfunctionlock)))           //look critical section
        seterror(error);
    /***update processprimetotal here**/
    processPrimeTotal += localThreadCounter;
    
    if((error = pthread_mutex_unlock(&threadfunctionlock)))         //unlock critical section
        seterror(error);
        return NULL;
}


int remmsgqueue(void) {
    return msgctl(queueid, IPC_RMID, NULL);
}

bool isPrime(int value){
    bool isPrime;                //initializing status to hold the True or false, prime or not
    int count = 0;
    
    for (int i = 1; i <= value; i++) {
        if ((value % i) == 0)
            count++;            // for loop counting the number of zeros, primes have only 2 zeros
    }
    if (count <= 2)
        isPrime = true;
    else
        isPrime = false;
    
    return isPrime;
}

int geterror(int *error){       //get the error for the specific thread
    int terror;             //local variable to hold error
    if((terror = pthread_mutex_lock(&errorlock)))
        return terror;
    *error = globalerror;
    return pthread_mutex_unlock(&errorlock);
}

int seterror(int error){        //get the error for the specific thread
    int terror;             //local variable to hold error
    if(!error)          //if no error - do not change the global variable
        return error;
    if((terror = pthread_mutex_lock(&errorlock)))
        return terror;
    if(!globalerror)
        globalerror = error;
    terror = pthread_mutex_unlock(&errorlock);
    return terror ? terror: error;      //if error in unlocking mutex then return mutex error else return the error originally passed
}