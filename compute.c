/*Written by: Aislinn Marsden
github.com/asmarsden/matrixCS300

compute.c: creted a thread pool of a specified number of threads
uses those threads to read in messages and compute dot products
then either sends messages with dot product results
or prints the results to screen with '-n'

to compile: makefile is included, so do make
usage: ./compute <number of threads> <-n or nothing>

*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <assert.h>

int sent;
int recieved;
//I had to make sent and recieved global in order to make the signal handler work properly.

void sigintHandler(int sig_num){//Signal handler that prints current number of sent and recieved jobs every time you press Ctrl-C during execution.
    signal(SIGINT, sigintHandler);
    printf("Jobs Sent %d Recieved %d\n", sent, recieved);
}

pthread_mutex_t lock;

typedef struct QueueMessage{//Struct for message type to send back and forth between compute and package.
    long type;
    int jobid;
    int rowvec;
    int colvec;
    int innerDim;
    int data[100];
} Msg;

void * nosend(){//The function the threads enter if -c is specified; no messages are sent.
//I am assuming this does not send messages, simply prints out the output. 
    while(1){
        key_t key;
        Msg msg;
        key = ftok("/home/asmarsden",11696847);
        int msgid = msgget(key, 0666 | IPC_CREAT);
        pthread_mutex_lock(&lock);
        int check = 0;
        while(check==0){
            check = msgrcv(msgid, &msg, 106*sizeof(int), 1, 0);//this searches for the max size, since compute doesnt know the actual size yet.
        }
        int size = ((msg.innerDim * 2 + 4)*32) / 8; //This calculates the size in bytes that the message is. 
        printf("Recieving job id %d type %ld size %d\n", msg.jobid, msg.type, size);
        recieved++;
        int sum = 0; 
        for (int i = 0; i < msg.innerDim; i++){//computing the dot product given in the message
            sum += msg.data[i] * msg.data[i+msg.innerDim];
        }
        printf("Sum for cell %d,%d is %d\n", msg.rowvec, msg.colvec, sum);
        pthread_mutex_unlock(&lock);
        }
    return NULL;
}


void * calculate(){//This is the function the threads go to if -n is not specified. This grabs a message, computes it, then sends the result back.
    while(1){
        key_t key;
        Msg msg;
        key = ftok("/home/asmarsden",11696847);
        int msgid = msgget(key, 0666 | IPC_CREAT);
        pthread_mutex_lock(&lock);
        int check = 0;
        while(check==0){//Added this to make sure a message is recieved; if the user Ctrl-C's while process is waiting for message, it will stop waiting but not get a message. This led to seg faults.
            check = msgrcv(msgid, &msg, 106*sizeof(int), 1, 0);
        }
        int size = ((msg.innerDim * 2 + 4)*32) / 8;
        printf("Recieving job id %d type %ld size %d\n", msg.jobid, msg.type, size);
        recieved++;
        Msg returnmsg;
        returnmsg.type = 2;
        returnmsg.rowvec = msg.rowvec;
        returnmsg.colvec = msg.colvec;
        returnmsg.jobid = msg.jobid;
        returnmsg.innerDim = msg.innerDim;
        int sum = 0;
        for (int i = 0; i < msg.innerDim; i++){
            sum += msg.data[i] * msg.data[i+msg.innerDim];
        }
        returnmsg.data[0] = sum;
        int rc = msgsnd(msgid, &returnmsg, 7*sizeof(int),2);
        size = ((returnmsg.innerDim * 2 + 4)*32) / 8;
        printf("Sending job id %d type %ld size %d (rc=%d)\n", returnmsg.jobid, returnmsg.type, size, rc);
        sent++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[]){ 
    signal(SIGINT, sigintHandler);
    sent = 0;
    recieved = 0;
    assert(argc == 2 || argc == 3);
    int numThreads = atoi(argv[1]);
    pthread_t p[numThreads];
    //pthread_attr_t attr;
    while(numThreads>0){
        numThreads--;
        if (argc == 3){
            assert(argv[2][0] == '-' && argv[2][1] == 'n');
            pthread_create(&p[0], NULL, nosend, NULL);
        }
        else{
            pthread_create(&p[0], NULL, calculate, NULL);
        }
    }
    while (numThreads<atoi(argv[1])){ 
        pthread_join(p[numThreads], NULL);
        numThreads++;
    }
    return 0;
}