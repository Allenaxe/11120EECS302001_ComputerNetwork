#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "header.h"

void myheadercreater(Segment *s) 
{
    s->header[0] = (s->l4info.SourcePort >> 8);
    s->header[1] = (s->l4info.SourcePort & 255);
    s->header[2] = (s->l4info.DesPort >> 8);
    s->header[3] = (s->l4info.DesPort & 255);
    s->header[4] = (s->l4info.SeqNum >> 24);
    s->header[5] = ((s->l4info.SeqNum >> 16) & 255);
    s->header[6] = ((s->l4info.SeqNum >> 8) & 255);
    s->header[7] = (s->l4info.SeqNum & 255);
    s->header[8] = (s->l4info.AckNum >> 24);
    s->header[9] = ((s->l4info.AckNum >> 16) & 255);
    s->header[10] = ((s->l4info.AckNum >> 8) & 255);
    s->header[11] = (s->l4info.AckNum & 255);
    s->header[12] = s->l4info.HeaderLen << 4;
    s->header[13] = s->l4info.Flag;
    s->header[14] = (s->l4info.WindowSize >> 8);
    s->header[15] = (s->l4info.WindowSize & 255);
    s->pseudoheader[8] = 0;
    s->pseudoheader[9] = s->l3info.protocol;
    s->pseudoheader[10] = 0;
    s->pseudoheader[11] = 20;
    int index = 0;
    char *source = strtok(s->l3info.SourceIpv4, ".");
    while(source != NULL) {
        int element = atoi(source);
        s->pseudoheader[index++] = element;
        source = strtok(NULL, ".");
    }
    char *des = strtok(s->l3info.DesIpv4, ".");
    while(des != NULL) {
        int element = atoi(des);
        s->pseudoheader[index++] = element;
        des = strtok(NULL, ".");
    }
    int checksum = 0, tcpchecksum = 0, pseudochecksum = 0;
    for(int i = 0; i < 16; i = i + 2) {
        int a = ((s->header[i] < 0) ? 255 + s->header[i] + 1 : s->header[i]);
        int b = ((s->header[i + 1] < 0) ? 255 + s->header[i + 1] + 1 : s->header[i + 1]);
        tcpchecksum += (a << 8) + b;
    }

    for(int i = 0; i < 12; i = i + 2) {
        int a = ((s->pseudoheader[i] < 0) ? 255 + s->pseudoheader[i] + 1 : s->pseudoheader[i]);
        int b = ((s->pseudoheader[i + 1] < 0) ? 255 + s->pseudoheader[i + 1] + 1 : s->pseudoheader[i + 1]);
        printf("%d %d ", a, b);
        pseudochecksum += (a << 8) + b;
    }
    checksum = pseudochecksum + tcpchecksum;
    checksum = (checksum >> 16) + (checksum & 65535);
    checksum = ~checksum;

    s->header[16] = (checksum >> 8);
    s->header[17] = (checksum & 255);
}

int main(int argc , char *argv[])
{  
    //Create TCP socket.//
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_fd == -1)
        printf("Failed to create socket!\n");

    //Set up server's address.//
    struct sockaddr_in info;

    int addrlen = sizeof(info);
    bzero(&info, sizeof(info));

    info.sin_family = PF_INET;
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(45525);

    //Connect to server's socket.//
    int err = connect(socket_fd, (struct sockaddr*)&info, sizeof(info));

    if(err == -1)
        printf("Connection failed!\n");
    
    //Receive message from server and print it out.//
    char message[] = "Hi there!\n";
    char receivemessage[100];

    //sendto(socket_fd, message, sizeof(message), 0, (struct sockaddr*)&info, addrlen);
    //recvfrom(socket_fd, receivemessage, sizeof(receivemessage), 0, (struct sockaddr*)&info, &addrlen);
    recv(socket_fd, receivemessage, sizeof(receivemessage), 0);

    printf("%s", receivemessage);

    Segment s;
    receivedata(socket_fd, &s);
    myheadercreater(&s);
    sendheader(socket_fd, s.header);


    //////////////////////////////////////////////////////////
    //                   TASK1(Client)                      //  
    //////////////////////////////////////////////////////////
    // TODO: create a socket and connet to server.          //
    //       (server's IP address = "127.0.0.1")            //
    //       (server's Port number = 45525)                 //
    //                                                      //
    // TODO: Receive 1 message from server and print it out.//
    //       (The message you sent from server)             //
    //////////////////////////////////////////////////////////
    


    ///////////////////////////////////////////////////////////
    //                   TASK2,3(Client)                     //
    ///////////////////////////////////////////////////////////
    // TODO: Instantiate a Segment                           //
    // TODO: Pass your socket_fd and the instance of Segment //
    //       into the receivedata().                         //
    // TODO: Write your own function to create header.       //
    //       (All information is in the Segment instance.    //
    // TODO: Use sendheader(char* header) to send the header //
    //       to server.                                      //
    //                                                       //
    // Example:                                              //
    //                                                       //
    //       Segment s;                                      //
    //       receivedata(sock_fd,&s);                        //
    //       myheadercreater(&s);  //your own function       //
    //       sendheader(sock_fd,s.header);                   // 
    //                                                       //
    //                                                       //
    // Then you will see the result.                         //  
    ///////////////////////////////////////////////////////////
    
    close(socket_fd);
}