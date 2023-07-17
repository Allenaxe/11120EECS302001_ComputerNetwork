#include "header.h"
#include <stdbool.h>
#define CORRUPT_DETECT 1

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
}

void myheaderreverse(Segment *s) 
{   
    s->l4info.SeqNum = 0, s->l4info.AckNum = 0;
    for(int i = 4, leftshift = 24; i <= 7; ++i, leftshift -= 8) {
        uint32_t tmp = ((s->header[i] < 0) ? 255 + s->header[i] + 1 : s->header[i]);
        s->l4info.SeqNum += (tmp << leftshift);
    }
    for(int i = 8, leftshift = 24; i <= 11; ++i, leftshift -= 8) {
        uint32_t tmp = ((s->header[i] < 0) ? 255 + s->header[i] + 1 : s->header[i]);
        s->l4info.AckNum += (tmp << leftshift);
    }
    s->l4info.Flag = s->header[13];
}

void initS(Segment* seg, uint16_t srcPort, uint16_t desPort) {
    seg->l4info.SourcePort = srcPort;
    seg->l4info.DesPort = desPort;
    seg->l4info.WindowSize = 65535;
}

void replyS(Segment* seg, uint32_t seqNum, uint32_t ackNum, uint8_t flags) {
    seg->l4info.SeqNum = seqNum;
    seg->l4info.AckNum = ackNum;
    seg->l4info.Flag = flags;
}

void sendpacket(int socket_fd, char* buffer, size_t length, Segment* seg, char *unknown, uint16_t flag) {
    myheadercreater(seg);
    memcpy(buffer, seg->header, sizeof(seg->header));
    send(socket_fd, buffer, length, 0);
}

ssize_t recvpacket(int socket_fd, char* buffer, size_t length, Segment* seg, char *unknown) {
    ssize_t byterecv = recv(socket_fd, buffer, length, 0);
    memcpy(seg->header, buffer, sizeof(char) * 20);
    memcpy(seg->payload, buffer + 20, sizeof(char) * 1000);
    myheaderreverse(seg); seg->p_len = ((byterecv) ? (byterecv - 20) : 0);
    return byterecv;
}

bool packet_corrupt(char* buffer, ssize_t length, const char* role) {
    printf("length: %ld\n", length);
    if(length == 0 || length == -1) return true;
    char check[1032]; memset(check, 0, sizeof(check));
    memcpy(check, buffer, sizeof(char) * 16);
    check[20] = 127; check[21] = 0; check[22] = 0; check[23] = 1;
    check[24] = 127; check[25] = 0; check[26] = 0; check[27] = 1;
    check[28] = 0; check[29] = 6; check[30] = 0; check[31] = 20;
    memcpy(check + 32, buffer + 20, sizeof(char) * (length - 20));
    uint16_t *p = (uint16_t *)buffer;
    uint16_t checksum = (*(p + 8));
    if(mychecksum(check, sizeof(check)) == ntohs(checksum)) return false;
    else return true;
}
int main(){
    /*---------------------------UDT SERVER----------------------------------*/
    srand(getpid());
        //Create socket.
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    //Set up server's address.
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr("127.0.0.1"),
        .sin_port = htons(45525)
    };
    int server_len = sizeof(serverAddr);

    //Connect to server's socket.
    if (connect(socket_fd, (struct sockaddr *)&serverAddr, server_len) == -1) {
        printf("Connect server failed!\n");
        close(socket_fd);
        exit(0);
    }
    /*---------------------------UDT SERVER-----------------------------------*/

    char o_buffer[20], i_buffer[1020];
    uint32_t currentSeg, currentAck;
    uint16_t sPort, Dport;
    Segment sendS, recvS;
    currentSeg = rand();
    currentAck = 0;
    sPort = rand()%65535 + 1;
    Dport = SERVER_PORT;
    initS(&sendS,sPort,Dport);
    replyS(&sendS,currentSeg,currentAck,SYN);
    sendpacket(socket_fd,o_buffer,sizeof(o_buffer),&sendS,"client",0);
    recvpacket(socket_fd,i_buffer,sizeof(i_buffer),&recvS,"client");
    currentAck = recvS.l4info.SeqNum+1;
    currentSeg = recvS.l4info.AckNum;
    replyS(&sendS,currentSeg,currentAck,ACK);
    sendpacket(socket_fd,o_buffer,sizeof(o_buffer),&sendS,"client",0);

    /*---------------------------3 way hand shake-----------------------------*/
    /*                                                                        */                                              
    /* TODO: Make a three way handshake with RDT server by using TCP header   */
    /*       char header[20] (lab2).                                          */
    /*       Make sure the SrcPort(Random), DesPort(45525), Seg#, Ack#, FLAG  */
    /*       are correct.                                                     */
    /*                                                                        */                                              
    /*---------------------------3 way hand shake-----------------------------*/

    printf("----------------------------------Recive data---------------------------------\n");
    Segment last_recv_packet = recvS;
    int base = sendS.l4info.AckNum;
    printf("base = %d\n", base);
    FILE* file = fopen("received_image.jpg", "wb");
    if (file == NULL) {
        perror("Fail to open");
        exit(1);
    }
    while(1){
        ssize_t byterecv = recvpacket(socket_fd,i_buffer, sizeof(i_buffer),&recvS,"client");
        if(byterecv == 0) break;
        if((CORRUPT_DETECT) ? (!packet_corrupt(i_buffer,byterecv,"client")) : 1){
            if(recvS.l4info.SeqNum == sendS.l4info.AckNum){
                last_recv_packet = recvS;
                fwrite(recvS.payload, 1, recvS.p_len, file);
                currentAck = last_recv_packet.l4info.SeqNum + last_recv_packet.p_len;
                currentSeg = last_recv_packet.l4info.AckNum;
                replyS(&sendS,currentSeg,currentAck,ACK);
                sendpacket(socket_fd,o_buffer,sizeof(o_buffer),&sendS,"client",0);

                sleep(0);
            }else{
                printf("recv seqnum == %d, send acknum == %d\n", recvS.l4info.SeqNum, sendS.l4info.AckNum);
                printf("Rdt client : cumulative ack, send last ack packet!\n");
                sendpacket(socket_fd,o_buffer,sizeof(o_buffer),&sendS, "client", 0);
                sleep(0);
            }
        }else{
            printf("Rdt client : Dropped corrupt packet\n");
            sendpacket(socket_fd,o_buffer,sizeof(o_buffer),&sendS,"client",0);
            sleep(0);
        }
    }
    
    /*----------------------------receive data--------------------------------*/
    /*                                                                        */                                              
    /* TODO: Receive data from the RDT server.                                */
    /*       Each packet will be 20bytes TCP header + 1000bytes paylaod       */
    /*       exclude the last one. (the payload may not be exactly 1000bytes) */
    /*                                                                        */
    /* TODO: Once you receive the packets, you should check whether it's      */                                                            
    /*       corrupt or not (checksum) , and send the corresponding ack       */                                                  
    /*       packet (also a char[20] ) back to the server.                    */
    /*                                                                        */
    /* TODO: fwrite the payload into a .jpg file, and check the picture.      */
    /*                                                                        */                                              
    /*----------------------------receive data--------------------------------*/



    /*-------------------------Something important----------------------------*/
    /* NOTE: TO make lab3 simple                                              */
    /*                                                                        */                                              
    /*       1. The SrcIP and DesIP are both 127.0.0.1,                       */
    /*          Tcp header length will be 20byts, windowsize = 65535 bytes    */                                              
    /*       2. The Handshake packets won't be corrupt.                       */
    /*       3. The packet will only corrupt but not miss or be disordered.   */                                              
    /*       4. Only the packets come from server may corrupt.(don't have to  */
    /*          worry that the ack sent by client will corrupt.)              */
    /*       5. We offer mychecksum() for you to verify the checksum, and     */
    /*          don't forget to verify pseudoheader part.                     */
    /*       6. Once server finish transmit the file, it will close the       */
    /*          client socket.                                                */                                              
    /*       7. You can adjust server by                                      */                                              
    /*          ./server {timeout duration} {corrupt probability}             */                                              
    /*                                                                        */                                              
    /*-------------------------Something important----------------------------*/

}