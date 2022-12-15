#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <string.h>
#include "../utils.c"

void* threadFunction(void* data);

void threadSharingMainFunction() {
    char *data = malloc(BUFFSIZE);
    data = OpenAndReadFile(data);

    pthread_t thread_id;

    printf("The start time for thread memory sharing is: %ld\n", ReturnTimeNs());

    pthread_create(&thread_id, NULL, threadFunction, data);
    pthread_join(thread_id, NULL);
    exit(0);
}

void* threadFunction(void* data) {
    int checksum = sender(data, BUFFSIZE);
    char* dataRead = malloc(BUFFSIZE);
    strcpy(dataRead, data);
    int checksumDifference = receiver(dataRead, BUFFSIZE, checksum);
    printf("The end time for thread memory sharing is: %ld\n", ReturnTimeNs());
    printf("The checksum difference for thread memory sharing is: %d\n", checksumDifference);
}