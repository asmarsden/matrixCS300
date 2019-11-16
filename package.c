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

//usage:
//./package matrix1 matrix2 result secs between thread creation
#include <stdio.h>
#include <stdlib.h>
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


int populate(int row, int col, int inner, int data[], int id){

    key_t key;
    key = ftok("/home/asmarsden",11696847);//use asmarsden??
    int msgid= msgget(key, 0666 | IPC_CREAT);
    Msg msg;
    msg.type = 1;
    //populate msg
    msg.jobid=id;
    msg.rowvec=row;
    msg.colvec=col;
    msg.innerDim =inner;
    msg.data=data; //have to figure out how big to make size...
    msgsnd(msgid, &msg, 7*sizeof(int), 1);//make sure this is right
    printf("sent message %d\n", x);
    return 0;
}

void recieve(int* result){
key_t key;
Msg msg;
    key = ftok("/home/asmarsden",11696847);//use asmarsden??
int msgid = msgget(key, 0666 | IPC_CREAT);
msgrcv(msgid, &msg, 7*sizeof(int), 2, 0);
result[msg.row][msg.col]=msg.data[0];//passing by reference
printf("recieved message. info:\n");
printf("type: %ld\njobid: %d\n rowvec:%d\n", msg.type, msg.jobid, msg.rowvec);
printf("colvec: %d\ninnerDim: %d\n, data[0]: %d\n", msg.colvec, msg.innerDim, msg.data[0]);
return;
}

int main(int argc, char *argv[]){
    FILE *matrix1 = fopen(argv[1], "r");
    printf("debug1\n");
    int mat1width, mat1height, mat2width, mat2height;
    fscanf(matrix1, "%d", &mat1width);//make sure its width then height
    fscanf(matrix1, "%d", &mat1height);
        printf("debug2\n");

    int *mat1 = (int *)malloc(mat1width *mat1height * sizeof(int));
    for (int i = 0; i < mat1height; i++){
        for (int j = 0; j < mat1width; j++){
            fscanf(matrix1, "%d", &mat1[i][j]);
        }
    }
        printf("debug3\n");

    fclose(matrix1);
    FILE *matrix2 = fopen(argv[2], "r");
    fscanf(matrix2, "%d", &mat2width);//make sure its width then height
    fscanf(matrix2, "%d", &mat2height);
    int *mat2 = (int *)malloc(mat2width *mat2height * sizeof(int));
    for (int i = 0; i < mat2height; i++){
        for (int j = 0; j < mat2width; j++){
            fscanf(matrix1, "%d", &mat2[i][j]);
        }
    }
        printf("debug4\n");

    fclose(matrix2);
    int id = 0;
    FILE *resultMatrix = fopen(argv[3], "w");
    //add some mathy mumbo jumbo.. mat1height * mat2 width?
    int *result = (int *)malloc(mat1width * mat2height *sizeof(int));
    //here we figure out how to package the stuff
    for (int i = 0; i < mat1width; i++){
        for (int j = 0; j < mat2height; j++){
            //TODO: mathy math
            populate(row, col, inner, data[inner*2], id);
            id++;
        }
    }
        for (int i = 0; i < mat1width*mat2height; i++){
            recieve(result);
            }
        printf("debug6\n");
    fprintf(resultMatrix, "%d ",mat1width);
    fprintf(resultMatrix, "%d\n", mat2height);
    for (int i = 0; i < mat1width; i++){
        for (int j = 0; j < mat2height; j++){
            fprintf(resultMatrix, "%d ", result[i][j]);
        }
    }
fclose(resultMatrix);
    free(mat1);
    free(mat2);
    free(result);
        printf("debug7\n");

    return 0;
}