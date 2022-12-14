////int main(int argc, char *argv[]) {
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
//int serverListener() {
////    //take the curren
////    long startT=ReturnTimeNs();
//    int port=atoi( portNumber);//convert the string of port from user to int
//    int socket_desc, client_sock, client_size;
//    struct sockaddr_in server_addr, client_addr;
//    char  client_message[104857600];
//
//    // Clean buffers:
//    memset(client_message, '\0', sizeof(client_message));
//
//    // Create socket:
//    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
//
//    if (socket_desc < 0) {
//        printf("Error while creating socket\n");
//        return -1;
//    }
//    printf("Socket created successfully\n");
//
//    // Set port and IP:
//    server_addr.sin_family = AF_INET;
//    server_addr.sin_port = htons(port);
//    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
//
//    // Bind to the set port and IP:
//    if (bind(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
//        printf("Couldn't bind to the port\n");
//        return -1;
//    }
//    printf("Done with binding\n");
//
//    // Listen for clients:
//    if (listen(socket_desc, 1) < 0) {
//        printf("Error while listening\n");
//        return -1;
//    }
//    printf("\nListening for incoming connections.....\n");
//
//    // Accept an incoming connection:
//    client_size = sizeof(client_addr);
//    client_sock = accept(socket_desc, (struct sockaddr *) &client_addr, &client_size);
//
//    if (client_sock < 0) {
//        printf("Can't accept\n");
//        return -1;
//    }
//    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
//        // Receive client's message:
////        if (recv(client_sock, client_message, sizeof(client_message), 0) < 0) {
////            printf("Couldn't receive\n");
////            return -1;
////        }
//
//        if (recv(client_sock, client_message, BUFFSIZE, 0) < 0) {
//            printf("Couldn't receive\n");
//            return -1;
//        }
////        //take the current time
////    long finalT=ReturnTimeNs();
//    // Closing the socket:
//    close(client_sock);
//    close(socket_desc);
//
//    return 0;
//}
//
//int main() {
//    //Open and read the file
//    char *buf = malloc(BUFFSIZE * sizeof(char));// We will allocate memory to the full paths of both files
//    buf=OpenAndReadFile(buf);
////    char* recievedData=OpenAndReadFile (buf);
//    //Calculate checksum of data
//    int CheckSumSender=sender(buf, BUFFSIZE);
//    printf("CheckSumSender is:%d",CheckSumSender);
//
//    //read the data and take Check sum in the 2nd process
//    int isListen=serverListener();
//    int CheckSumReciever=receiver(buf, BUFFSIZE,CheckSumSender);
//    printf("CheckSumReciever is:%d", CheckSumReciever);
//
//    //Take time
//    long finalT=ReturnTimeNs();
//    printf("6. Take time:%ld",finalT);
//    //Ensure that data is identical (using checksum)
//    if (CheckSumSender==CheckSumReciever)
//    {
//        printf("data is identical");
//    }
//    else{printf("data NOT identical");}
//
//
//
//}