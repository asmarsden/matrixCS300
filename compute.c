//this will create a pool of threads
//to read subtasks from the message queue
//calulate the value
//complete the calculation
///and put the result back on the queue
//with message type two
//when ctrl-c, print out status
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <assert.h>

int sent;
int recieved;

void sigintHandler(int sig_num){
    signal(SIGINT, sigintHandler);
    printf("Jobs Sent %d Recieved %d\n", sent, recieved);
}

pthread_mutex_t lock;
typedef struct QueueMessage{
    long type;
    int jobid;
    int rowvec;
    int colvec;
    int innerDim;
    int data[100];
} Msg;


void * nosend(){
    while(1){
        key_t key;
        Msg msg;
        key = ftok("/home/asmarsden",11696847);//use asmarsden??
        int msgid = msgget(key, 0666 | IPC_CREAT);
        pthread_mutex_lock(&lock);
        int check = 0;
        while(check==0){
            check = msgrcv(msgid, &msg, 106*sizeof(int), 1, 0);
        }
        int size = ((msg.innerDim * 2 + 4)*32) / 8;
        printf("Recieving job id %d type %ld size %d\n", msg.jobid, msg.type, size);
        recieved++;
        int sum = 0;
        for (int i = 0; i < msg.innerDim; i++){
            sum += msg.data[i] * msg.data[i+msg.innerDim];
        }
        printf("Sum for cell %d,%d is %d\n", msg.rowvec, msg.colvec, sum);
        pthread_mutex_unlock(&lock);
        }
    return NULL;
}


void * calculate(){
    while(1){
        key_t key;
        Msg msg;
        printf("debug 1\n");
        key = ftok("/home/asmarsden",11696847);
        int msgid = msgget(key, 0666 | IPC_CREAT);
                printf("debug a\n");

        pthread_mutex_lock(&lock);
        int check = 0;
                printf("debug b\n");

         while(check==0){
                    printf("check = %d\n", check);

            check = msgrcv(msgid, &msg, 106*sizeof(int), 1, 0);
   }
           printf("debug C\n");

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
                printf("debug 3\n");

        for (int i = 0; i < msg.innerDim * 2; i++){
            printf("data[%d] = %d\n",i, msg.data[i]); 
        }
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
            printf("debug 4\n");

    int numThreads = atoi(argv[1]);
    pthread_t p[numThreads];
    //pthread_attr_t attr;
    while(numThreads>0){
                printf("debug 5\n");

        numThreads--;
        if (argc == 3){
            assert(argv[2][0] == '-' && argv[2][1] == 'c');
            pthread_create(&p[0], NULL, nosend, NULL);
        }
        else{
            pthread_create(&p[0], NULL, calculate, NULL);//figure out if those nulls should be there
        }
    }
    while (numThreads<atoi(argv[1])){
        pthread_join(p[numThreads], NULL);
        numThreads++;
    }
    return 0;
}