//this will create and populate the input
//matrices from a pair of input files
//then create a thread to package up
//dot product subtasks: one row * one col
//each subtask will be put on message queue
//once package is done, it will read
//completed calculations from queue and 
//safely populate the outut matrix
//once all calculations are completed
//package will print output matrix
//to the screen and the output file
//then exit
//gcc package.c -Wall -o package -std=c99
//gcc compute.c -Wall -o compute -std=c99
//ipcs -q -o to check message queue
//ipcrm -q (number) to remove message from queue

/*right now the biggest thing i want to do
is figure out how compute.c should know
the size of the thing that will be sent off
and read in*/

//nothing should be hardcoded, so i gotta fix those



//usage:
//./package matrix1 matrix2 result secs between thread creation
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>

int sent;
int recieved;

void sigintHandler(int sig_num){
    signal(SIGINT, sigintHandler);
    printf("Jobs Sent %d Recieved %d\n", sent, recieved);
}

typedef struct QueueMessage{
    long type;
    int jobid;
    int rowvec;
    int colvec;
    int innerDim;
    int data[100];
} Msg;


void populate(int row, int col, int inner, int data[], int id, int sl){
    key_t key;
    key = ftok("/home/asmarsden",11696847);
    int msgid= msgget(key, 0666 | IPC_CREAT);
    Msg msg;
    msg.type = 1;
    msg.jobid=id;
    msg.rowvec=row;
    msg.colvec=col;
    msg.innerDim =inner;
    for (int i = 0; i < inner * 2; i++){
        msg.data[i]=data[i];
    }
    int rc = msgsnd(msgid, &msg, ((inner*2)+6)*sizeof(int), 1);//make sure this is right
    int size = ((msg.innerDim * 2 + 4)*32) / 8;
    printf("Sending job id %d type %ld size %d (rc=%d)\n", msg.jobid, msg.type, size, rc);
    sent++;
    sleep(sl);
    return;
}

void recieve(int** result){
    key_t key;
    Msg msg;
    key = ftok("/home/asmarsden",11696847);//use asmarsden??
    int msgid = msgget(key, 0666 | IPC_CREAT);
    int check = 0;
    while(check==0){//added this while to make sure this actually gets a message, since without it, if i ctrl-c'd it would move on without getting a message then seg fault
       check = msgrcv(msgid, &msg, 7*sizeof(int), 2, 0);
    }
    recieved++;
    int size = ((msg.innerDim * 2 + 4)*32) / 8;
    printf("Recieving job id %d type %ld size %d\n", msg.jobid, msg.type, size);
    result[msg.rowvec][msg.colvec]=msg.data[0];
    return;
}

int main(int argc, char *argv[]){
    signal(SIGINT, sigintHandler);
    sent = 0;
    recieved = 0;
    FILE *matrix1 = fopen(argv[1], "r");
    assert(matrix1 != NULL);
    int mat1width, mat1height, mat2width, mat2height;
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
    int id = 0;
    FILE *resultMatrix = fopen(argv[3], "w");
    assert(resultMatrix != NULL);
    int **result = (int **)malloc(mat1height * sizeof(int*));
    for (int i = 0; i < mat1height; i++){
        result[i] = malloc(mat2width * sizeof(int));
    }
    for (int i = 0; i < mat1height; i++){
        for (int j = 0; j < mat2width; j++){
            int data[mat1width*2];
            for (int k = 0; k < mat1width; k++){
                data[k] = mat1[i][k];
                data[k+mat1width] = mat2[k][j];
            }
            populate(i, j, mat1width, data, id, atoi(argv[4]));
            id++;
        }
    }
    for (int i = 0; i < mat1height*mat2width; i++){
        recieve(result);
    }//can't combine these for loops because the above one doesnt guarentee any particular order. this couldve been faster otherwise
    for (int i = 0; i < mat1height; i++){
        for (int j = 0; j < mat2width; j++){
            fprintf(resultMatrix, "%d ", result[i][j]);
        }
    }
    fclose(resultMatrix);
    for (int i = 0; i < mat1height; i++){
        free(mat1[i]);
    }
    for (int i = 0; i < mat2height; i++){
        free(mat2[i]);
    }
    for (int i = 0; i < mat1height; i++){
        free(result[i]);
    }
    free(mat1);
    free(mat2);
    free(result);
    return 0;
}