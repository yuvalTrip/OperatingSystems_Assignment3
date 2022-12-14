////
////int main(int argc, char *argv[]) {
////    //sender(argv,)
////}
//
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <arpa/inet.h>
//#include "main.c"
//#define portNumber "20001"
//#define BUFFSIZE 104857600           // define file size in bytes (binary)
//
//
//int clientSender(char *IpAddr) {
//
////    //take the current time
////    long startT=ReturnTimeNs();
//    int port=atoi(portNumber);//convert the string of port from user to int
//
//    struct sockaddr_in sa;
//    char str[INET_ADDRSTRLEN];
//    //store the IP address in sa
//    inet_pton(AF_INET,IpAddr,&(sa.sin_addr));
//    //now get it back and we will use it
//    inet_ntop(AF_INET,&(sa.sin_addr),str,INET_ADDRSTRLEN);
//    //int ipAddr=atoi(IpAddr);
//    int socket_desc;
//    struct sockaddr_in server_addr;
//    char client_message[104857600];//num of bytes
//
//    // Clean buffers:
//    memset(client_message, '\0', sizeof(client_message));
//
//    // Create socket:
//    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
//
//    if (socket_desc < 0) {
//        printf("Unable to create socket\n");
//        return -1;
//    }
//
//    printf("Socket created successfully\n");
//
//    // Set port and IP the same as server-side:
//    server_addr.sin_family = AF_INET;
//    server_addr.sin_port = htons(port);
//    server_addr.sin_addr.s_addr = inet_addr(str);
//
//    // Send connection request to server:
//    if (connect(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
//        printf("Unable to connect\n");
//        return -1;
//    }
//    printf("Connected with server successfully\n");
//
//    // Get input from the user:
//    //printf("Enter message: ");
//
////    while (1) {
//
//        //read(STDIN_FILENO, client_message, 1048576);
//        char *trueMessage = malloc(104857600);
////        sprintf(trueMessage, "Output: %s", client_message);
//        trueMessage=OpenAndReadFile(trueMessage);
//        // Send the message to server:
//        if (send(socket_desc, trueMessage, 104857600, 0) < 0) {
//            printf("Unable to send message\n");
//            return -1;
//        }
////    //take the current time
////    long finalT=ReturnTimeNs();
//
//    // Close the socket:
//    close(socket_desc);
//    return 0;
//}
//
//
//int main() {
//    //Open and read the file
//    char *buf = malloc(BUFFSIZE * sizeof(char));// We will allocate memory to the full paths of both files
//    buf=OpenAndReadFile(buf);
////    char* recievedData=OpenAndReadFile (buf);
//    //Calculate checksum of data
//    int CheckSumSender=sender(buf, BUFFSIZE);
//    //Take time in both tasks
//    long finalT=ReturnTimeNs();
//    printf("4. Take time in both tasks:%ld",finalT);
//
//    //Transfer the data to the second task.
//    int isSend=clientSender("127.1.1.1");
//
//
//
//}