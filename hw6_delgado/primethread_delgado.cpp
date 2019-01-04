/****************************************************************************************
 * Systems Programming (HomeWork_6)
 * Isaac Delgado
 * COSC_4348
 * 4/19/2017
 * Purpose: The purpose of the this program is to determine the prime numbers through a range
 *          that is predetermined through command line arguments. This program utilizez the
 *          pthread library to utilize threads to determine the listing of primes. Mutex locks
 *          are used to manage these threads accordingly.
 *
 * Input:   the input for this program are four arguments including the executatble as follows
 *          ./executable [begging range][end range][desired amount of threads to use].
*           example exection is as follows ./primethread 1 1000 20
 *
 * Output: The output of this program will create two files. THere will be a "result_delgado.dat"
 *          this file will have the origianl listing of primes. (They are supposed to be in random
 *          order, according to when each thread was executed, however for some reason they are not)
 *          the second file will be "sorted_delgad.dat". This file will contain the same list after
 *          going through an Insert sort. Lastly this orded list will be displayed to the screen.
 *
 *          *important - if the files "result_Delgado.dat" and "sorted_delgado.dat" already exist
 *              the data will be appended to those files. Thus before executing make sure they
 *              are not existant in your files directory**
 *          *addtionally to compile on joker utilize -pthread option- in example
 *                  g++ -pthread primethread_delgado.cpp -o primethread
 ***************************************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fstream>
#include <unistd.h>

using namespace std;

struct nodeType{						//this is a struct for creating a linked list
    int info;
    nodeType *link;
};

class linklist{							//linked list class to encaspulate linked list functionality
public:
    linklist();                         //condtructor to intialize list
    void insert(const int&);
    void print() const;
    bool isEmpty() const;
    void linkedInsertionSort();         //function to sort linked list by Insertion sort
    void destroyList();
    int getLength();
    void writeToFile(const char* file_name);
    nodeType * HEAD();                  //returns the head of the list
private:
    nodeType *first;				//first position in our list
    nodeType *last;					//last position in our list
    int length;
};

static int globalerror = 0;         //global error to allow any thread to set error
static pthread_mutex_t errorlock = PTHREAD_MUTEX_INITIALIZER;               //mutex lock strictly for setting errors of threads
static pthread_mutex_t threadfunctionlock = PTHREAD_MUTEX_INITIALIZER;      //mutex lock for anipulating thread local varibales

int COUNTER = 1;                //counter to distinguish threads exection
int NUMTHREADS = 0;             //number of threads specified by the command line arguments
int THREADRANGE = 0;            //varibale to hold the number of integer each thread will hold

int geterror(int *error);       //thread error getting
int seterror(int error);        //thread error setting
bool isPrime(int value);
void *threadFunction(void *argument);    //thread function
void *lastThreadFunction(void *argument);

int main(int argc, const char * argv[]) {
    int error, i;
    int numinrange;

    if(argc != 4){				//command line arguments must be exactly 4 (this includes the executable - see header comments)
        perror("Incorret number of passed arguments");
        return 1;
    }
    
    NUMTHREADS = atoi(argv[3]);         //retrieve number of thread specified from command line argument
    printf("Number of threads requested is: %d\n", atoi(argv[3]));
    pthread_t t_ids[NUMTHREADS];
    pthread_t lastThread;
    
    numinrange = (atoi(argv[2]) - atoi(argv[1]) + 1);   //get the number of integers within range to determine how many integers each thread gets
    if(numinrange == 0 || numinrange < 0){
        perror("Error in Range! Please adjust your range parameters\n");
        return 1;
    }
    
    printf("Number of values that will be tested: %d\n", numinrange);
    
    if((numinrange % NUMTHREADS) != 0){             //if range does not equally divide to number of threads
        perror("Uneven threads to range, please adjust thread count-or-range to have equal ranges for all threads\n");
        return 1;
    }
    
    THREADRANGE = ((numinrange/NUMTHREADS));          //calculate the range for individual threads
    printf("Thread reange is: %d\n", THREADRANGE);

    for (i = 1; i <= NUMTHREADS; i++) {     //make i "<=" to number of threads requested to create an extra thread for sorting
        if((error = pthread_create(&t_ids[i], NULL, threadFunction, (void*)&i))){
            fprintf(stderr, "Failed to start thread %d:%s\n", i, strerror(error));
            return 1;
        }
    }
    
    for(i = 1; i < NUMTHREADS; i++){
        if ((error = pthread_join(t_ids[i], NULL))) {       //join all threads as they finish executing
            fprintf(stderr, "Failed to join thread %d:%s\n", i, strerror(error));
            return 1;
        }
    }
    
                                //this thread will sort the the final listing and then print them on the screen as well as to an individual datafile.
    if((error = pthread_create(&lastThread, NULL, lastThreadFunction, NULL))){
        fprintf(stderr, "Failed to create last thread: %s", strerror(error));
        return 1;
    }
    
    if ((error = pthread_join(lastThread, NULL))) { //join the last thread to the main thread
        fprintf(stderr, "Failed to join remainig thread:%s\n", strerror(error));
        return 1;
    }
    
    pthread_mutex_destroy(&threadfunctionlock);     //destroy our mutex locks
    
    return 0;
}

void *lastThreadFunction(void *argument){
    ifstream datfile;
    datfile.open("results_delgado.dat");
    int num = 0;
    linklist templist;
    while (datfile >> num) {
        templist.insert(num);
    }
    templist.linkedInsertionSort();
    templist.writeToFile("sorted_delgado.dat");
    templist.print();
    
    datfile.close();
    
    return NULL;
}


void *threadFunction(void *argument){
    int error = 0;
    if((error = pthread_mutex_lock(&threadfunctionlock)))           //look critical section
        seterror(error);
    
    int currentValue = ((COUNTER - 1) * THREADRANGE);
    int finalValue = (currentValue + THREADRANGE);
    COUNTER++;
    
    if((error = pthread_mutex_unlock(&threadfunctionlock)))         //unlock critical section
        seterror(error);
    
    linklist templist;
    while(currentValue <= finalValue){
        if(isPrime(currentValue)){          //if the number read is a prime then write the number to outputfile
            templist.insert(currentValue);
        }
        currentValue++;
    }
    templist.writeToFile("results_delgado.dat");
    templist.destroyList();
    
    return NULL;
}

void linklist::writeToFile(const char* file_name){
    FILE* file = fopen (file_name, "a"); //we use append flag in order not delete the other PIDs
    
    if(!file)
        printf("failed to open file");
  		
    nodeType *current;
    current = first;
    while (current != NULL) {
        fprintf(file, "%d\n", current->info);
        current = current -> link;
    }
    
    fclose(file);
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

linklist::linklist(){
    first = NULL;
    last = NULL;
    length = 0;
}

void linklist::insert(const int &item){
    
    nodeType *newNode;
    newNode = new nodeType;
    
    newNode->info = item;
    newNode->link = NULL;
    
    if(first == NULL){
        first = newNode;
        last = newNode;
    }
    else{
        last -> link = newNode;
        last = newNode;
    }
    
    length++;
}


void linklist::print() const{
    int i = 1;
    nodeType *current;
    current = first;
    while (current != NULL) {
        printf("Prime %d = %d\n", i, current -> info);//cout << current -> info << endl;
        current = current -> link;
        i++;
    }
    
    cout << endl;
}

int linklist::getLength(){
    return length;
}

void linklist::linkedInsertionSort(){
    nodeType *firstout;
    nodeType *lastout;
    nodeType *current;
    nodeType *trailcurrent;
    
    lastout = first;
    
    if (first == NULL) {
        cerr << "The list is empty\n";
    }
    else if (first -> link == NULL){
        cerr << "There is only one item in the list\n";
    }
    else{
        while (lastout -> link != NULL) {
            firstout = lastout -> link;
            
            if (firstout -> info < first -> info) {
                lastout -> link = firstout -> link;
                firstout -> link = first;
                first = firstout;
            }
            else{
                trailcurrent = first;
                current = first -> link;
                
                while (current -> info < firstout -> info) {
                    trailcurrent = current;
                    current = current -> link;
                }
                
                if (current != firstout) {
                    lastout -> link = firstout -> link;
                    firstout -> link = current;
                    trailcurrent -> link = firstout;
                }
                else{
                    lastout = lastout -> link;
                }
                
            }
        }
    }
}

void linklist::destroyList(){
    nodeType *temp;
    
    while (first != NULL) {
        temp = first;
        first = first -> link;
        delete temp;
    }
    
    last = NULL;
}

nodeType * linklist::HEAD(){
    return first;
}
