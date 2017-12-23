#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define CHUNKSIZE 1024 //todo: change to 1024

pthread_mutex_t shardBufferMutex, numDeactivatedThreadsMutex;
int out_fd, globalXorCounter, numInFiles, numCurrentlyActiveThreads, numDeactivatedThreadsInCurrIteration = 0;
char sharedResultBuff[CHUNKSIZE];
pthread_cond_t finishedIterationCV;

 //TODO: check ret values of pthreacreate, mutex_init, etx..
//TODO: destroy mutex and cv
//TODO: fix balagan with multiple numRead>0 checks etc.
/*
 * assuming size(buff2)<=size(buff1)
 */
void xorTwoBuffs(char buff1[CHUNKSIZE], char buff2[CHUNKSIZE], int size2){ //TODO: does it make sense to use char*'s??
    for (int i=0;i<size2; i++){
        buff1[i] ^= buff2[i];
    }
}

bool anyActiveFile(char* activeFiles, int numInFiles){ //todo: rm?
   bool res = false;
   for (int i=0; i<numInFiles; i++){
       if (activeFiles[i] == 1) {
           res = true;
       }
   }
   return res;
}

int getFileSize(int fd){
    int currPos = lseek(fd, 0, SEEK_CUR);
    int size = lseek(fd,0,SEEK_END);
    lseek(fd, currPos, SEEK_SET);
    return size;
}

void* threadWork(void* filePathParam){ //todo rename..
    char *filePath = (char*) filePathParam;
    int numRead,totalNumRead, filzeSize;
    bool active = true;
    char buff[CHUNKSIZE];
    int fd;

    //printf("in thread %d\n\tfileName = %s\n", (int) pthread_self(), filePath);//TODO rm
    //printf("\t%s: numInFiles = %d\n",filePath, numInFiles);//TODO rm
    if ( (fd = open(filePath, O_RDONLY)) == -1 ){
     printf("Error while opening file\n");
        //todo: exit from thread and pass err somehow..
    }
    filzeSize = getFileSize(fd);
    totalNumRead = 0;
    while (active){
        //printf("\t*****\n");
        if ((numRead = read(fd, buff, CHUNKSIZE)) == -1) {
            printf("Error while reading files.\n");
            //todo: exit from thread and pass err somehow..
        }
        totalNumRead += numRead;
        //printf("\t%s: just read %d bytes | buff = %s | totalRead = %d \n",filePath, numRead, buff, totalNumRead);
        if (totalNumRead == filzeSize) {
            active = false;
            //printf("\t%s: deactivated with totalRead. NO LONGER ACTIVE\n",filePath);

            pthread_mutex_lock(&numDeactivatedThreadsMutex);
            numDeactivatedThreadsInCurrIteration++;
            pthread_mutex_unlock(&numDeactivatedThreadsMutex);

        }
        if (numRead == 0) { continue; }
        // **CRITICAL SECTION BEGIN**
        pthread_mutex_lock(&shardBufferMutex);

        //printf("\t%s: now xoring: sharedResultBuff (before)=%s | buff = %s\n", filePath, sharedResultBuff, buff);
        xorTwoBuffs(sharedResultBuff ,buff, numRead);
        //printf("\t%s: sharedResultBuff (after xor)=%s | buff = %s\n", filePath, sharedResultBuff, buff);

        //printf("\t%s: globalXorCounter (before inc) = %d\n",filePath,globalXorCounter);
        globalXorCounter++;
        //printf("\t%s: globalXorCounter (after inc) = %d | activeThrds = %d \n",filePath,globalXorCounter, numCurrentlyActiveThreads);

        if (globalXorCounter == numCurrentlyActiveThreads){ //i.e, is this the last thread to reach this point.
            //printf("\t%s: last one. will write to outfile.\n",filePath);
            globalXorCounter = 0;
            if (write(out_fd, sharedResultBuff, CHUNKSIZE) == -1 ) {
                //printf("%s: Error while writing to outFile.\n", filePath);
                //todo: exit from thread and pass err somehow..
            }

            for (int i=0; i<CHUNKSIZE; i++) { sharedResultBuff[i] = 0; } //reset shared buff before next iteration.

            //Update # of active threads for next iteration (done only by last thread):
            numCurrentlyActiveThreads -= numDeactivatedThreadsInCurrIteration;
            numDeactivatedThreadsInCurrIteration = 0;
            //printf("\t%s: updated #activeThreads for next iter: #active=%d\n", filePath, numCurrentlyActiveThreads);


            //wake up all other threads:
            //printf("\t%s: Broadcasting\n", filePath);
            pthread_cond_broadcast(&finishedIterationCV);
        } else { // i.e, this thread is not the last to xor, don't continue to next iteration untill last thread has finished.
            //printf("\t%s: going to sleep on CV\n", filePath);
            pthread_cond_wait(&finishedIterationCV, &shardBufferMutex);
            //printf("\t%s: Woke up!\n", filePath);
        }
        pthread_mutex_unlock(&shardBufferMutex);
        // **CRITICAL SECTION END**

    }
    printf("\t%s: Finished.\n", filePath);
    pthread_exit(NULL); //todo; is this ok?
}



int main(int argc, char **argv){

    char tst;
    tst = (char) 100;
    //printf("tst=%c | %d | %s", tst, tst, &tst);

    char* ofp = argv[1];
    int i;
    numInFiles = argc-2;
    numCurrentlyActiveThreads = numInFiles; //initialize
    out_fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 00777); //TODO: ok to use 777?
    int* infiles = malloc((argc-1)*sizeof(int));
    for (i=0; i<CHUNKSIZE; i++) { sharedResultBuff[i] = 0; } //first initialization
    pthread_mutex_init(&shardBufferMutex, NULL);
    pthread_mutex_init(&numDeactivatedThreadsMutex, NULL);

    pthread_cond_init(&finishedIterationCV, NULL);
    pthread_t *threads = malloc(numInFiles * sizeof(pthread_t*));
    printf("Hello, creating %s from %d input files\n",ofp,numInFiles);
    for (i=2; i<argc; i++){
        pthread_create(&threads[i-2], NULL, threadWork, argv[i]); //todo: should &attr parameter be null? or like in example code from recit8?
    }
    for (i=0; i<numInFiles; i++){
        pthread_join(threads[i],NULL);
    }
    printf("Created %s with size %d bytes\n", ofp, getFileSize(out_fd));


    free(infiles);
    close(out_fd);
    return 0;
}



