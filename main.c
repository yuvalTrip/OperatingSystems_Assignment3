#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>


#define SIZE_IN_BYTES 104857600
#define CHUNK_SIZE 1024
#define FILENAME "file.txt"
#define RECEIVER_PORT 2001
#define IPADDR_TCP "127.0.0.1"
#define IPADDR_UDP "::1"
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
#define SERVER_PATH_DGRAM "tpf_unix_sock.server_Dgram"


long Endtime;
long Starttime;

// The code for Stream UDS was adapted from https://www.ibm.com/docs/en/ztpf/1.1.0.15?topic=considerations-unix-domain-sockets
void UDS_Stream_Sender();

void UDS_Stream_Receiver();

void UDS_Dgram_Sender();

void UDS_Dgram_Receiver();

void mmapSenderAndReceiver();

void pipeTransfer();

void* threadFunction(void* data);

void threadSharingMainFunction();

int process_senderTCP();

int process_recieverTCP();

int process_senderUDP();

int process_recieverUDP ();

int main(int argc, char * argv[]) {
    if(fork() == 0)
    {
        process_senderTCP();
    }
    else
    {
        if(fork() == 0)
        {
            process_recieverTCP();
        }
        else
        {
            if(fork() == 0)
            {
                process_senderUDP();
            }
            else
            {
                if(fork() == 0)
                {
                    process_recieverUDP();
                }
                else
                {
                    if (fork() == 0) {
                        UDS_Stream_Sender();
                    }
                    else {
                        if (fork() == 0) {
                            UDS_Stream_Receiver();
                        }
                        else {
                            if (fork() == 0) {
                                UDS_Dgram_Sender();
                            }
                            else {
                                if (fork() == 0) {
                                    UDS_Dgram_Receiver();
                                }
                                else {
                                    if (fork() == 0) {
                                        mmapSenderAndReceiver();
                                    }
                                    else {
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
                    wait(NULL);
                }
                wait(NULL);
            }
            wait(NULL);
        }
        wait(NULL);
    }

    return 1;
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
    char *data = malloc(SIZE_IN_BYTES * sizeof(char));
    memset(data, 0, SIZE_IN_BYTES);
    data = OpenAndReadFile(data);

    //Take time before send
    long startTime = ReturnTimeNs();
    printf("Start time of UDS-Stream: %ld\n", startTime);
//    int checksum = sender(data, BUFFSIZE);


    for (int i = 0; i < SIZE_IN_BYTES / 1024; i++) {
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
    char *buf = malloc(SIZE_IN_BYTES * sizeof(char));
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


    char *data = malloc(SIZE_IN_BYTES * sizeof(char));
    memset(data, 0, SIZE_IN_BYTES);
    data = OpenAndReadFile(data);

//    //Take time before receive
//    long startTime = ReturnTimeNs();

    /**************************************/
    /* Read the data sent from the server */
    /* and print it.                      */
    /**************************************/
    int checksum;
    memset(buf, 0, SIZE_IN_BYTES);

    for (int i = 0; i < SIZE_IN_BYTES / 1024; i++) {
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
    checksum = sender(data, SIZE_IN_BYTES);
    //Take time after receive
    long endTime = ReturnTimeNs();

    int checksumReceived = receiver(buf, SIZE_IN_BYTES, checksum);

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
    char *buf = malloc(SIZE_IN_BYTES);
    char *data = malloc(SIZE_IN_BYTES);
    data = OpenAndReadFile(data);

    //Take time before send
    long startTime = ReturnTimeNs();
    printf("Start time of UDS-Stream: %ld\n", startTime);
    int checksum = sender(data, SIZE_IN_BYTES);

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
    buf[SIZE_IN_BYTES] = '\0';
    char *startPointer = buf;
    for (int i = 0; i < SIZE_IN_BYTES / 1024; i++) {
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
    char *buf = malloc(SIZE_IN_BYTES);
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
    char *trueData = malloc(SIZE_IN_BYTES * sizeof(char));
    memset(trueData, 0, SIZE_IN_BYTES);
    trueData = OpenAndReadFile(trueData);

    /****************************************/
    /* Read data on the server from clients */
    /* and print the data that was read.    */
    /****************************************/
    char *data = malloc(SIZE_IN_BYTES);
    for (int i = 0; i < SIZE_IN_BYTES / 1024; i++) {
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
    int checksum = sender(data, SIZE_IN_BYTES);
    //Take time after receive
    long endTime = ReturnTimeNs();

    int checksumReceived = receiver(trueData, SIZE_IN_BYTES, checksum);

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
    char *data = malloc(SIZE_IN_BYTES);
    data = OpenAndReadFile(data);

    char *pmap = mmap(0, SIZE_IN_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pmap == MAP_FAILED) {
        printf("Error");
    }

    printf("mmap before write time: %ld\n", ReturnTimeNs());
    strcpy(pmap, data); //copying data to new shared file

    if (fork() == 0) {
        char *receiveData = malloc(SIZE_IN_BYTES);
        strncpy(receiveData, pmap, SIZE_IN_BYTES);
        int senderChecksum = sender(data, SIZE_IN_BYTES);
        int checksumComparison = receiver(receiveData, SIZE_IN_BYTES, senderChecksum);

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

    char *data = malloc(SIZE_IN_BYTES);
    data = OpenAndReadFile(data);

    pid = fork();
    if (pid == 0) { // Child process
        char *readMsg = malloc(SIZE_IN_BYTES);
        for (int i = 0; i < SIZE_IN_BYTES / 1024; i++) {
            char *currMsg = malloc(1024);
            read(pipefds[0], currMsg, 1024);
            strcat(readMsg, currMsg);
        }

        int senderChecksum = sender(data, SIZE_IN_BYTES);
        int checksumComparison = receiver(readMsg, SIZE_IN_BYTES, senderChecksum);

        if (checksumComparison != 0) {
            printf("end time of pipe transfer is: -1\n");
        } else {
            printf("pipe transfer after read time: %ld\n", ReturnTimeNs());
        }
        printf("Checksum difference for pipe transfer is: %d\n", checksumComparison);

    } else { //Parent process
        for (int i = 0; i < SIZE_IN_BYTES / 1024; i++) {
            write(pipefds[1], data, 1024);
            data += 1024;
        }
        wait(NULL);
    }
}

void threadSharingMainFunction() {
    // The code was adapted from https://www.geeksforgeeks.org/multithreading-c-2/
    char *data = malloc(SIZE_IN_BYTES);
    data = OpenAndReadFile(data);

    pthread_t thread_id;

    printf("The start time for thread memory sharing is: %ld\n", ReturnTimeNs());

    pthread_create(&thread_id, NULL, threadFunction, data);
    pthread_join(thread_id, NULL);
}

void* threadFunction(void* data) {
    int checksum = sender(data, SIZE_IN_BYTES);
    char* dataRead = malloc(SIZE_IN_BYTES);
    strcpy(dataRead, data);
    int checksumDifference = receiver(dataRead, SIZE_IN_BYTES, checksum);
    printf("The checksum difference for thread memory sharing is: %d\n", checksumDifference);
    if (checksumDifference != 0) {
        printf("The end time for thread memory sharing is: -1\n");
    }
    else {
        printf("The end time for thread memory sharing is: %ld\n", ReturnTimeNs());
    }

    long ReturnTimeNs() {
        struct timespec currTime;

        if (clock_gettime(CLOCK_REALTIME, &currTime) == -1) {
            perror("clock gettime");
            return EXIT_FAILURE;
        }
        return currTime.tv_nsec;
    }

    int getCheckSum(char * file)
    {
        int ChSu, sum = 0;
        for (int i = 0; i < strlen(file); i++)
            sum += file[i];
        ChSu=~sum;    //1's complement of sum
        return ChSu;
    }

    void send_file(FILE * fp, int sockfd){
        // Loop until the end of the file
        while (!feof(fp)) {
            // Read a chunk of data from the file
            char buffer[1024];
            size_t bytes_read = fread(buffer, 1, 1024, fp);

            // Send the chunk of data to the server
            send(sockfd, buffer, bytes_read, 0);
        }
        // Close the file
        fclose(fp);
    }

    int process1TCP()
    {
        FILE * fp;
        fp = fopen(FILENAME, "r");
        if (fp == NULL) {
            perror("[-]Error in reading file.");
            exit(1);
        }

        int port = RECEIVER_PORT;//convert the string of port from user to int
        int socket_desc;
        struct sockaddr_in server_addr;

        // Create socket:
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);

        if(socket_desc < 0){
            printf("Unable to create socket\n");
            return -1;
        }

        printf("Socket created successfully\n");
        // Set port and IP the same as server-side:
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(IPADDR_TCP);

        // Send connection request to server:
        if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
            printf("Unable to connect\n");
            return -1;
        }
        printf("Connected with server successfully\n");


        Starttime = ReturnTimeNs();//getTime();
        //we run in an infinite loop to always read from stdin(user)
        send_file(fp, socket_desc);
        printf("\n");

        close(socket_desc);
    }

    int process2TCP()
    {
        int port = RECEIVER_PORT;//convert the string of port from user to int
        int socket_desc, client_sock, client_size;
        struct sockaddr_in server_addr, client_addr;
        char * client_message;

        client_message = malloc(SIZE_IN_BYTES);

        // Create socket:
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);

        if(socket_desc < 0){
            printf("Error while creating socket\n");
            return -1;
        }
        printf("Socket created successfully\n");

        // Set port and IP:
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

        // Bind to the set port and IP:
        if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
            printf("Couldn't bind to the port\n");
            return -1;
        }
        printf("Done with binding\n");

        // Listen for clients:
        if(listen(socket_desc, 1) < 0){
            printf("Error while listening\n");
            return -1;
        }
        printf("\nListening for incoming connections.....\n");

        // Accept an incoming connection:
        client_size = sizeof(client_addr);
        client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);

        if (client_sock < 0){
            printf("Can't accept\n");
            return -1;
        }
        int fd = open("recivedFile.txt", O_WRONLY | O_APPEND | O_CREAT);

        //we use an infinite loop to always read from the client until we can't
        for (int i = 0; i < 102400; i++) {
            // Receive client's message:
            if (recv(client_sock, client_message, sizeof(client_message), 0) < 0) {
                printf("Couldn't receive\n");
                return -1;
            }

            write(fd, client_message, SIZE_IN_BYTES);
        }


        Endtime = ReturnTimeNs();//getTime();

        //get checksum for process1
        int ch1;
        FILE * fp;
        fp = fopen(FILENAME, "r");
        if (fp == NULL) {
            perror("[-]Error in reading file.");
            exit(1);
        }
        char * buff = malloc(SIZE_IN_BYTES);
        fread(buff, 1, SIZE_IN_BYTES, fp);
        ch1 = getCheckSum(buff);
        free(buff);

        //get checksum for process2
        char * fileRecived = malloc(SIZE_IN_BYTES);
        read(fd, fileRecived, SIZE_IN_BYTES);
        int ch2 = getCheckSum(fileRecived);
        free(fileRecived);
        close(socket_desc);
        if(ch1 == ch2)
        {
            printf("TCP/IPv4 Socket - Start: %ld\n", Starttime);
            printf("TCP/IPv4 Socket - End: %ld\n", Endtime);
        }
        else
            printf("the checksums are not identical, \n -1");
    }

    int process1UDP()
    {
        // sender

        // Open a UDP socket
        int sock = socket(AF_INET6, SOCK_DGRAM, 0);
        if (sock < 0) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Set up the receiver's address
        struct sockaddr_in6 receiver_addr;
//    memset(&receiver_addr, 0, sizeof(receiver_addr));
        receiver_addr.sin6_family = AF_INET6;
        receiver_addr.sin6_port = htons(RECEIVER_PORT);
        inet_pton(AF_INET6, IPADDR_UDP, &receiver_addr.sin6_addr);

        // Open the file to be sent
        int fd = open(FILENAME, O_RDONLY);
        if (fd < 0) {
            perror("Couldn't open file\n");
            exit(1);
        }

        // Read the file into a buffer
        char *buffer = malloc(CHUNK_SIZE);
        if (!buffer) {
            perror("Couldn't allocate memmory\n");
            exit(1);
        }

        Starttime = ReturnTimeNs();//getTime();
        ssize_t bytes_read;
        ssize_t bytesTot = 0;
        // Send the file in chunks
        while (1) {

            // Read a chunk from the file
            bytes_read = read(fd, buffer, CHUNK_SIZE);

            // If we reached the end of the file, break out of the loop
            if (bytes_read == 0) {
                strcpy(buffer, "by");
                // Send the chunk to the receiver
                sendto(sock, buffer, strlen(buffer), 0,
                       (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
                break;
            }

            // Send the chunk to the receiver
            ssize_t bytes_sent = sendto(sock, buffer, CHUNK_SIZE, 0,
                                        (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));

            if (bytes_sent < 0) {
                perror("Unable to send\n");
                exit(1);
            }
            bytesTot += bytes_sent;
        }
        //printf("bytes: %d\n", (int)bytesTot);
        printf("file sent.\n");

        // Close the socket and file
        close(sock);
        close(fd);
    }

    int process2UDP()
    {
        // receiver

        // Open a UDP socket
        int sock = socket(AF_INET6, SOCK_DGRAM, 0);
        if (sock < 0) {
            printf("Error while creating socket\n");
            return -1;
        }
        printf("Socket created successfully\n");


// Bind the socket to the local port
        struct sockaddr_in6 local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin6_family = AF_INET6;
        local_addr.sin6_port = htons(RECEIVER_PORT);
        local_addr.sin6_addr = in6addr_any;

        if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            printf("Couldn't bind to the port\n");
            return -1;
        }
        printf("Done with binding\n");



        char * fullMassage = malloc(SIZE_IN_BYTES);
        ssize_t totbytes = 0;
        // Receive the file in chunks
        while (1) {
            // Receive a chunk from the sender
            char buffer[CHUNK_SIZE];
            struct sockaddr_in6 sender_addr;
            socklen_t addrlen = sizeof(sender_addr);
            ssize_t bytes_received = recvfrom(sock, buffer, CHUNK_SIZE, 0,
                                              (struct sockaddr*)&sender_addr, &addrlen);

            if (bytes_received < 0) {
                perror("recvfrom");
                exit(1);
            }

            if (strcmp(buffer, "by") == 0) {
                break;
            }

            totbytes += bytes_received;
            // If we received 0 bytes, it means the sender has finished sending
            // the file and we can break out of the loop
            strcat(fullMassage, buffer);
        }

        //printf("bytes recived: %d\n", (int)totbytes);

        printf("file recived\n");
        //take time
        Endtime = ReturnTimeNs();//getTime();

        //get checksum for process1
        //open the file to be received
        int fd = open(FILENAME, O_RDONLY);
        if (fd < 0) {
            perror("Couldn't open file\n");
            exit(1);
        }

        char * buff = malloc(SIZE_IN_BYTES);
        read(fd, buff, SIZE_IN_BYTES);
        int ch1 = getCheckSum(buff);
        free(buff);

        //get checksum for process2
        int ch2 = getCheckSum(fullMassage);
        free(fullMassage);

// Close the socket and file
        close(sock);
        if(ch1 == ch2)
        {
            //print time
            printf("UDP/IPv6 Socket - Start: %ld\n", Starttime);
            printf("UDP/IPv6 Socket - End: %ld\n", Endtime);
        }
        printf("the checksums are not identical, \n -1\n");
        return 0;
    }
}

