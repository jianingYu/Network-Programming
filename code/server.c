#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<net/if.h>
#include<time.h>

#include "dhcp.h"
#define ECHOMAX 2500
#define IPPOOLSIZE 10

void setRenew(struct dhcp_packet echoBuffer,unsigned char option51[4]);
void getIPAddrSvr();
void informSvr();
void updateLeaseFile(struct in_addr curIPaddr,char *proFlag);

struct in_addr curIP,setEthIP;
long t, t1, t2, t3;//lease time, current time+0.5t, current time+0.875t, current time+t
u_int32_t leaseTime;
unsigned char option51[4];

int sock;
struct sockaddr_in echoServAddr;
struct sockaddr_in echoClntAddr;
struct sockaddr_in echoFromAddr;
unsigned int cliAddrLen;
unsigned int fromAddrLen;
int recvMsgSize;
unsigned short echoServPort = 67;
unsigned short echoClntPort = 6666;

struct dhcp_packet replyPkt;
struct dhcp_packet tempPkt;
struct dhcp_packet echoBuffer;


int main(int argc,char *argv[]){

        if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
                printf("socket() failed.\n");

        //bind eth1
         int i = 1;
         struct ifreq if_eth1;
         strcpy(if_eth1.ifr_name,"eth1");
         socklen_t len = sizeof(i);
         setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&i,len);
         if(setsockopt(sock,SOL_SOCKET,SO_BINDTODEVICE,"eth1",sizeof(if_eth1)) < 0){
                printf("bind socket to eth1 error\n");
         }


        /* Construct local address structure */
        memset(&echoServAddr, 0, sizeof(echoServAddr));
        echoServAddr.sin_family = AF_INET;
        echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        echoServAddr.sin_port =htons(echoServPort);

        if ((bind(sock, (struct sockaddr *)&echoServAddr,
        sizeof(echoServAddr))) < 0)
                printf("bind() failed.\n");


        //for sending back, bind client port 68
        echoClntAddr.sin_port = htons(echoClntPort);
        echoClntAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
        echoClntAddr.sin_family = AF_INET;

        for (;;)/* Run forever */
        {
                /* Set the size of the in-out parameter */
                cliAddrLen = sizeof(echoClntAddr);
                fromAddrLen = sizeof(echoFromAddr);

                memset(&echoBuffer,0,sizeof(echoBuffer));
                memset(&leaseTime,0,sizeof(leaseTime));
                memset(&replyPkt,0,sizeof(replyPkt));
                memset(&tempPkt,0,sizeof(tempPkt));
                /* Block until receive message from a client */
                if ( (recvMsgSize = recvfrom(sock, &echoBuffer, ECHOMAX,  0,(struct sockaddr *)&echoFromAddr, &fromAddrLen)) < 0)
                        printf("recvfrom() failed.\n");
                if(echoBuffer.options[2]==3){       //If the packet sent to server is Request, then should receive another socket of lease time
                        if ( (recvMsgSize = recvfrom(sock, &leaseTime, ECHOMAX,  0,(struct sockaddr *)&echoFromAddr, &fromAddrLen)) < 0)
                                printf("recvfrom() failed.\n");
                         //change received lease time into unsigned char
                         *(long *)option51=htonl(leaseTime);
                }

                //cant receive client addr??define one
//                echoClntAddr.sin_port = htons(echoClntPort);
//                echoClntAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
//                echoClntAddr.sin_family = AF_INET;

                printf("Handling client %s",inet_ntoa(echoClntAddr.sin_addr));

                //echoBuffer[recvMsgSize] = '\0';
                printf("Received:%x\n",echoBuffer.options[2]);

                //change received lease time into unsigned char
                //option51=(unsigned char*)&leaseTime;

                switch(echoBuffer.options[2]){
                        case 7://release
                        {       printf("Have released IP address:%s\n",inet_ntoa(echoBuffer.ciaddr));
                                updateLeaseFile(setEthIP,"1");
                                break;
                        }
                        case 3://request
                        {
                                setRenew(echoBuffer,option51);
                                break;

                        }
                        case 1://discover
                        {
                                getIPAddrSvr();
                                break;
                        }
                        case 8://inform
                        {
                                informSvr();
                                break;
                        }

                        default:
                                printf("No reply.\n");
                        }


        }
}

void getIPAddrSvr(){
        //send DHCPOFFER
        memset(&tempPkt,0,sizeof(tempPkt));
        FILE *fp = fopen("dhcp.config","r");
        char rowIP[IPPOOLSIZE][16],getIP[16];
        srand((int)time(0));
        int totalRow=0;
        while(fgets(rowIP[totalRow],16,fp)){    //read each line
                totalRow++;
        }
        int randomRow=1+(int)(totalRow*rand()/(RAND_MAX+1.0));
        strcpy(getIP,rowIP[randomRow]);
        fclose(fp);

        int row;
        ulong lAdd = inet_addr(getIP);//read from ipPool
        struct in_addr setIP;
        memcpy(&setIP,&lAdd,4);
        memcpy(&setEthIP,&setIP,4);
        tempPkt = getOff(setIP);

        sendto(sock, &tempPkt, sizeof(tempPkt), 0,(struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr));
        printf("Send DHCPOFFER, the offer is:%s\n",inet_ntoa(tempPkt.yiaddr));
        //receive DHCPREQUEST   wenti
        memset(&echoBuffer,0,sizeof(echoBuffer));
        recvfrom(sock, &echoBuffer, ECHOMAX,  0,(struct sockaddr *)&echoFromAddr, &fromAddrLen);
        printf("Received Request\n");
        recvfrom(sock, &leaseTime, ECHOMAX,  0,(struct sockaddr *)&echoFromAddr, &fromAddrLen);
        *(long *)option51=htonl(leaseTime);
        setRenew(echoBuffer,option51);
}

void informSvr(){

        printf("Received DHCPDISCOVER:from %s\n",inet_ntoa(replyPkt.ciaddr));

        memset(&tempPkt,0,sizeof(tempPkt));
        tempPkt = getAck(replyPkt.ciaddr,replyPkt.ciaddr,2,0x00000000);
        tempPkt.yiaddr = replyPkt.ciaddr;
        sendto(sock, &tempPkt, sizeof(tempPkt), 0,(struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr));
        printf("Send DHCPACK to:%s\n",inet_ntoa(tempPkt.yiaddr));
}

void setRenew(struct dhcp_packet echoBuffer,unsigned char option51[4] ){

        unsigned int *temp;//requested IP address
        unsigned char reqIP[4];
        reqIP[0]=echoBuffer.options[5];reqIP[1]=echoBuffer.options[6];reqIP[2]=echoBuffer.options[7];reqIP[3]=echoBuffer.options[8];
        temp = (unsigned int *)reqIP;

        struct in_addr iniAdd;//0.0.0.0
        ulong ls = inet_addr("0.0.0.0");
        memcpy(&iniAdd,&ls,4);

        curIP=echoBuffer.ciaddr;//current IP address
        unsigned int current;

        if (curIP.s_addr!=(*temp)&&curIP.s_addr!=iniAdd.s_addr){
                replyPkt = getNak();
                printf("Option1:%d\n",replyPkt.options[2]);
                sendto(sock, &replyPkt, sizeof(replyPkt), 0,(struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr));
                printf("Send to %s\nOption:%d\n",inet_ntoa(echoClntAddr.sin_addr),replyPkt.options[2]);
        }
        else if(curIP.s_addr==(*temp)){//renew 
                replyPkt = getAck(curIP,curIP,1,option51);
                printf("Option1:%d\n",replyPkt.options[2]);
                sendto(sock, &replyPkt, sizeof(replyPkt), 0,(struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr));
                printf("Send to %s\nOption:%d\n",inet_ntoa(echoClntAddr.sin_addr),replyPkt.options[2]);
        }
        else if(curIP.s_addr==iniAdd.s_addr){//in the first process
                replyPkt = getAck(setEthIP,setEthIP,1,option51);//curIP is chosen from IP pool
                printf("Option1:%d\n",replyPkt.options[2]);
                sendto(sock, &replyPkt, sizeof(replyPkt), 0,(struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr));
                printf("Send to %s\nOption:%d\n",inet_ntoa(echoClntAddr.sin_addr),replyPkt.options[2]);

                updateLeaseFile(setEthIP,"0");
        }

}


void updateLeaseFile(struct in_addr curIPaddr,char *proFlag){

        char *curIP = inet_ntoa(curIPaddr);
        FILE *fp=fopen("dhcp.lease", "r");

        printf("dayin %s\n",curIP);
        int i=0,j=0,q=0;
        int times = 0;
        int flag = 0,new = 1;//whether have the same assigned address
        char buff[200];
        char *temp[10] ;//length of records
        for(q=0;q<5;q++){
                temp[q] = (char *)malloc(sizeof(char)*50);
        }

//char **temp = (char**)malloc(sizeof(char*) * 10);

        char delim[] = ";";

        char *token = NULL;

        while(fgets(buff,sizeof(buff),fp)!= NULL){
                times = 0;

//              printf("%s\n",buff);
                token = strtok(buff,delim);

//                      printf("%s\n",token);

//              temp[i] = token;//always the last one 
                strcpy(temp[i],token);
                strcat(temp[i],";");

                times++;
                if(strcmp(token,curIP) == 0){
//                      printf("equal\n");
                        flag = 1;
                        new = 0;
                }


                while((token = strtok(NULL,delim)) != NULL)
                {
//                      printf("%s\n",token);
                        times++;
//                      printf("%d\n",times);
                        if(flag == 1 && times == 4){
//                              printf("change %s\n",token);

                                if(strncmp(proFlag,"0",1) == 0)
                                        strcpy(token,"1");
                                else if(strncmp(proFlag,"1",1) == 0){
                                        strcpy(token,"0");
                                        //printf("gaile %s\n",token);
                                }

                        }
                        strcat(temp[i],token);
                        if(times != 4)
                                strcat(temp[i],";");
                }

                i++;
                flag = 0;
        }


        fclose(fp);


        if(new == 1){
                strcpy(temp[i],curIP);
                strcat(temp[i],";E0:06:E6:6A:99:39;");

                time_t tNow =time(NULL);

                struct tm ptm = { 0 };
                localtime_r(&tNow, &ptm);

                char szTmp[50] = {0};
                strftime(szTmp,50,"%H:%M:%S",&ptm);

                strcat(temp[i],szTmp);

                strcat(temp[i],";1");

                i++;
        }

        fp = fopen("dhcp.lease","w");
        //i = length 
        for(j=0;j<i;j++){
                printf("%s\n",temp[j]);
                fputs(temp[j],fp);
                //fputs("\n",fp);
        }


        fclose(fp);
}


