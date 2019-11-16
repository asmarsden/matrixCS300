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

typedef struct QueueMessage{
    long type;
    int jobid;
    int rowvec;
    int colvec;
    int innerDim;
    int data[100];
} Msg;

void * calculate(){
//grab message type 1
    printf("debug compute 5\n");

key_t key;
Msg msg;
    key = ftok("/home/asmarsden",11696847);//use asmarsden??
    printf("debug compute 6\n");
//TODO: mathy math and locks
int msgid = msgget(key, 0666 | IPC_CREAT);
msgrcv(msgid, &msg, 7*sizeof(int), 1, 0);//check tha tthis is right
printf("recieved message\n");
printf("message info: type: %ld\n", msg.type);
printf("message info: jobid: %d\n", msg.jobid);
printf("message info: rowvec: %d\n", msg.rowvec);
printf("message info: colvec: %d\n", msg.colvec);
printf("message info: innerdim: %d\n", msg.innerDim);
printf("message info: data[0]: %d\n", msg.data[0]);
//lock
    printf("debug compute 7\n");

msg.type = 2;
msg.jobid = 2*msg.jobid;
msg.rowvec = 2*msg.rowvec;
msg.colvec = 2*msg.colvec;
msg.innerDim = 2*msg.innerDim;
msg.data[0] = 2*msg.data[0];
msgsnd(msgid, &msg, 7*sizeof(int),2);//make sure this is right
//unlock

//int sum = 0;
//lock. might have to lock before this?
//for (int i = 0; i < rowvec.size; i++){
//    sum += rowvec[i]*colvec[i];
//}
//package sum and send to package
//print sent successful
//unlock
    printf("debug compute 8\n");

return NULL;
}

int main(){ 
    int numThreads = 0;
    printf("debug compute 1\n");
    pthread_t p[100];//is this a good number?
    pthread_attr_t attr;
    while(1){numThreads++;
        printf("debug compute 2\n");
        //calculate();
        //make threads
        pthread_create(&p[0], NULL, calculate, NULL);//figure out if those nulls should be there
//im overcomplicating this
if (numThreads > 100) break;
    }
        printf("debug compute3\n");

while (numThreads>0){
    pthread_join(p[numThreads-1], NULL);
    numThreads--;
}
    printf("debug compute 4\n");


return 0;
}