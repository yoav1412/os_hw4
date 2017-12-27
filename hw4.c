#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define CHUNKSIZE 1412 //todo: change to 1024

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

    //printf("in thread %d\n\tfileName = %s\n", (int) pthread_self(), filePath);//TODO rm
    //printf("\t%s: numInFiles = %d\n",filePath, numInFiles);//TODO rm
    if ( (fd = open(filePath, O_RDONLY)) == -1 ){
     printf("Error while opening file: %s\n", filePath);
     exit(-1);
    }
    if  ((filzeSize = getFileSize(fd)) == -1) { exit(-1); }
    totalNumRead = 0;
    while (active){
        //printf("\t*****\n");
        if ((numRead = read(fd, buff, CHUNKSIZE)) == -1) {
            printf("Error while reading files.\n");
            exit(-1);
        }
        totalNumRead += numRead;
        //printf("\t%s: just read %d bytes | buff = %s | totalRead = %d \n",filePath, numRead, buff, totalNumRead);
        if (totalNumRead == filzeSize) {
            active = false;
            //printf("\t%s: deactivated with totalRead. NO LONGER ACTIVE\n",filePath);

            if (pthread_mutex_lock(&numDeactivatedThreadsMutex) != 0 ){exit(-1);}
            numDeactivatedThreadsInCurrIteration++;
            if (pthread_mutex_unlock(&numDeactivatedThreadsMutex) != 0){exit(-1);}
        }

        //if (numRead == 0) { continue; } TODO: really dont need this line?
        // **CRITICAL SECTION BEGIN**
        if (pthread_mutex_lock(&shardBufferMutex) != 0){exit(-1);}

        //printf("\t%s: now xoring: sharedResultBuff (before)=%s | buff = %s\n", filePath, sharedResultBuff, buff);
        xorTwoBuffs(sharedResultBuff ,buff, numRead);
        //printf("\t%s: sharedResultBuff (after xor)=%s | buff = %s\n", filePath, sharedResultBuff, buff);

        //printf("\t%s: globalXorCounter (before inc) = %d\n",filePath,globalXorCounter);
        globalXorCounter++;
        //printf("\t%s: globalXorCounter (after inc) = %d | activeThrds = %d \n",filePath,globalXorCounter, numCurrentlyActiveThreads);
        //update global var that hold max number of bytes read by any thread in current iteration.
        globalMaxNumReadInIteration = numRead > globalMaxNumReadInIteration ? numRead : globalMaxNumReadInIteration;
        if (globalMaxNumReadInIteration != CHUNKSIZE) printf("\t%s: globalMaxNumReadInIteration (after update) = %d \n",filePath,globalMaxNumReadInIteration);//todo rm
        if (globalXorCounter == numCurrentlyActiveThreads){ //i.e, is this the last thread to reach this point.
            //printf("\t%s: last one. will write to outfile.\n",filePath);
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
            //printf("\t%s: updated #activeThreads for next iter: #active=%d\n", filePath, numCurrentlyActiveThreads);


            //wake up all other threads:
            //printf("\t%s: Broadcasting\n", filePath); //todo rm
            if (pthread_cond_broadcast(&finishedIterationCV) != 0){exit(-1);}
        } else { // i.e, this thread is not the last to xor, don't continue to next iteration untill last thread has finished.
            //printf("\t%s: going to sleep on CV\n", filePath); // todo rm
            if (pthread_cond_wait(&finishedIterationCV, &shardBufferMutex) != 0){exit(-1);}
            //printf("\t%s: Woke up!\n", filePath); //todo rm
        }
        if (pthread_mutex_unlock(&shardBufferMutex) != 0){exit(-1);}
        // **CRITICAL SECTION END**

    }
    if (close(fd) != 0){exit(-1);}
    printf("\t%s: Finished.\n", filePath);
    pthread_exit(NULL); //todo; is this ok?
}



int main(int argc, char **argv){

    //printf("tst=%c | %d | %s", tst, tst, &tst);

    char* ofp = argv[1];
    int i, outFileSize;
    pthread_t *threads;
    numInFiles = argc-2;
    numCurrentlyActiveThreads = numInFiles; //initialize
    if ( (out_fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 00777)) == -1){exit(-1);}
    for (i=0; i<CHUNKSIZE; i++) { sharedResultBuff[i] = 0; } //first initialization
    if (pthread_mutex_init(&shardBufferMutex, NULL) != 0){exit(-1);}
    if (pthread_mutex_init(&numDeactivatedThreadsMutex, NULL) != 0){exit(-1);}

    if (pthread_cond_init(&finishedIterationCV, NULL) != 0){exit(-1);}
    if ( (threads = malloc(numInFiles * sizeof(pthread_t*))) == NULL ){exit(-1);}
    printf("Hello, creating %s from %d input files\n",ofp,numInFiles);
    for (i=2; i<argc; i++){
        if (pthread_create(&threads[i-2], NULL, threadWork, argv[i]) != 0){exit(-1);}
    }
    for (i=0; i<numInFiles; i++){
        if (pthread_join(threads[i],NULL) != 0){exit(-1);}
    }
    if ( (outFileSize = getFileSize(out_fd)) == -1 ){exit(-1);}
    printf("Created %s with size %d bytes\n", ofp, outFileSize);

    if (pthread_mutex_destroy(&shardBufferMutex) != 0){exit(-1);}
    if (pthread_mutex_destroy(&numDeactivatedThreadsMutex) != 0){exit(-1);}
    if (pthread_cond_destroy(&finishedIterationCV) != 0){exit(-1);}

    free(threads);
    if (close(out_fd) != 0){exit(-1);}
    return 0;
}



