/************************************************************/
/* This is a datagram socket server sample program for UNIX */
/* domain sockets. This program creates a socket and        */
/* receives data from a client.                             */
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../utils.c"

#define SOCK_PATH "/home/appeldaniel/CLionProjects/OperatingSystems_Assignment3/testRuns/tpf_unix_sock.server"

void UDS_Dgram_Receiver(){

    int server_sock, len, rc;
    int bytes_rec = 0;
    struct sockaddr_un server_sockaddr, peer_sock;
    char* buf = malloc (BUFFSIZE);
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(buf, 0, 256);

    /****************************************/
    /* Create a UNIX domain datagram socket */
    /****************************************/
    server_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (server_sock == -1){
        printf("SOCKET ERROR = %s\n", strerror(errno));
        exit(1);
    }

    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* by using AF_UNIX for the family and */
    /* giving it a filepath to bind to.    */
    /*                                     */
    /* Unlink the file so the bind will    */
    /* succeed, then bind to that file.    */
    /***************************************/
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SOCK_PATH);
    len = sizeof(server_sockaddr);
    unlink(SOCK_PATH);
    rc = bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
    if (rc == -1){
        printf("BIND ERROR = %s\n", strerror(errno));
        close(server_sock);
        exit(1);
    }
    char *trueData = malloc(BUFFSIZE * sizeof(char));
    memset(trueData, 0, BUFFSIZE);
    trueData = OpenAndReadFile(trueData);

    /****************************************/
    /* Read data on the server from clients */
    /* and print the data that was read.    */
    /****************************************/
    char* data = malloc(BUFFSIZE);
    printf("waiting to recvfrom...\n");
    for (int i = 0; i < BUFFSIZE/1024; i++) {
        bytes_rec = recvfrom(server_sock, buf, 1024, 0, (struct sockaddr *) &peer_sock, &len);
        if (bytes_rec == -1) {
            printf("RECVFROM ERROR = %s\n", strerror(errno));
            close(server_sock);
            exit(1);
        } else {
//            printf("%s", buf);
            strcat(data, buf);
        }
    }
    printf("%s", data);
    printf("\n");
    int checksum = sender(data, BUFFSIZE);
    //Take time after receive
    long endTime = ReturnTimeNs();

    int checksumReceived = receiver(trueData, BUFFSIZE, checksum);

    if (checksumReceived != 0) {
        endTime = -1;
    }
    printf("End time of UDS-Stream: %ld\n", endTime);
    printf("The checksum difference is: %d\n", checksumReceived);

    /*****************************/
    /* Close the socket and exit */
    /*****************************/
    close(server_sock);
}