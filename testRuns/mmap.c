#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/wait.h>
#include "../utils.c"

void mmapSenderAndReceiver(){
    int fd = open("mmapTest.txt", O_RDWR);
    char *data = malloc(BUFFSIZE);
    data = OpenAndReadFile(data);

    char* pmap = mmap(0, BUFFSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pmap == MAP_FAILED) {
        printf("Error");
    }

    printf("mmap before write time: %ld", ReturnTimeNs());
    strcpy(pmap, data); //copying data to new shared file

    if(fork() == 0) {
        char* receiveData = malloc(BUFFSIZE);
        strncpy(receiveData, pmap, BUFFSIZE);
        int senderChecksum = sender(data, BUFFSIZE);
        int checksumComparison = receiver(receiveData, BUFFSIZE, senderChecksum);

        if (checksumComparison != 0) {
            printf("end time of mmap is: -1\n");
        }
        else {
            printf("mmap after read time: %ld\n", ReturnTimeNs());
        }
        printf("Checksum difference for mmap is: %d\n", checksumComparison);

    }
    else {
        wait(NULL);
    }
}
