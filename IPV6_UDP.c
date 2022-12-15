#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define CHUNK_SIZE 1024
#define SIZE_IN_BYTES 104857600
#define FILENAME "testfile.txt"
#define RECEIVER_PORT 2001
#define IPADDR "::1"

long Etime;
long Stime;


// The function was adapted from: https://www.educba.com/clock_gettime/
long ReturnTimeNs() {
    struct timespec currTime;

    if (clock_gettime(CLOCK_REALTIME, &currTime) == -1) {
        perror("clock gettime");
        return EXIT_FAILURE;
    }
    return currTime.tv_nsec;
}

int getCheckSumUDP(char * file)
{
    int ChSu = 0, sum = 0;
    for (int i = 0; i < strlen(file); i++)
        sum += file[i];
    ChSu=~sum;    //1's complement of sum
    return ChSu;
}

int process1(char * ipAddr)
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
    inet_pton(AF_INET6, ipAddr, &receiver_addr.sin6_addr);

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

    Stime = ReturnTimeNs();//getTime();
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

int process2()
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
    Etime = ReturnTimeNs();//getTime();

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
        return 1;
    }
    return 0;
}

int main(int argc)
{
    int ch;//, ch2;

    if(argc == 2)
    {
        process1(IPADDR);
    }
    if(argc == 1)
    {
        ch = process2();
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
        printf("the checksums are not identical, \n -1\n");
    }
    return 1;

}