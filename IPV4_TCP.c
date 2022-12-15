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
//#define port 2001
//#define
long Etime;
long Stime;
#define CHUNK_SIZE 1024
#define SIZE_IN_BYTES 104857600
#define FILENAME "testfile.txt"
#define RECEIVER_PORT "2001"
#define IPADDR "127.0.0.1"


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
        char buffer[CHUNK_SIZE];
        size_t bytes_read = fread(buffer, 1, CHUNK_SIZE, fp);

        // Send the chunk of data to the server
        send(sockfd, buffer, bytes_read, 0);
    }
    // Close the file
    fclose(fp);
}

//client
int process1(char * portNum, char * ipAddr, FILE * fp)
{
    printf("hiiiii\n");

    int port = atoi(portNum);//convert the string of port from user to int
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
    server_addr.sin_addr.s_addr = inet_addr(ipAddr);

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");


    Stime = ReturnTimeNs();//getTime();
    //we run in an infinite loop to always read from stdin(user)
    send_file(fp, socket_desc);
    printf("\n");

    close(socket_desc);
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

    if (client_sock < 0)
    {
        printf("Can't accept\n");
        return -1;
    }

    int fd = open(FILENAME, O_WRONLY | O_APPEND | O_CREAT);

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
    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        perror("[-]Error in reading file.");
        exit(1);
    }

    int ch;

    if(argc == 3)
    {
        process1(RECEIVER_PORT, argv[2], fp);
    }
    if(argc == 2)
    {
        ch = process2(RECEIVER_PORT, fp);
    }
//    printf("sahgdasg%s\n", EndTime);
    //check if the 2 checksums are the same and print according to the orders
    if(ch == 1)
    {
        //print time
        printf("TCP/IPv4 Socket - Start: %ld\n", Stime);
        printf("TCP/IPv4 Socket - End: %ld\n", Etime);
    }
    else//if checksum is not identical
    {
        printf("the checksums are not identical, \n -1");
    }
    return 1;

}