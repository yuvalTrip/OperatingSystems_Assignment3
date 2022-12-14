/************************************************************/
/* This is a stream socket client sample program for UNIX   */
/* domain sockets. This program creates a socket, connects  */
/* to a server, sends data, then receives and prints a      */
/* message from the server.                                 */
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../utils.c"

#define SERVER_PATH "/home/appeldaniel/CLionProjects/OperatingSystems_Assignment3/cmake-build-debug/tpf_unix_sock.server"
#define CLIENT_PATH "/home/appeldaniel/CLionProjects/OperatingSystems_Assignment3/cmake-build-debug/tpf_unix_sock.client"

void UDS_Stream_Receiver() {

    int client_sock, rc, len;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;
    char *buf = malloc(BUFFSIZE * sizeof(char));
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));

    /**************************************/
    /* Create a UNIX domain stream socket */
    /**************************************/
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock == -1) {
        printf("SOCKET ERROR =\n");
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
    client_sockaddr.sun_family = AF_UNIX;
    strcpy(client_sockaddr.sun_path, CLIENT_PATH);
    len = sizeof(client_sockaddr);

    unlink(CLIENT_PATH);
    rc = bind(client_sock, (struct sockaddr *) &client_sockaddr, len);
    if (rc == -1) {
        printf("BIND ERROR:\n");
        close(client_sock);
        exit(1);
    }

    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* for the server socket and connect   */
    /* to it.                              */
    /***************************************/
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SERVER_PATH);
    rc = connect(client_sock, (struct sockaddr *) &server_sockaddr, len);
    if (rc == -1) {
        printf("CONNECT ERROR =\n");
        close(client_sock);
        exit(1);
    }


    char *data = malloc(BUFFSIZE * sizeof(char));
    memset(data, 0, BUFFSIZE);
    data = OpenAndReadFile(data);

//    //Take time before receive
//    long startTime = ReturnTimeNs();

    /**************************************/
    /* Read the data sent from the server */
    /* and print it.                      */
    /**************************************/
    int checksum;
    printf("Waiting to receive data...\n");
    memset(buf, 0, BUFFSIZE);

    for(int i = 0; i < BUFFSIZE/1024; i++) {
        char* tempBuff = malloc(1024);
        rc = recv(client_sock, tempBuff, 1024, 0);
        if (rc == -1) {
            printf("RECV ERROR =\n");
            close(client_sock);
            exit(1);
        } else {
            strcat(buf, tempBuff);
            free(tempBuff);
        }
    }
    checksum = sender(data, BUFFSIZE);
    //Take time after receive
    long endTime = ReturnTimeNs();

    int checksumReceived = receiver(buf, BUFFSIZE, checksum);

    if (checksumReceived != 0) {
        endTime = -1;
    }
    printf("End time of UDS-Stream: %ld\n", endTime);
    printf("The checksum difference is: %d\n", checksumReceived);

    /******************************/
    /* Close the socket and exit. */
    /******************************/
    close(client_sock);
    free(buf);
}