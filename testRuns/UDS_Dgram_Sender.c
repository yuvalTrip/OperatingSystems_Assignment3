/************************************************************/
/* This is a datagram socket client sample program for UNIX */
/* domain sockets. This program creates a socket and sends  */
/* data to a server.                                        */
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../utils.c"

#define SERVER_PATH "/home/appeldaniel/CLionProjects/OperatingSystems_Assignment3/testRuns/tpf_unix_sock.server"

void UDS_Dgram_Sender () {
    int client_socket, rc;
    struct sockaddr_un remote;
    char *buf = malloc(BUFFSIZE);
    char *data = malloc(BUFFSIZE);
    data = OpenAndReadFile(data);

    //Take time before send
    long startTime = ReturnTimeNs();
    printf("Start time of UDS-Stream: %ld\n", startTime);
    int checksum = sender(data, BUFFSIZE);

    memset(&remote, 0, sizeof(struct sockaddr_un));

    /****************************************/
    /* Create a UNIX domain datagram socket */
    /****************************************/
    client_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        printf("SOCKET ERROR = %s\n", strerror(errno));
        exit(1);
    }

    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* by using AF_UNIX for the family and */
    /* giving it a filepath to send to.    */
    /***************************************/
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SERVER_PATH);

    /***************************************/
    /* Copy the data to be sent to the     */
    /* buffer and send it to the server.   */
    /***************************************/
    strcpy(buf, data);
    printf("Sending data...\n");
    buf[BUFFSIZE] = '\0';
    char *startPointer = buf;
    for (int i = 0; i < BUFFSIZE / 1024; i++) {
        char *tempSend = malloc(1024);
        strncpy(tempSend, startPointer, 1024);
        rc = sendto(client_socket, tempSend, 1024, 0, (struct sockaddr *) &remote, sizeof(remote));
        if (rc == -1) {
            printf("SENDTO ERROR = %s\n", strerror(errno));
            close(client_socket);
            exit(1);
        } else {
            startPointer += 1024;
        }
        free(tempSend);
    }

    /*****************************/
    /* Close the socket and exit */
    /*****************************/
    rc = close(client_socket);
}