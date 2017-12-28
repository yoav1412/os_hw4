#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define CHUNKSIZE 1048576 //2**20 bytes

pthread_mutex_t shardBufferMutex, numDeactivatedThreadsMutex;
int out_fd, globalXorCounter, numInFiles, numCurrentlyActiveThreads, numDeactivatedThreadsInCurrIteration = 0;
int globalMaxNumReadInIteration = 0;
char sharedResultBuff[CHUNKSIZE];
pthread_cond_t finishedIterationCV;


/*
 * assuming size(buff2)<=size(buff1)
 */
void xorTwoBuffs(char buff1[CHUNKSIZE], char buff2[CHUNKSIZE], int size2){
    for (int i=0;i<size2; i++){
        buff1[i] ^= buff2[i];
    }
}

int getFileSize(int fd){
    int currPos, size;
    if ( (currPos = lseek(fd, 0, SEEK_CUR)) == -1 ){ return -1; }
    if ( (size = lseek(fd,0,SEEK_END)) == -1 ){ return -1; }
    if (lseek(fd, currPos, SEEK_SET) == -1) { return -1; }
    return size;
}

void* threadWork(void* filePathParam){
    char *filePath = (char*) filePathParam;
    int numRead,totalNumRead, filzeSize;
    bool active = true;
    char buff[CHUNKSIZE];
    int fd;

    if ( (fd = open(filePath, O_RDONLY)) == -1 ){
     printf("Error while opening file: %s\n", filePath);
     exit(-1);
    }
    if  ((filzeSize = getFileSize(fd)) == -1) {
        printf("Error Encountered, exiting...\n");
        exit(-1); }
    totalNumRead = 0;
    while (active){
        if ((numRead = read(fd, buff, CHUNKSIZE)) == -1) {
            printf("Error while reading files.\n");
            exit(-1);
        }
        totalNumRead += numRead;
        if (totalNumRead == filzeSize) {
            active = false;
            if (pthread_mutex_lock(&numDeactivatedThreadsMutex) != 0 ){
                printf("Error Encountered, exiting...\n");
                exit(-1);}
            numDeactivatedThreadsInCurrIteration++;
            if (pthread_mutex_unlock(&numDeactivatedThreadsMutex) != 0){
                printf("Error Encountered, exiting...\n");
                exit(-1);}
        }
        // **CRITICAL SECTION BEGIN**
        if (pthread_mutex_lock(&shardBufferMutex) != 0){
            printf("Error Encountered, exiting...\n");
            exit(-1);}
        xorTwoBuffs(sharedResultBuff ,buff, numRead);
        globalXorCounter++;
        //update global var that hold max number of bytes read by *any* thread in current iteration:
        globalMaxNumReadInIteration = numRead > globalMaxNumReadInIteration ? numRead : globalMaxNumReadInIteration;
        if (globalXorCounter == numCurrentlyActiveThreads){ //i.e, is this the last thread to reach this point.
            globalXorCounter = 0;
            if (write(out_fd, sharedResultBuff, globalMaxNumReadInIteration) == -1 ) {
                printf("%s: Error while writing to outFile.\n", filePath);
                exit(-1);
            }
            globalMaxNumReadInIteration = 0; //reset
            for (int i=0; i<CHUNKSIZE; i++) { sharedResultBuff[i] = 0; } //reset shared buff before next iteration.
            //Update # of active threads for next iteration (done only by last thread):
            numCurrentlyActiveThreads -= numDeactivatedThreadsInCurrIteration;
            numDeactivatedThreadsInCurrIteration = 0;
            //wake up all other threads:
            if (pthread_cond_broadcast(&finishedIterationCV) != 0){
                printf("Error Encountered, exiting...\n");
                exit(-1);}
        } else { // i.e, this thread is not the last to xor, don't continue to next iteration untill last thread has finished.
            if (pthread_cond_wait(&finishedIterationCV, &shardBufferMutex) != 0){
                printf("Error Encountered, exiting...\n");
                exit(-1);}
        }
        if (pthread_mutex_unlock(&shardBufferMutex) != 0){
            printf("Error Encountered, exiting...\n");
            exit(-1);}
        // **CRITICAL SECTION END**

    }
    if (close(fd) != 0){
        printf("Error Encountered, exiting...\n");
        exit(-1);}
    pthread_exit(NULL);
}



int main(int argc, char **argv){
    char* ofp = argv[1];
    int i, outFileSize;
    pthread_t *threads;
    numInFiles = argc-2;
    numCurrentlyActiveThreads = numInFiles; //initialize
    if ( (out_fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 00777)) == -1){
        printf("Error Encountered, exiting...\n");
        exit(-1);}
    for (i=0; i<CHUNKSIZE; i++) { sharedResultBuff[i] = 0; } //first initialization
    if (pthread_mutex_init(&shardBufferMutex, NULL) != 0){
        printf("Error Encountered, exiting...\n");
        exit(-1);}
    if (pthread_mutex_init(&numDeactivatedThreadsMutex, NULL) != 0){
        printf("Error Encountered, exiting...\n");
        exit(-1);}

    if (pthread_cond_init(&finishedIterationCV, NULL) != 0){
        printf("Error Encountered, exiting...\n");
        exit(-1);}
    if ( (threads = malloc(numInFiles * sizeof(pthread_t*))) == NULL ){
        printf("Error Encountered, exiting...\n");
        exit(-1);}
    printf("Hello, creating %s from %d input files\n",ofp,numInFiles);
    for (i=2; i<argc; i++){
        if (pthread_create(&threads[i-2], NULL, threadWork, argv[i]) != 0){
            printf("Error Encountered, exiting...\n");
            exit(-1);}
    }
    for (i=0; i<numInFiles; i++){
        if (pthread_join(threads[i],NULL) != 0){
            printf("Error Encountered, exiting...\n");
            exit(-1);}
    }
    if ( (outFileSize = getFileSize(out_fd)) == -1 ){
        printf("Error Encountered, exiting...\n");
        exit(-1);}

    if (close(out_fd) != 0){
        printf("Error Encountered, exiting...\n");
        exit(-1);}
    printf("Created %s with size %d bytes\n", ofp, outFileSize);

    if (pthread_mutex_destroy(&shardBufferMutex) != 0){
        printf("Error Encountered, exiting...\n");
        exit(-1);}
    if (pthread_mutex_destroy(&numDeactivatedThreadsMutex) != 0){
        printf("Error Encountered, exiting...\n");
        exit(-1);}
    if (pthread_cond_destroy(&finishedIterationCV) != 0){
        printf("Error Encountered, exiting...\n");
        exit(-1);}

    free(threads);
    return 0;
}



