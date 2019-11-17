/* Written by: Aislinn Marsden
github.com/asmarsden/matrixCS300

package.c: this reads in two matrices and packages up the dot products needed to multiply them together.
It sends these dot products onto a message queue for compute.c to compute.
It then recieves any messages compute.c sends back, and writes those to a result file. 

to compile: makefile is included, so just use make
usage: ./package <matrix1 name> <matrix2 name> <result matrix name> <seconds between thread creation>

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

int sent;
int recieved;
//I had to make sent and recieved global in order to make the signal handler work properly.
void sigintHandler(int sig_num){//Same signal handler as in compute.c
    signal(SIGINT, sigintHandler);
    printf("Jobs Sent %d Recieved %d\n", sent, recieved);
}

pthread_mutex_t lock;

typedef struct QueueMessage{//Same message struct as in compute.c
    long type;
    int jobid;
    int rowvec;
    int colvec;
    int innerDim;
    int data[100];
} Msg;

void * populate(void *in){//This takes in the info needed to send a message to the message queue, and sends a message with this info to the message queue.
    key_t key;
    key = ftok("/home/asmarsden",11696847);
    Msg *temp = (Msg*)in;
    Msg msg;
    msg.type = temp->type;
    msg.rowvec = temp->rowvec;
    msg.colvec = temp->colvec;
    msg.jobid = temp->jobid;
    msg.innerDim = temp->innerDim;
    for (int i = 0; i < temp->innerDim*2; i++){
        msg.data[i]=temp->data[i];
    }
    int msgid= msgget(key, 0666 | IPC_CREAT);
    pthread_mutex_lock(&lock);
    int rc = msgsnd(msgid, &msg, ((msg.innerDim*2)+4)*sizeof(int), 1);//make sure this is right
    int size = ((msg.innerDim * 2 + 4)*32) / 8;
    printf("Sending job id %d type %ld size %d (rc=%d)\n", msg.jobid, msg.type, size, rc);
    sent++;
    //gotta free in here somewhere
    //free(temp);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void recieve(int** result){//This recieves messages that have come back from compute.c and adds them to the resulting matrix.
    key_t key;
    Msg msg;
    key = ftok("/home/asmarsden",11696847);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    int check = 0;
    while(check==0){//added this while to make sure this actually gets a message, since without it, if i ctrl-c'd it would move on without getting a message then seg fault
       check = msgrcv(msgid, &msg, 7*sizeof(int), 2, 0);
    }
    recieved++;
    int size = ((1 + 4)*32) / 8;//1 item in data
    printf("Recieving job id %d type %ld size %d\n", msg.jobid, msg.type, size);
    result[msg.rowvec][msg.colvec]=msg.data[0];
    return;
}

int main(int argc, char *argv[]){
    signal(SIGINT, sigintHandler);
    sent = 0;
    recieved = 0;
    int mat1width, mat1height, mat2width, mat2height;
  

    FILE *matrix1 = fopen(argv[1], "r");
    assert(matrix1 != NULL);
    fscanf(matrix1, "%d", &mat1height);
    fscanf(matrix1, "%d", &mat1width);
    int **mat1 = (int **)malloc(mat1height * sizeof(int*));
    for (int i = 0; i < mat1height; i++){
        mat1[i] = malloc(mat1width * sizeof(int));
    }
    for (int i = 0; i < mat1height; i++){
        for (int j = 0; j < mat1width; j++){
            fscanf(matrix1, "%d", &mat1[i][j]);
        }
    }
    fclose(matrix1);

    FILE *matrix2 = fopen(argv[2], "r");
    assert(matrix2 != NULL);
    fscanf(matrix2, "%d", &mat2height);
    fscanf(matrix2, "%d", &mat2width);
    assert(mat1width == mat2height); //If this isn't the case, then the matrices cannot be multiplied.
    int **mat2 = (int **)malloc(mat2height * sizeof(int*));
    for (int i = 0; i < mat2height; i++){
        mat2[i] = malloc(mat2width * sizeof(int));
    }    
    for (int i = 0; i < mat2height; i++){
        for (int j = 0; j < mat2width; j++){
            fscanf(matrix1, "%d", &mat2[i][j]);
        }
    }
    fclose(matrix2);
    pthread_t p[mat1height*mat2width];
    Msg * msg = malloc((mat1height*mat2width)*sizeof(Msg));
    int id = 0;
    int **result = (int **)malloc(mat1height * sizeof(int*));
    for (int i = 0; i < mat1height; i++){
        result[i] = malloc(mat2width * sizeof(int));
    }

    //pthread_create(&p, NULL, dotProduct, NULL);
    for (int i = 0; i < mat1height; i++){
        for (int j = 0; j < mat2width; j++){
            //int data[mat1width*2];

            //Msg *msg = malloc(sizeof(Msg));
            (msg+id)->type = 1;
            (msg+id)->jobid = id; 
            (msg+id)->rowvec = i; 
            (msg+id)->colvec = j; 
            (msg+id)->innerDim = mat1width;
            for (int k = 0; k < mat1width; k++){
                (msg+id)->data[k] = mat1[i][k];
                (msg+id)->data[k+mat1width] = mat2[k][j];
            }
            pthread_create(&p[id], NULL, populate,(void*) (msg+id));
            sleep(atoi(argv[4]));
            id++;
        }
    }

    FILE *resultMatrix = fopen(argv[3], "w");
    assert(resultMatrix != NULL);
    for (int i = 0; i < mat1height*mat2width; i++){
        recieve(result);
    }//can't combine these for loops because the above one doesnt guarentee any particular order. this couldve been faster otherwise
    for (int i = 0; i < mat1height; i++){
        for (int j = 0; j < mat2width; j++){
            fprintf(resultMatrix, "%d ", result[i][j]);
        }
    }
    fclose(resultMatrix);
    while (id >0){
        pthread_join(p[id], NULL);
        id--;
    }
    for (int i = 0; i < mat1height; i++){
        free(mat1[i]);
    }
    for (int i = 0; i < mat2height; i++){
        free(mat2[i]);
    }
    for (int i = 0; i < mat1height; i++){
        free(result[i]);
    }
    free(msg);
    free(mat1);
    free(mat2);
    free(result);
    return 0;
}