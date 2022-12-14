#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define SIZE_IN_BYTES 104857600
long Etime;
long Stime;
int sockfd;
char buffer[SIZE_IN_BYTES];
struct sockaddr_in	 servaddr;

// The function was adapted from: https://www.educba.com/clock_gettime/
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
        sendto(sockfd, buffer, bytes_read, 0);

        sendto(sockfd, buffer, bytes_read,
               MSG_CONFIRM, (const struct sockaddr *) &servaddr,
               sizeof(servaddr));

    }
    // Close the file
    fclose(fp);
}

//client
int process1(char * portNum, char * ipAddr, FILE * fp)
{
    int port = atoi(portNum);//convert the string of port from user to int

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET6;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ipAddr);

    int n, len;
    Stime = ReturnTimeNs();//getTime();
    sendto(sockfd, (const char *)fp, strlen(fp),
           MSG_CONFIRM, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    printf("file sent.\n");
    close(sockfd);
    printf("\n");
    return 0;

}

//server
int process2(char * portNum, FILE * fp)
{
    int port = atoi(portNum);//convert the string of port from user to int
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


    Etime = ReturnTimeNs();//getTime();

    //get checksum for process1
    int ch1;
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
        return 1;
    }
    else
        return 0;
}

int main(int argc, char * argv[])
{
    FILE * fp;
    fp = fopen("file.txt", "r");
    if (fp == NULL) {
        perror("[-]Error in reading file.");
        exit(1);
    }

    int ch;

    if(argc == 3)
    {
        process1(argv[1], argv[2], fp);
    }
    if(argc == 2)
    {
        ch = process2(argv[1], fp);
    }
    //check if the 2 checksums are the same and print according to the orders
    if(ch == 1)
    {
        //print time
        printf("UDP/IPv6 Socket - Start: %ld\n", Stime);
        printf("UDP/IPv6 Socket - End: %ld\n", Etime);
    }
    else
    {
        printf("the checksums are not identical, \n -1");
    }
    return 1;

}