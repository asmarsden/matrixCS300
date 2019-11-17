# matrixCS300

Compute and Package work together to compute the multiplication of two matrices. Package reads in two files, each containing one matrix, and packages up the dot products of each and sends thos via a message queue to compute, which uses a thread pool of a specified size to compute each dot product and either send the result back to package, or print the result to screen. 

This repository (or zip file, as I'm turning it in that way) contains a series of test matrices, each named with the following format:
matrix(letter)(1 or 2, for 1st or 2nd matrix).dat

Compute and Package can be tested with any corresponding pair of matrices. 

A makefile is included; you can do make to compile and make clean to remove the executables. 

Note: When doing very large computations, with lots of messages to be sent and recieved, the message queue may become full. This can be avoided by doing -n on compute, but this makes it so that a result is not created, the output is simply printed to screen. 

Update: I was making package wrong. Now that I have added threads to package, the above statement is false.

./package matrixa1.dat matrixa2.dat result.dat 1
./compute 3
./compute 3 -n