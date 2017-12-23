//
// Created by Yoav on 21-Dec-17.
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#define CHUNKSIZE 1024

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


int main(int argc, char **argv){
    char* ofp = argv[1];
    int numInFiles = argc-2;
    int outfile = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 00777); //TODO: check success & fix problem of when outfile already exists
    int* infiles = malloc((argc-1)*sizeof(int));
    for (int i=2; i<argc; i++){
        infiles[i-2] = open(argv[i], O_RDONLY); //TODO: check success
    }
    char buff[CHUNKSIZE];
    char result[CHUNKSIZE];
    char* finalRes = malloc(9); int k=0;//todo rm
    char* activeInFiles = malloc(numInFiles * sizeof(char));
    for (int i=0; i<numInFiles; i++) { activeInFiles[i] = 1; } // set all inFiles as active
    //for every inFile, read next chunk of data:
    int in_fd, numRead;
    bool anyBytesRead;
    while (anyActiveFile(activeInFiles, numInFiles)) {
        anyBytesRead = false;
        for (int i=0; i<CHUNKSIZE; i++) { result[i] = 0; }
        for (int i = 0; i < numInFiles; i++) {
            in_fd = infiles[i];
            if (!activeInFiles[i]) { //make sure we only read from files that were not finished.
                continue;
            }
            if ((numRead = read(in_fd, buff, CHUNKSIZE)) == -1) {
                printf("Error while reading files.\n");
                return -1;
            };
            if (numRead < CHUNKSIZE) {
                activeInFiles[i] = 0;
            }
            if (numRead > 0) { anyBytesRead = true; }
            xorTwoBuffs(result, buff, numRead);
        }
        if (anyBytesRead && write(outfile, result, CHUNKSIZE) == -1 ) {
            printf("Error while writing to outFile.\n");
            return -1;
        }

    }
    //
    free(infiles);
    free(activeInFiles);
    return 0;
}



