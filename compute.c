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
pthread_mutex_t lock;
typedef struct QueueMessage{
    long type;
    int jobid;
    int rowvec;
    int colvec;
    int innerDim;
    int data[100];
} Msg;

void * calculate(){//still need to figure out what this should return

//grab message type 1
    //printf("debug compute 5\n");

key_t key;
Msg msg;
    key = ftok("/home/asmarsden",11696847);//use asmarsden??
    //printf("debug compute 6\n");
//TODO: mathy math and locks. i think this is done
int msgid = msgget(key, 0666 | IPC_CREAT);
pthread_mutex_lock(&lock);
msgrcv(msgid, &msg, 106*sizeof(int), 1, 0);//check tha tthis is right
//printf("recieved message\n");
//printf("message info: type: %ld\n", msg.type);
//printf("message info: jobid: %d\n", msg.jobid);
//printf("message info: rowvec: %d\n", msg.rowvec);
//printf("message info: colvec: %d\n", msg.colvec);
//printf("message info: innerdim: %d\n", msg.innerDim);
//printf("message info: data[0]: %d\n", msg.data[0]);
//lock
    //printf("debug compute 7\n");
Msg returnmsg;
returnmsg.type = 2;
returnmsg.rowvec = msg.rowvec;
returnmsg.colvec = msg.colvec;
returnmsg.jobid = msg.jobid;
returnmsg.innerDim = msg.innerDim;
int sum = 0;
for (int i = 0; i < msg.innerDim * 2; i++){
    printf("data[%d] = %d\n",i, msg.data[i]); 
}
for (int i = 0; i < msg.innerDim; i++){
    sum += msg.data[i] * msg.data[i+msg.innerDim];
    //printf("sum += %d * %d to = %d\n", msg.data[i], msg.data[i+msg.innerDim], sum);
}
//printf("sum for %d,%d is %d\n", msg.colvec, msg.rowvec, sum);
returnmsg.data[0] = sum;
msgsnd(msgid, &returnmsg, 7*sizeof(int),2);//make sure this is right
//unlock
pthread_mutex_unlock(&lock);
//printf("debug compute 8\n");
return NULL;
}

int main(){ 
    int numThreads = 0;
  //  printf("debug compute 1\n");
    pthread_t p[100];//is this a good number?
    //pthread_attr_t attr;
    while(1){numThreads++;
    //    printf("debug compute 2\n");
        //calculate();
        //make threads
        pthread_create(&p[0], NULL, calculate, NULL);//figure out if those nulls should be there
//im overcomplicating this
if (numThreads > 100) break;
    }
      //  printf("debug compute3\n");

while (numThreads>0){
    pthread_join(p[numThreads-1], NULL);
    numThreads--;
}
    //printf("debug compute 4\n");


return 0;
}