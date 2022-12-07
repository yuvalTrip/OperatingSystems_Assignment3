#include <stdio.h>
#include<stdio.h>
#include<math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
# include <time.h>
#define BUFFSIZE 1048576 // define file size in bytes (binary)

// The random data file was constructed using a linux command following the link:
// https://www.computernetworkingnotes.com/linux-tutorials/generate-or-create-a-large-text-file-in-linux.html

/*
 * C program to implement Checksum
 */
//Most of the code was adapted from :
//https://www.sanfoundry.com/c-program-implement-checksum-method/
int sender(char *recievedData, int numOfChar)
{
    int checksum,sum=0,i;
    printf("\n****SENDER SIDE****\n");
    for(i=0; i < numOfChar; i++)
        sum+=recievedData[i];
    printf("SUM IS: %d",sum);
    checksum=~sum;    //1's complement of sum
    printf("\nCHECKSUM IS:%d",checksum);
    return checksum;
}

int receiver( char* recievedData,int numOfChar,int SenderChecksum)
{
    int checksum,sum=0,i;
    printf("\n\n****RECEIVER SIDE****\n");
    for(i=0;i<numOfChar;i++)
        sum+=recievedData[i];
    printf("SUM IS:%d",sum);
    sum=sum+SenderChecksum;
    checksum=~sum;    //1's complement of sum
    printf("\nCHECKSUM IS:%d",checksum);
    return checksum;
}

char* OpenAndReadFile (char* buf)
{
    int scrFile=0;
    // let us open the input file
    scrFile = open("file.txt", O_RDONLY);
    if (scrFile > 0) { // there are things to read from the input
        int succe=read(scrFile, buf, BUFFSIZE);
        close(scrFile);
    }
    return buf;
}

//function measure time and return value in nanoSec
// The function was adapted from: https://www.educba.com/clock_gettime/
long ReturnTimeNs(char* argv)
{
    struct timespec currTime;

    if( clock_gettime( CLOCK_REALTIME, &currTime) == -1 )
    {
        perror( "clock gettime" );
        return EXIT_FAILURE;
    }
    return currTime.tv_nsec;
}

void main()
{
    //int n,sch,rch;
//    printf("\nENTER SIZE OF THE STRING:");
//    scanf("%d",&n);
//    int arr[n];
//    printf("ENTER THE ELEMENTS OF THE ARRAY TO CALCULATE CHECKSUM:\n");
//    for(int i=0;i<n;i++)
//    {
//        scanf("%d",&arr[i]);
//    }
    char *buf = malloc(BUFFSIZE * sizeof(char));// We will allocate memory to the full paths of both files
    buf=OpenAndReadFile(buf);
    printf("hi");
//    sch=sender(arr,n);
//    receiver(arr,n,sch);
}