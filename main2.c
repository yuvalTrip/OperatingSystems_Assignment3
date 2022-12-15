#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include "utils.c"

#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"

#define SERVER_PATH_DGRAM "tpf_unix_sock.server_Dgram"

// The code for Stream UDS was adapted from https://www.ibm.com/docs/en/ztpf/1.1.0.15?topic=considerations-unix-domain-sockets
void UDS_Stream_Sender();

void UDS_Stream_Receiver();

void UDS_Dgram_Sender();

void UDS_Dgram_Receiver();

void mmapSenderAndReceiver();

void pipeTransfer();

void* threadFunction(void* data);

void threadSharingMainFunction();

int main() {
    if (fork() == 0) {
        UDS_Stream_Sender();
    } else {
        if (fork() == 0) {
            UDS_Stream_Receiver();
        } else {
            if (fork() == 0) {
                UDS_Dgram_Receiver();
            }
            else {
                if (fork() == 0) {
                    UDS_Dgram_Sender();
                } else {
                    if (fork() == 0) {
                        mmapSenderAndReceiver();
                    } else {
                        if (fork() == 0) {
                            pipeTransfer();
                        }
                        else {
                            if (fork() == 0) {
                                threadSharingMainFunction();
                            }
                            else {
                                wait(NULL);
                            }
                            wait(NULL);
                        }
                        wait(NULL);
                    }
                    wait(NULL);
                }
                wait(NULL);
            }
            wait(NULL);
        }
        wait(NULL);
    }
}


void UDS_Stream_Sender() {
    // The code was adapted from: https://www.ibm.com/docs/en/ztpf/1.1.0.15?topic=considerations-unix-domain-sockets
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
    strcpy(server_sockaddr.sun_path, SERVER_PATH);
    len = sizeof(server_sockaddr);

    unlink(SERVER_PATH);
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


    for (int i = 0; i < BUFFSIZE / 1024; i++) {
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

void UDS_Stream_Receiver() {
    // The code was adapted from: https://www.ibm.com/docs/en/ztpf/1.1.0.15?topic=considerations-unix-domain-sockets
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
//    rc = connect(client_sock, (struct sockaddr *) &server_sockaddr, len);
    while (connect(client_sock, (struct sockaddr *) &server_sockaddr, len) < 0) {
        continue;
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
    memset(buf, 0, BUFFSIZE);

    for (int i = 0; i < BUFFSIZE / 1024; i++) {
        char *tempBuff = malloc(1024);
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

void UDS_Dgram_Sender() {
    // The code was adapted from: https://www.ibm.com/docs/en/ztpf/1.1.0.15?topic=considerations-unix-domain-sockets
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
    strcpy(remote.sun_path, SERVER_PATH_DGRAM);

    /***************************************/
    /* Copy the data to be sent to the     */
    /* buffer and send it to the server.   */
    /***************************************/
    strcpy(buf, data);
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

void UDS_Dgram_Receiver() {
    // The code was adapted from: https://www.ibm.com/docs/en/ztpf/1.1.0.15?topic=considerations-unix-domain-sockets
    int server_sock, len, rc;
    int bytes_rec = 0;
    struct sockaddr_un server_sockaddr, peer_sock;
    char *buf = malloc(BUFFSIZE);
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(buf, 0, 256);

    /****************************************/
    /* Create a UNIX domain datagram socket */
    /****************************************/
    server_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (server_sock == -1) {
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
    strcpy(server_sockaddr.sun_path, SERVER_PATH_DGRAM);
    len = sizeof(server_sockaddr);
    unlink(SERVER_PATH_DGRAM);
    rc = bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
    if (rc == -1) {
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
    char *data = malloc(BUFFSIZE);
    for (int i = 0; i < BUFFSIZE / 1024; i++) {
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
    int checksum = sender(data, BUFFSIZE);
    //Take time after receive
    long endTime = ReturnTimeNs();

    int checksumReceived = receiver(trueData, BUFFSIZE, checksum);

    if (checksumReceived != 0) {
        endTime = -1;
    }
    printf("End time of UDS-Datagram: %ld\n", endTime);
    printf("The checksum difference for UDS-Datagram is: %d\n", checksumReceived);

    /*****************************/
    /* Close the socket and exit */
    /*****************************/
    close(server_sock);
}

void mmapSenderAndReceiver() {
    // The code was adapted from https://www.youtube.com/watch?v=F3z-SIxu1Tw
    int fd = open("mmapTest.txt", O_RDWR);
    char *data = malloc(BUFFSIZE);
    data = OpenAndReadFile(data);

    char *pmap = mmap(0, BUFFSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pmap == MAP_FAILED) {
        printf("Error");
    }

    printf("mmap before write time: %ld\n", ReturnTimeNs());
    strcpy(pmap, data); //copying data to new shared file

    if (fork() == 0) {
        char *receiveData = malloc(BUFFSIZE);
        strncpy(receiveData, pmap, BUFFSIZE);
        int senderChecksum = sender(data, BUFFSIZE);
        int checksumComparison = receiver(receiveData, BUFFSIZE, senderChecksum);

        if (checksumComparison != 0) {
            printf("end time of mmap is: -1\n");
        } else {
            printf("mmap after read time: %ld\n", ReturnTimeNs());
        }
        printf("Checksum difference for mmap is: %d\n", checksumComparison);

    } else {
        wait(NULL);
    }
}

void pipeTransfer() {
    // The code was adapted from https://www.tutorialspoint.com/inter_process_communication/inter_process_communication_pipes.htm
    int pipefds[2];
    int returnStatus;
    int pid;

    returnStatus = pipe(pipefds);
    if (returnStatus == -1) {
        printf("Unable to create pipe\n");
    }

    char *data = malloc(BUFFSIZE);
    data = OpenAndReadFile(data);

    pid = fork();
    if (pid == 0) { // Child process
        char *readMsg = malloc(BUFFSIZE);
        for (int i = 0; i < BUFFSIZE / 1024; i++) {
            char *currMsg = malloc(1024);
            read(pipefds[0], currMsg, 1024);
            strcat(readMsg, currMsg);
        }

        int senderChecksum = sender(data, BUFFSIZE);
        int checksumComparison = receiver(readMsg, BUFFSIZE, senderChecksum);

        if (checksumComparison != 0) {
            printf("end time of pipe transfer is: -1\n");
        } else {
            printf("pipe transfer after read time: %ld\n", ReturnTimeNs());
        }
        printf("Checksum difference for pipe transfer is: %d\n", checksumComparison);

    } else { //Parent process
        for (int i = 0; i < BUFFSIZE / 1024; i++) {
            write(pipefds[1], data, 1024);
            data += 1024;
        }
        wait(NULL);
    }
}

void threadSharingMainFunction() {
    // The code was adapted from https://www.geeksforgeeks.org/multithreading-c-2/
    char *data = malloc(BUFFSIZE);
    data = OpenAndReadFile(data);

    pthread_t thread_id;

    printf("The start time for thread memory sharing is: %ld\n", ReturnTimeNs());

    pthread_create(&thread_id, NULL, threadFunction, data);
    pthread_join(thread_id, NULL);
}

void* threadFunction(void* data) {
    int checksum = sender(data, BUFFSIZE);
    char* dataRead = malloc(BUFFSIZE);
    strcpy(dataRead, data);
    int checksumDifference = receiver(dataRead, BUFFSIZE, checksum);
    printf("The checksum difference for thread memory sharing is: %d\n", checksumDifference);
    if (checksumDifference != 0) {
        printf("The end time for thread memory sharing is: -1\n");
    }
    else {
        printf("The end time for thread memory sharing is: %ld\n", ReturnTimeNs());
    }
}