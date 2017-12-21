#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define CHUNKSIZE 1 //todo: change to 1024

pthread_mutex_t shardBufferMutex;
int out_fd, globalXorCounter, numInFiles, numCurrentlyActiveThreads;
bool anyBytesRead;
char sharedResultBuff[CHUNKSIZE];
pthread_cond_t finishedIterationCV;

 //TODO: check ret values of pthreacreate, mutex_init, etx..
//TODO: destroy mutex and cv
/*
 * assuming size(buff2)<=size(buff1)
 */
void xorTwoBuffs(char buff1[CHUNKSIZE], char buff2[CHUNKSIZE], int size2){ //TODO: does it make sense to use char*'s??
    for (int i=0;i<size2; i++){
        buff1[i] ^= buff2[i];
    }
}

bool anyActiveFile(char* activeFiles, int numInFiles){
   bool res = false;
   for (int i=0; i<numInFiles; i++){
       if (activeFiles[i] == 1) {
           res = true;
       }
   }
   return res;
}


void* threadWork(void* filePathParam){ //todo rename..
    char *filePath = (char*) filePathParam;
    int numRead;
    bool active = true;
    char buff[CHUNKSIZE];
    int fd;
    printf("in thread %d\n\tfileName = %s\n", (int) pthread_self(), filePath);//TODO rm
    printf("\tnumInFiles = %d\n", numInFiles);//TODO rm
    if ( (fd = open(filePath, O_RDONLY)) == -1 ){
     printf("Error while opening file\n");
        //todo: exit from thread and pass err somehow..
    }
    while (active){
        printf("\t*****\n");
        if ((numRead = read(fd, buff, CHUNKSIZE)) == -1) {
            printf("Error while reading files.\n");
            //todo: exit from thread and pass err somehow..
        }
        printf("\tjust read %d bytes. | buff = %s \n", numRead, buff);
        if (numRead < CHUNKSIZE) {
            active = false;
            printf("\tNO LONGER ACTIVE\n");
            numCurrentlyActiveThreads--;//TODO: NOT THREAD SAFE!
        }
        // **CRITICAL SECTION BEGIN**
        pthread_mutex_lock(&shardBufferMutex);
        if (numRead > 0) { anyBytesRead = true; }
        //if (numRead == 0) { continue; }
        if (numRead > 0) {
            xorTwoBuffs(sharedResultBuff ,buff, numRead);
        }

        printf("\tglobalXorCounter (befire inc) = %d\n",globalXorCounter);
        if (numRead > 0) {
            globalXorCounter++;
        }
        printf("\tglobalXorCounter (after inc) = %d | activeThrds = %d \n",globalXorCounter, numCurrentlyActiveThreads);
        if (globalXorCounter == numCurrentlyActiveThreads){
            printf("\twill try to write to outfile (anyBytesRead=%d).\n", anyBytesRead);
            globalXorCounter = 0;
            if ( anyBytesRead && write(out_fd, sharedResultBuff, CHUNKSIZE) == -1 ) {
                printf("Error while writing to outFile.\n");
                //todo: exit from thread and pass err somehow..
            }
            anyBytesRead = false;
            //notify
            printf("\tBroadcasting\n");
            pthread_cond_broadcast(&finishedIterationCV);
        } else {
            // if this thread is not the last to xor, don't continue to next iteration untill last thread has finished.
            pthread_cond_wait(&finishedIterationCV, &shardBufferMutex);
        }
        pthread_mutex_unlock(&shardBufferMutex);
        // **CRITICAL SECTION END**
    }
    pthread_exit(NULL); //todo; is this ok?
}



int main(int argc, char **argv){
    char* ofp = argv[1];
    int i;
    numInFiles = argc-2;
    numCurrentlyActiveThreads = numInFiles; //initialize
    out_fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, S_IRWXO); //TODO: check success & fix problem of when outfile already exists. //todo: verify that S_IRWXO is the correct flag
    int* infiles = malloc((argc-1)*sizeof(int));

    pthread_mutex_init(&shardBufferMutex, NULL);
    pthread_cond_init(&finishedIterationCV, NULL);
    int in_fd, numRead;
    bool anyBytesRead;
    pthread_t *threads = malloc(numInFiles * sizeof(pthread_t*));
    printf("Hello, creating %s from %d input files\n",ofp,numInFiles);
    for (i=2; i<argc; i++){
        pthread_create(&threads[i-2], NULL, threadWork, argv[i]); //todo: should &attr parameter be null? or like in example code from recit8?
    }
    for (i=0; i<numInFiles; i++){
        pthread_join(threads[i],NULL);
    }
    printf("Created %s with size %d bytes\n", ofp, 999);//TODO: get num of written bytes..


    free(infiles);
    close(out_fd);
    return 0;
}



