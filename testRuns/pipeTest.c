#include<stdio.h>
#include<unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "../utils.c"

void pipeTransfer() {
    int pipefds[2];
    int returnStatus;
    int pid;

    returnStatus = pipe(pipefds);
    if (returnStatus == -1) {
        printf("Unable to create pipe\n");
    }

    int fd = open("mmapTest.txt", O_RDWR);
    char *data = malloc(BUFFSIZE);
    data = OpenAndReadFile(data);

    pid = fork();
    if (pid == 0) { // Child process
        char *readMsg = malloc(BUFFSIZE);
        for (int i = 0; i < BUFFSIZE/1024; i++) {
            char *currMsg = malloc(1024);
            read(pipefds[0], currMsg, 1024);
            strcat(readMsg, currMsg);
        }

        int senderChecksum = sender(data, BUFFSIZE);
        int checksumComparison = receiver(readMsg, BUFFSIZE, senderChecksum);

        if (checksumComparison != 0) {
            printf("end time of pipe transfer is: -1\n");
        }
        else {
            printf("pipe transfer after read time: %ld\n", ReturnTimeNs());
        }
        printf("Checksum difference for pipe transfer is: %d\n", checksumComparison);

    } else { //Parent process
        for (int i = 0; i < BUFFSIZE/1024; i++) {
            write(pipefds[1], data, 1024);
            data += 1024;
        }
        wait(NULL);
    }
}
