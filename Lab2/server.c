#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "header.h"

int main(int argc , char *argv[]){

    //Create TCP socket//
    char inputbuffer[256]= {};
    char message[] = "Hi! I'm server 110020037.\n";
    int socket_fd = (socket(AF_INET, SOCK_STREAM, 0));
    
    if(socket_fd == -1)
        printf("Failed to create socket!\n");

    //Set up server's address.//
    struct sockaddr_in serverinfo, clientinfo;

    int addrlen = sizeof(clientinfo);
    bzero(&serverinfo, sizeof(serverinfo));

    serverinfo.sin_family = PF_INET;
    serverinfo.sin_addr.s_addr = INADDR_ANY;
    serverinfo.sin_port = htons(45525);
 
    //Bind socket to the address.//  
    bind(socket_fd, (struct sockaddr*)&serverinfo, sizeof(serverinfo));

    //Listening the socket.//
    listen(socket_fd, 5);

    //Accept the connect request.//
    int clientsocket_fd = accept(socket_fd, (struct sockaddr*)&clientinfo, &addrlen);

    //Send message to client.//
    send(clientsocket_fd, message, sizeof(message), 0);
    //sendto(clientsocket_fd, message, sizeof(message), 0, (struct sockaddr*)&clientinfo, addrlen);
    //recvfrom(clientsocket_fd, inputbuffer, sizeof(inputbuffer), 0, (struct sockaddr*)&clientinfo, &addrlen);
    //printf("%s", inputbuffer);

    serverfunction(clientsocket_fd);


    ////////////////////////////////////////////////////////////
    //                   TASK 1(Server)                       //
    ////////////////////////////////////////////////////////////
    // TODO: Create a TCP socket bind to port 45525.          //
    // TODO: Listen the TCP socket.                           //
    // TODO: Accept the connect and get the Client socket     //
    //       file descriptor.                                 //
    // TODO: Send 1 message "Hi, I'm server {Your_student_ID}"//
    //       to client.                                       //
    // Then go finish the client.c TASK 1                     //
    ////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    //                   TASK 2,3(Server)                     //
    ////////////////////////////////////////////////////////////
    // TODO: Pass the client socket fd into serverfuntion()   //
    //                                                        //
    // Example:                                               //
    //           serverfunction(client_fd);                   //
    //                                                        //
    // Then go finish the client.c TASK2,3                    //
    ////////////////////////////////////////////////////////////

    close(socket_fd);
}
