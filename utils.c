#include <stdio.h>
#include<stdio.h>
#include<math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
# include <time.h>

#define BUFFSIZE 1048576 // define file size in bytes (binary)

// The random data file was constructed using a linux command following the link:
// https://www.computernetworkingnotes.com/linux-tutorials/generate-or-create-a-large-text-file-in-linux.html

/*
 * C program to implement Checksum
 */
//Most of the code was adapted from :
//https://www.sanfoundry.com/c-program-implement-checksum-method/
int sender(char *recievedData, int numOfChar) {
    int checksum, sum = 0, i;
    for (i = 0; i < numOfChar; i++)
        sum += recievedData[i];
    checksum = ~sum;    //1's complement of sum
    return checksum;
}

int receiver(char *recievedData, int numOfChar, int SenderChecksum) {
    int checksum, sum = 0, i;
    for (i = 0; i < numOfChar; i++)
        sum += recievedData[i];
    sum = sum + SenderChecksum;
    checksum = ~sum;    //1's complement of sum
    return checksum;
}

char *OpenAndReadFile(char *buf) {
    int scrFile = 0;
    // let us open the input file
    scrFile = open("file.txt", O_RDONLY);
    if (scrFile > 0) { // there are things to read from the input
        int succe = read(scrFile, buf, BUFFSIZE);
        close(scrFile);
    }
    return buf;
}

//function measure time and return value in nanoSec
// The function was adapted from: https://www.educba.com/clock_gettime/
long ReturnTimeNs() {
    struct timespec currTime;

    if (clock_gettime(CLOCK_REALTIME, &currTime) == -1) {
        perror("clock gettime");
        return EXIT_FAILURE;
    }
    return currTime.tv_nsec;
}