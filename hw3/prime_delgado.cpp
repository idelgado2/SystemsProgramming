/****************************************************************************************
 * Systems Programming (HomeWork_3)
 * Isaac Delgado
 * COSC_4348
 * 3/6/2017
 * Purpose: Will fork 4 process, each of which will read individual text files contatining
 *          integers. Each process will strip the primes in that file. Once this is done
 *          the parent process will strip the duplicates and then print the list of primes
 *          out.
 *
 * Input:   This program will take one command line argument. The possible command line
 *          arguments are "chain" and "fan". If chain is chosen the program will fork 
 *			processes in a chain methodology. If yfan is chosen then fan method will be used
 *          (example execution command:   "./prime_delgado.c fan" or "./prime_delgado.c chain")
 *
 * Output:  The output of this program will be a list of all primes out of these 4 text files.
 *
 *		(*Additionally the linked list functionality was adapted from my previous data 
 *         structure labs, which in turn is adapted from teh Data Structures Book used in
 *           that course*)
 ***************************************************************************************/
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

using namespace std;

struct nodeType{						//this is a struct for creating a linked list
    int info;
    nodeType *link;
};

class linklist{							//linked list class to encaspulate linked list functionality
    public:
        linklist();						//condtructor to intialize list
        void insert(const int&);
        void print() const;
        bool isEmpty() const;
        void linkedInsertionSort();		//function to sort linked list by Insertion sort
        void destroyList();
        linklist removeDuplicates();
    private:
        nodeType *first;				//first position in our list
        nodeType *last;					//last position in our list
        int length;
};


bool isPrime(int);						//function to determin the passed number is a prime
void readContent(string, string);		//function to read & write to givin files
void readFile(int);	


int main (int argc, const char * argv[]) {
    
    int choice, status, ppid;
    int counter = 0;			//counter to determine what file each process is assinged to
    pid_t childpid = 0;
    
    if(argc != 2){				//command line arguments must be exactly 2
    	perror("Too many or NOT enough passed arguments");
    	return 1;
    }
    
    if(strcmp(argv[1], "chain") == 0){			//Chain fork porcess
    	ppid = getpid();
        for (int i = 0; i < 4; i++) {
            
            childpid = fork();
            
            if(childpid == -1){
                perror ("Fork failed");
                break;
            }
            if(childpid){         //parent process waits or breaks so child will continue forking
                waitpid(childpid, &status, 0);       //make parent wait -- so the child is able to get parents' process IDs
                break;
            }else{                 //child process
                counter++;
            }
            
            cout<< "I am child number " << getpid() << ", my parent is "
                            << getppid() << " I computed prime numbers in Number" << counter << ".txt" << endl;
            readFile(counter);
        }

    }else if(strcmp(argv[1], "fan") == 0){		//Fan fork process
    	ppid = getpid();
        for(int i = 0; i < 4; i++){             //parent process creates all child process
            counter++;
            childpid = fork();
            
            if(childpid == -1){
                perror("Fork Failed");
                break;
            }
            
            if(childpid == 0){              	//child process
                readFile(counter);
                cout<< "I am child number " << getpid() << ", my parent is "
                            << getppid() << " I computed prime numbers in Number" << counter << ".txt" << endl;
                return 0;//break;
            }else{
                waitpid(childpid, &status, 0);
            }
        }
     }else{
         perror("Invalid Option");
      }

    if(getpid() == ppid){                           //Ultimate Parent process
        cout << "I am the parent process, My id is " << getpid() << endl;
        linklist sortedlist;
        ifstream myfile;
        myfile.open("results_delgado.dat");
        int number;
        
        while(myfile >> number){                    //insert all primes numbers into linked list
            sortedlist.insert(number);				
        }
        
        sortedlist.linkedInsertionSort();
        linklist list = sortedlist.removeDuplicates();
        list.print();
        
        sortedlist.destroyList();					//destroy allocated memory and close opened files
        list.destroyList();
        myfile.close();
    }
    
    return 0;
}

linklist linklist::removeDuplicates(){
    linklist list;
    
    nodeType *current;
    nodeType *trail;
    current = first -> link;            //current will point to second item
    trail = first;                      //trail will point to the first item
    
    list.insert(trail->info);           //insert first item into the list
    
    while(current -> link != NULL){             //while there are items to read from linked list
        if(current -> info == trail -> info){   //if there are duplicates move current node and ignore
            current = current -> link;
            continue;
        }else{                                  //else insert current item into the list
            list.insert(current -> info);
            trail = current;
            current = current -> link;
        }
    }
    
    return list;
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
    nodeType *current;
    current = first;
    int i = 1;
    while (current != NULL) {
        cout << "Prime" << i << "=" << current -> info << endl;
        i++;
        current = current -> link;
    }
    
    cout << endl;
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

void readContent(string txtfile, string datfile){
    ifstream myfile;
    myfile.open(txtfile.c_str());
    
    ofstream outputFile;
    outputFile.open(datfile.c_str());
    int number;
    
    while(myfile >> number){
        if(isPrime(number)){          //if the number read is a prime then write the number to outputfile
            outputFile << number << endl;
        }
    }
    
    outputFile.close();
    myfile.close();
}

void readFile(int counter){
    
    switch (counter) {
        case 1:
            readContent("Numbers1.txt", "results_delgado.dat");
            break;
        case 2:
            readContent("Numbers2.txt", "results_delgado.dat");
            break;
        case 3:
            readContent("Numbers3.txt", "results_delgado.dat");
            break;
        case 4:
            readContent("Numbers4.txt", "results_delgado.dat");
            break;
        default:
            perror("error too many child processes");
            break;
    }
}



