#include<stdio.h>
#include<net/if.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<time.h>
#include<sys/time.h>

#include "dhcp.h"
#define ECHOMAX 2500

void selectFunc();
void releaseAddr(char *curIPChar);
void getIPAddrClt();
void informClt();
void autoClt();
void sendReq(struct in_addr curIP, unsigned char* option50, u_int32_t leaseTime,int optcast);
void timerHandler(int sig);

struct dhcp_packet dhcpDis;
struct in_addr curIP;
u_int32_t t1, t2, t3=0;//lease time, current time+0.5t, current time+0.875t, current time+t
u_int32_t leaseTime=20;

int sock;
struct sockaddr_in echoServAddr;
struct sockaddr_in fromAddr;
unsigned int fromAddrLen;
struct sockaddr_in echoClntAddr;

struct dhcp_packet tempPkt;
struct dhcp_packet replyPkt;

void timerHandler(int sig){
        unsigned char option50[4];
        *(int *)option50=curIP.s_addr;
        sendReq(curIP, option50,leaseTime,1);
}

void selectFunc(){
        int choice;
        printf("Please choose a function: 1.Renew 2.Inform 3.Auto mode 4.Release\n");
        scanf("%d",&choice);
        switch (choice) {
                case 1:
                {
                        signal(SIGALRM,timerHandler);
                        alarm(t1-time((time_t*)NULL));
                        printf("Please enter IP address(your current IP address)you want to renew(e.g.10.3.8.211)\n");
                        char inputAdd[16];
                        struct in_addr reqAdd;
                        scanf("%s",inputAdd);
                        ulong l1 = inet_addr(inputAdd);
                        memcpy(&reqAdd,&l1,4);
                        printf("Please enter expire time in second\n");
                        u_int32_t inputTime;
                        scanf("%d",&inputTime);
                        alarm(0);

                        memset(&replyPkt,0,sizeof(replyPkt));
                        int recieved=recvfrom(sock, &replyPkt, ECHOMAX, MSG_WAITALL,(struct sockaddr *)&fromAddr, &fromAddrLen);
                        if(recieved<0){
                                //change curIP into unsigned char option50[4]
                                unsigned char option50[4];
                                *(int *)option50=curIP.s_addr;
                                sendReq(reqAdd,option50,inputTime,1);
                        }
                        break;
                }
                case 2:
                {
                        informClt();
                        break;
                }
                case 3:
                {
                        autoClt();
                        break;
                }
                case 4:
                {
                        releaseAddr(inet_ntoa(curIP));
                }
                default:
                {
                        printf("Please enter 1 or 2 or 3\n");
                        selectFunc();
                        break;
                }
        }
}

void releaseAddr(char *curIPChar){
        struct dhcp_packet usePkt;
        usePkt = getDHCPRel();
        usePkt.ciaddr.s_addr = inet_addr(curIPChar);
        sendto(sock, &usePkt, sizeof(usePkt), 0,(struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
        system("sudo ifconfig eth1 0.0.0.0");
        printf("Realease address:%s\n",inet_ntoa(usePkt.ciaddr));

        //updateLeaseFile(curIP);
}

void sendReq(struct in_addr curIP, unsigned char* option50, u_int32_t leaseTime,int optcast){

        //unicast configuration
        if(optcast == 1){

                echoServAddr.sin_addr.s_addr = inet_addr("192.168.0.1");
                system("arp -s 192.168.0.1 08:00:27:3c:74:6a");
        }

        //send DHCPREQUEST      
        memset(&tempPkt,0,sizeof(tempPkt));
        tempPkt = getReq(curIP,option50);tempPkt.flags = 0x0000;

        sendto(sock,&tempPkt,sizeof(tempPkt),MSG_WAITALL,(struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
        printf("Send DHCPREQUEST to:%s\n", inet_ntoa(echoServAddr.sin_addr));

        //send leaseTime to the server to compose Ack packet
        sendto(sock,&leaseTime,sizeof(leaseTime),0,(struct sockaddr *)&echoServAddr,sizeof(echoServAddr));
        memset(&replyPkt,0,sizeof(replyPkt));
        int recieved=recvfrom(sock, &replyPkt, ECHOMAX, MSG_WAITALL,(struct sockaddr *)&fromAddr, &fromAddrLen);
        if(recieved<0){
                printf("NO DHCPACK PACKET RECEIVED!\n");

                sleep(t2-time((time_t*)NULL));

                //configure back to broadcast
                echoServAddr.sin_addr.s_addr = inet_addr("255.255.255.255");tempPkt.flags = 0x8000;

                sendto(sock,&tempPkt,sizeof(tempPkt),MSG_WAITALL,(struct sockaddr *)&echoServAddr, sizeof(echoServAddr));//zhe li gai broadcast
                printf("Send DHCPREQUEST to: 255.255.255.255\n");


                int recieved2=recvfrom(sock, &replyPkt, ECHOMAX, MSG_WAITALL,(struct sockaddr *)&fromAddr, &fromAddrLen);
                if(recieved2<0){
                        printf("NO DHCPACK PACKET RECEIVED!!\n");
                        sleep(t3-time((time_t*)NULL));
                        memset(&tempPkt,0,sizeof(tempPkt));
                        tempPkt = getDis();
                        sendto(sock, &tempPkt, sizeof(tempPkt), 0,(struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
                        printf("Send DHCPDISCOVER to:%s\n",inet_ntoa(echoServAddr.sin_addr));

                        int recieved3=recvfrom(sock, &replyPkt, ECHOMAX,  0,(struct sockaddr *)&fromAddr, &fromAddrLen);
                        if(recieved3<0){
                                printf("NO DHCPOFFER RECIEVED!!!\n");
                                exit(0);
                        }
                }

        }
        //received properly
        if(replyPkt.options[2]==5){
                printf("Received DHCPACK. Success!\n");
                curIP=replyPkt.yiaddr;
                //write into dhcp.lease file as a record
                //updateLeaseFile(curIP);

                char *setEthIP;
                char setEth[50];
                strcpy(setEth,"sudo ifconfig eth1 ");
                setEthIP=inet_ntoa(curIP);
                strcat(setEth,setEthIP);
                strcat(setEth," netmask 255.255.255.0");
                system(setEth);
                printf("The IP address of Eth1 is set to %s\n",setEthIP);
                t1 = time((time_t*)NULL)+0.5*leaseTime;
                t2 = time((time_t*)NULL)+0.875*leaseTime;
                t3 = time((time_t*)NULL)+leaseTime;
                selectFunc();
      }
        else if(replyPkt.options[2]==6){
                printf("Received DHCPNAK. Failed! The requested IP address you entered is wrong.\n");
                getIPAddrClt();
        }
}



void getIPAddrClt(){
        //set leaseTime
        printf("Please enter expire time in second\n");
        u_int32_t inputTime;
        scanf("%d",&inputTime);
        leaseTime=inputTime;

        memset(&tempPkt,0,sizeof(tempPkt));
        tempPkt = getDis();
        sendto(sock, &tempPkt, sizeof(tempPkt), 0,(struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
        printf("Send DHCPDISCOVER to:%s\n",inet_ntoa(echoServAddr.sin_addr));

        memset(&replyPkt,0,sizeof(replyPkt));
        recvfrom(sock, &replyPkt, ECHOMAX,  0,(struct sockaddr *)&fromAddr, &fromAddrLen);
        printf("Received DHCPOFFER:offer IP address %s\n",inet_ntoa(replyPkt.yiaddr));
        curIP=replyPkt.yiaddr;
        struct in_addr iniAdd;//0.0.0.0
        ulong ls = inet_addr("0.0.0.0");
        memcpy(&iniAdd,&ls,4);

        //change yiaddr from DHCPOFFER to option50
        unsigned char option50[4];
        *(int *)option50=replyPkt.yiaddr.s_addr;
        sendReq(iniAdd,option50,leaseTime,0);
}

void informClt(){
        memset(&tempPkt,0,sizeof(tempPkt));
        tempPkt = getInf();
        sendto(sock, &tempPkt, sizeof(tempPkt), 0,(struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
        printf("Sending DHCPINFORM:query option%d,%d,%d,%d,%d,%d,%d\n",tempPkt.options[17],
        tempPkt.options[18],tempPkt.options[19],tempPkt.options[20],tempPkt.options[21],
        tempPkt.options[22],tempPkt.options[23]);

        memset(&replyPkt,0,sizeof(replyPkt));
        int received;
        if(received=recvfrom(sock, &replyPkt, ECHOMAX,  0,(struct sockaddr *)&fromAddr, &fromAddrLen)<0){
                printf("failed!!!!!!!\n");}
        printf("Received DHCPACK\n");
        selectFunc();
}

void autoClt(){
        sleep(t1-time((time_t*)NULL));
        unsigned char option50[4];
        *(int *)option50=curIP.s_addr;
        sendReq(curIP, option50,leaseTime,1);
}

int main(int argc,char *argv[]){

        unsigned short echoServPort = 67;
        unsigned short echoClntPort = 6666;
        char *servIP = "255.255.255.255";

        /* Create a datagram/UDP socket */
        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
                printf("socket() failed.\n");
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

         int i = 1;
         struct ifreq if_eth1;
         strcpy(if_eth1.ifr_name,"eth1");
         socklen_t len = sizeof(i);
         setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&i,len);
         if(setsockopt(sock,SOL_SOCKET,SO_BINDTODEVICE,"eth1",sizeof(if_eth1)) < 0){
                printf("bind socket to eth1 error\n");
         }

        /* Construct the server address structure */
        memset(&echoServAddr, 0, sizeof(echoServAddr));/*Zero out structure*/
        echoServAddr.sin_family = AF_INET; /* Internet addr family */
        echoServAddr.sin_addr.s_addr = inet_addr(servIP);/*Server IP address*/
        echoServAddr.sin_port = htons(echoServPort); /* Server port */


        //constrcut client binding for receiving
        memset(&echoClntAddr, 0, sizeof(echoClntAddr));
        echoClntAddr.sin_family = AF_INET;
        echoClntAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        echoClntAddr.sin_port =htons(echoClntPort);

        if ((bind(sock, (struct sockaddr *)&echoClntAddr,
        sizeof(echoClntAddr))) < 0)
                printf("bind() failed.\n");
        //bein functions!!
       //releaseAddr("192.168.0.2");

       getIPAddrClt();

        close(sock);
        exit(0);
}
