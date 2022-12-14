/************************************************************/
/* This is a stream socket server sample program for UNIX   */
/* domain sockets. This program listens for a connection    */
/* from a client program, accepts it, reads data from the   */
/* client, then sends data back to connected UNIX socket.   */
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../utils.c"

#define SOCK_PATH  "/home/appeldaniel/CLionProjects/OperatingSystems_Assignment3/cmake-build-debug/tpf_unix_sock.server"

int main() {

    int server_sock, client_sock, len, rc;
    int bytes_rec = 0;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;
    char buf[256];
    int backlog = 10;
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(buf, 0, 256);

    /**************************************/
    /* Create a UNIX domain stream socket */
    /**************************************/
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1) {
        printf("SOCKET ERROR\n");
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
    if (rc == -1) {
        printf("BIND ERROR:\n");
        close(server_sock);
        exit(1);
    }

    /*********************************/
    /* Listen for any client sockets */
    /*********************************/
    rc = listen(server_sock, backlog);
    if (rc == -1) {
        printf("LISTEN ERROR:\n");
        close(server_sock);
        exit(1);
    }
    printf("socket listening...\n");

    /*********************************/
    /* Accept an incoming connection */
    /*********************************/
    client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, &len);
    if (client_sock == -1) {
        printf("ACCEPT ERROR:\n");
        close(server_sock);
        close(client_sock);
        exit(1);
    }

    /****************************************/
    /* Get the name of the connected socket */
    /****************************************/
    len = sizeof(client_sockaddr);
    rc = getpeername(client_sock, (struct sockaddr *) &client_sockaddr, &len);
    if (rc == -1) {
        printf("GETPEERNAME ERROR:\n");
        close(server_sock);
        close(client_sock);
        exit(1);
    } else {
        printf("Client socket filepath: %s\n", client_sockaddr.sun_path);
    }


    /******************************************/
    /* Send data back to the connected socket */
    /******************************************/
    char *data = malloc(BUFFSIZE * sizeof(char));
    memset(data, 0, BUFFSIZE);
    data = OpenAndReadFile(data);

    //Take time before send
    long startTime = ReturnTimeNs();
    printf("Start time of UDS-Stream: %ld\n", startTime);
//    int checksum = sender(data, BUFFSIZE);


    for (int i = 0; i < BUFFSIZE/1024; i++) {
        rc = send(client_sock, data, 1024, 0);
        if (rc == -1) {
            printf("SEND ERROR:");
            close(server_sock);
            close(client_sock);
            exit(1);
        } else {
            data += 1024;
        }
    }

//    //Take time after send
//    long endTime = ReturnTimeNs();
//
//    printf("The time of sending was: %ld\n", endTime-startTime);

    /******************************/
    /* Close the sockets and exit */
    /******************************/
    close(server_sock);
    close(client_sock);
}