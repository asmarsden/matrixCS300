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

int sent;
int recieved;

void sigintHandler(int sig_num){
    //if (sig_num == SIGINT){
    signal(SIGINT, sigintHandler);
    printf("Jobs Sent %d Recieved %d\n", sent, recieved);
    //}
    //return;
}

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
    for (int i = 0; i < inner * 2; i++){
        msg.data[i]=data[i];
        //printfprintf("adding %d to dot product of %d\n", data[i], id);
    }
    
    //msg.data=data; //have to figure out how big to make size...
    msgsnd(msgid, &msg, ((inner*2)+6)*sizeof(int), 1);//make sure this is right
    sent++;
    return 0;
}

void recieve(int** result){
key_t key;
Msg msg;
    key = ftok("/home/asmarsden",11696847);//use asmarsden??
int msgid = msgget(key, 0666 | IPC_CREAT);
printf("debuga\n");
int check = 0;
while(check==0){
msgrcv(msgid, &msg, 7*sizeof(int), 2, 0);
}
printf("debugb\n");
recieved++;
printf("debugc\n");
result[msg.rowvec][msg.colvec]=msg.data[0];
printf("debugd\n");//passing by reference
//printf("recieved message. info:\n");
//printf("type: %ld\njobid: %d\n rowvec:%d\n", msg.type, msg.jobid, msg.rowvec);
//printf("colvec: %d\ninnerDim: %d\n, data[0]: %d\n", msg.colvec, msg.innerDim, msg.data[0]);
return;
}

int main(int argc, char *argv[]){
    signal(SIGINT, sigintHandler);
    printf("debug1\n");
    sent = 0;
    recieved = 0;
    printf("debug2\n");
    //must figure out how to send info...
    //looks like ill have t use a global variable
    FILE *matrix1 = fopen(argv[1], "r");
    //printf("debug1\n");
    printf("debug3\n");
    int mat1width, mat1height, mat2width, mat2height;
    fscanf(matrix1, "%d", &mat1height);//make sure its width then height
    fscanf(matrix1, "%d", &mat1width);
        //printf("debug2\n");

    int **mat1 = (int **)malloc(mat1height * sizeof(int*));
    for (int i = 0; i < mat1height; i++){
        mat1[i] = malloc(mat1width * sizeof(int));
    }
    for (int i = 0; i < mat1height; i++){
        for (int j = 0; j < mat1width; j++){
            fscanf(matrix1, "%d", &mat1[i][j]);
        }
    }
        //printf("debug3\n");

    fclose(matrix1);
    printf("debug4\n");
    FILE *matrix2 = fopen(argv[2], "r");
    fscanf(matrix2, "%d", &mat2height);//make sure its width then height
    fscanf(matrix2, "%d", &mat2width);
    int **mat2 = (int **)malloc(mat2height * sizeof(int*));
    for (int i = 0; i < mat2height; i++){
        mat2[i] = malloc(mat2width * sizeof(int));
    }    for (int i = 0; i < mat2height; i++){
        for (int j = 0; j < mat2width; j++){
            fscanf(matrix1, "%d", &mat2[i][j]);
        }
    }
        //printf("debug4\n");

    fclose(matrix2);
    int id = 0;
    printf("debug5\n");
    FILE *resultMatrix = fopen(argv[3], "w");
    //add some mathy mumbo jumbo.. mat1height * mat2 width?
    int **result = (int **)malloc(mat1height * sizeof(int*));
    for (int i = 0; i < mat1height; i++){
        result[i] = malloc(mat2width * sizeof(int));
    }
    //here we figure out how to package the stuff
    for (int i = 0; i < mat1height; i++){
        for (int j = 0; j < mat2width; j++){

            //TODO: mathy math
            //ugh i need paper
            int data[mat1width*2];
            //i wonder if there are notebooks for sale downstairs
            for (int k = 0; k < mat1width; k++){
                //i think i have my widths and heights mixed up
                //ill fix that momentarily
                data[k] = mat1[i][k];
                data[k+mat1width] = mat2[k][j];
            }
            populate(i, j, mat1width, data, id);
            id++;
        }
    }
        for (int i = 0; i < mat1height*mat2width; i++){
            recieve(result);
            }
        //printf("debug6\n");
    //fprintf(resultMatrix, "%d ",mat1height);
    //fprintf(resultMatrix, "%d\n", mat2width);
    for (int i = 0; i < mat1height; i++){
        for (int j = 0; j < mat2width; j++){
            fprintf(resultMatrix, "%d ", result[i][j]);
        }
    }
    printf("debug6\n");
fclose(resultMatrix);
    free(mat1);
    free(mat2);
    free(result);
        printf("debug7\n");

    return 0;
}