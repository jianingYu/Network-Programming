```
#define DHCP_CHADDR_LEN 16
#define DHCP_SNAME_LEN  64
#define DHCP_FILE_LEN   128
#define DHCP_MAX_OPTION_LEN  1200

//ACK and OFFER lacking renewal time value and rebinding time value

struct dhcp_packet {
 u_int8_t  op;          /* 0: Message opcode/type */
        u_int8_t  htype;        /* 1: Hardware addr type (net/if_types.h) */
        u_int8_t  hlen;         /* 2: Hardware addr length */
        u_int8_t  hops;         /* 3: Number of relay agent hops from client */
        u_int32_t xid;          /* 4: Transaction ID */
        u_int16_t secs;         /* 8: Seconds since client started looking */
        u_int16_t flags;        /* 10: Flag bits */
        struct in_addr ciaddr;  /* 12: Client IP address (if already in use) */
        struct in_addr yiaddr;  /* 16: Client IP address */
        struct in_addr siaddr;  /* 18: IP address of next server to talk to */
        struct in_addr giaddr;  /* 20: DHCP relay agent IP address */
        unsigned char chaddr [DHCP_CHADDR_LEN]; /* 24: Client hardware address */
        char sname [DHCP_SNAME_LEN];    /* 40: Server name */
        char file [DHCP_FILE_LEN];      /* 104: Boot filename */
//      uint32_t    magic_cookie;
        unsigned char  padding[64];
        unsigned char options[DHCP_MAX_OPTION_LEN];
//      struct dhcp_opt opt[64];                        
};
typedef struct dhcp_packet dhcp_packet;


//Discover packet
/*dhcp_packet getDis(dhcp_packet dhcpDis){
        memset(&dhcpDis, 0,sizeof(dhcpDis));
        /*Construct the dhcp release packet*/
        dhcpDis.op = 1;
        dhcpDis.htype = 1;
        dhcpDis.hlen = 6;
        dhcpDis.hops = 0;
        dhcpDis.xid = 12;//able to be random
        dhcpDis.secs = 0;
        dhcpDis.flags = 0x8000;        
        ulong l1 = inet_addr("110.0.20.30");          
	 memcpy(&dhcpDis.ciaddr,&l1,4);


        //dhcpDis.yiaddr = "0.0.0.0";
        //dhcpDis.siaddr = "0.0.0.0";
        //dhcpDis.giaddr = "0.0.0.0";

        //dhcpRelease.chaddr = "AA:EC:F9:23:44:19";
        //dhcpDis.magic_cookie = 0x63825363;

        
//dhcpDis.options[0] = 0x63;
//dhcpDis.options[1] = 0x82;
//dhcpDis.options[2] = 0x53;
//dhcpDis.options[3] = 0x63;

        dhcpDis.options[0] =0x35;
        dhcpDis.options[1] = 0x01; 
        dhcpDis.options[2] = 0x01; 

        dhcpDis.options[3] = 0xff; 

	return dhcpDis;

}*/

//Offer packet
struct dhcp_packet getOff(struct dhcp_packet dhcpOff){
        memset(&dhcpOff, 0,sizeof(dhcpOff));
        dhcpOff.op = 2;
        dhcpOff.htype = 1;
        dhcpOff.hlen = 6;
        dhcpOff.hops = 0;
        dhcpOff.xid = 12;//able to be random
        dhcpOff.secs = 0;
        dhcpOff.flags = 0x0000;        
        ulong lc2 = inet_addr("0.0.0.0");           
	memcpy(&dhcpDis.ciaddr,&lc2,4);

        ulong ly2 = inet_addr("110.0.20.30");           
	memcpy(&dhcpOff.yiaddr,&ly2,4);

        ulong ls2 = inet_addr("0.0.0.0");           
	memcpy(&dhcpOff.siaddr,&ls2,4);

        ulong lg2 = inet_addr("0.0.0.0");           
	memcpy(&dhcpOff.giaddr,&lg2,4);

        //dhcpDis.magic_cookie = 0x63825363;
	
	dhcpOff.chaddr[0]= 0xe0;
	dhcpOff.chaddr[1]= 0x06;
	dhcpOff.chaddr[2]= 0xe6;
	dhcpOff.chaddr[3]= 0x6a;
	dhcpOff.chaddr[4]= 0x99;
	dhcpOff.chaddr[5]= 0x39;

	//option53
        dhcpOff.options[0] =0x35;
        dhcpOff.options[1] = 0x01; 
        dhcpOff.options[2] = 0x02; 

	//option54
        dhcpOff.options[3] = 0x36;
        dhcpOff.options[4] = 0x04; 
        dhcpOff.options[5] = 0xc0;
        dhcpOff.options[6] = 0xa8;
        dhcpOff.options[7] = 0x00;
        dhcpOff.options[8] = 0x01;
	
	//option1
        dhcpOff.options[15] = 0x01;
        dhcpOff.options[16] = 0x04; 
        dhcpOff.options[17] = 0xff;
        dhcpOff.options[18] = 0xff;
        dhcpOff.options[19] = 0xff;
        dhcpOff.options[20] = 0x00;

	//option3
        dhcpOff.options[21] = 0x03;
        dhcpOff.options[22] = 0x04; 
        dhcpOff.options[23] = 0xc0;
        dhcpOff.options[24] = 0xa8;
        dhcpOff.options[25] = 0x00;
        dhcpOff.options[26] = 0x01;

	//option6
        dhcpOff.options[27] = 0x06;
        dhcpOff.options[28] = 0x04; 
        dhcpOff.options[29] = 0xc0;
        dhcpOff.options[30] = 0xa8;
        dhcpOff.options[31] = 0x00;
        dhcpOff.options[32] = 0x01;

	//option255
	dhcpOff.options[33] = 0xff; 

	return dhcpOff;
}

//Request packet
struct dhcp_packet getReq(struct dhcp_packet dhcpReq){
 	memset(&dhcpReq, 0,sizeof(dhcpReq));
        dhcpReq.op = 1;
        dhcpReq.htype = 1;
        dhcpReq.hlen = 6;
        dhcpReq.hops = 0;
        dhcpReq.xid = 12;//able to be random
        dhcpReq.secs = 0;
        dhcpReq.flags = 0x0000;        
        ulong lc3 = inet_addr("0.0.0.0");           
	memcpy(&dhcpReq.ciaddr,&lc3,4);

        ulong ly3 = inet_addr("0.0.0.0");           
	memcpy(&dhcpReq.yiaddr,&ly3,4);

        ulong ls3 = inet_addr("0.0.0.0");           
	memcpy(&dhcpReq.siaddr,&ls3,4);

        ulong lg3 = inet_addr("0.0.0.0");           
	memcpy(&dhcpReq.giaddr,&lg3,4);

        //dhcpDis.magic_cookie = 0x63825363;
	
	dhcpReq.chaddr[0]= 0xe0;
	dhcpReq.chaddr[1]= 0x06;
	dhcpReq.chaddr[2]= 0xe6;
	dhcpReq.chaddr[3]= 0x6a;
	dhcpReq.chaddr[4]= 0x99;
	dhcpReq.chaddr[5]= 0x39;

	//option53
        dhcpReq.options[0] =0x35;
        dhcpReq.options[1] = 0x01; 
        dhcpReq.options[2] = 0x03; 

	//option50
        dhcpReq.options[3] = 0x32;
        dhcpReq.options[4] = 0x04; 
        dhcpReq.options[5] = 0xc0;
        dhcpReq.options[6] = 0xa8;
        dhcpReq.options[7] = 0x00;
        dhcpReq.options[8] = 0x69;

	//option54
        dhcpReq.options[9] = 0x36;
        dhcpReq.options[10] = 0x04; 
        dhcpReq.options[11] = 0xc0;
        dhcpReq.options[12] = 0xa8;
        dhcpReq.options[13] = 0x00;
        dhcpReq.options[14] = 0x01;
	
	//option55
	dhcpReq.options[15] = 0x37; 
        dhcpReq.options[16] = 0x07; //13 length
        dhcpReq.options[17] = 0x01; //1
        //dhcpReq.options[18] = 0x1c; //28
        dhcpReq.options[19] = 0x02; //2
        dhcpReq.options[20] = 0x03; //3
        dhcpReq.options[21] = 0x0f; //15
        dhcpReq.options[22] = 0x06; //6
        //dhcpReq.options[23] = 0x77; //119
        dhcpReq.options[24] = 0x0c; //12
        //dhcpReq.options[25] = 0x2c; //44
        //dhcpReq.options[26] = 0x2f; //47
        dhcpReq.options[27] = 0x1a; //26
        //dhcpReq.options[28] = 0x79; //121
        //dhcpReq.options[29] = 0x2a; //42
	

	//option255
	dhcpReq.options[29] = 0xff; 

	return dhcpReq;
}

//ACK packet
struct dhcp_packet getAck(struct dhcp_packet dhcpAck){
        memset(&dhcpAck, 0,sizeof(dhcpAck));
        /*Construct the dhcp ack packet*/
        dhcpAck.op = 2;
        dhcpAck.htype = 1;
        dhcpAck.hlen = 6;
        dhcpAck.hops = 0;
        dhcpAck.xid = 12;//able to be random
        dhcpAck.secs = 0;
        dhcpAck.flags = 0x0000;        
        ulong lc4 = inet_addr("0.0.0.0");           
	memcpy(&dhcpAck.ciaddr,&lc4,4);

        ulong ly4 = inet_addr("110.0.20.30");           
	memcpy(&dhcpAck.yiaddr,&ly4,4);

        ulong ls4 = inet_addr("0.0.0.0");           
	memcpy(&dhcpAck.siaddr,&ls4,4);

        ulong lg4 = inet_addr("0.0.0.0");           
	memcpy(&dhcpAck.giaddr,&lg4,4);

        //dhcpDis.magic_cookie = 0x63825363;
	
	dhcpAck.chaddr[0]= 0xe0;
	dhcpAck.chaddr[1]= 0x06;
	dhcpAck.chaddr[2]= 0xe6;
	dhcpAck.chaddr[3]= 0x6a;
	dhcpAck.chaddr[4]= 0x99;
	dhcpAck.chaddr[5]= 0x39;

	//option53
        dhcpAck.options[0] =0x35;
        dhcpAck.options[1] = 0x01; 
        dhcpAck.options[2] = 0x05; 

	//option54
        dhcpAck.options[3] = 0x36;
        dhcpAck.options[4] = 0x04; 
        dhcpAck.options[5] = 0xc0;
        dhcpAck.options[6] = 0xa8;
        dhcpAck.options[7] = 0x00;
        dhcpAck.options[8] = 0x01;

	//option51
        dhcpAck.options[9] = 0x33;
        dhcpAck.options[10] = 0x04; 
        dhcpAck.options[11] = 0x00;
        dhcpAck.options[12] = 0x01;
        dhcpAck.options[13] = 0x51;
        dhcpAck.options[14] = 0x80;
	
	//option1
        dhcpAck.options[15] = 0x01;
        dhcpAck.options[16] = 0x04; 
        dhcpAck.options[17] = 0xff;
        dhcpAck.options[18] = 0xff;
        dhcpAck.options[19] = 0xff;
        dhcpAck.options[20] = 0x00;

	//option3
        dhcpAck.options[21] = 0x03;
        dhcpAck.options[22] = 0x04; 
        dhcpAck.options[23] = 0xc0;
        dhcpAck.options[24] = 0xa8;
        dhcpAck.options[25] = 0x00;
        dhcpAck.options[26] = 0x01;

	//option6
        dhcpAck.options[27] = 0x06;
        dhcpAck.options[28] = 0x04; 
        dhcpAck.options[29] = 0xc0;
        dhcpAck.options[30] = 0xa8;
        dhcpAck.options[31] = 0x00;
        dhcpAck.options[32] = 0x01;

	//option255
	dhcpAck.options[33] = 0xff; 


	//option 28 2 15 6 119 12 44 47 26 121 42
	return dhcpAck;
}

struct dhcp_packet getInf(struct dhcp_packet dhcpInf){
	

        memset(&dhcpInf, 0,sizeof(dhcpInf));
        dhcpInf.op = 1;
        dhcpInf.htype = 1;
        dhcpInf.hlen = 6;
        dhcpInf.hops = 0;
        dhcpInf.xid = 12;//able to be random
        dhcpInf.secs = 0;   //or seconds since DHCP process started
        dhcpInf.flags = 0x8000;  
      
        ulong lc5 = inet_addr("110.0.20.30");           
	memcpy(&dhcpInf.ciaddr,&lc5,4);

        ulong ly5 = inet_addr("0.0.0.0");           
	memcpy(&dhcpInf.yiaddr,&ly5,4);

        ulong ls5 = inet_addr("0.0.0.0");           
	memcpy(&dhcpInf.siaddr,&ls5,4);

        ulong lg5 = inet_addr("0.0.0.0");           
	memcpy(&dhcpInf.giaddr,&lg5,4);

        //dhcpDis.magic_cookie = 0x63825363;

	//option53
        dhcpInf.options[0] =0x35;
        dhcpInf.options[1] = 0x01; 
        dhcpInf.options[2] = 0x08; 

	//option55
	dhcpInf.options[15] = 0x37; 
        dhcpInf.options[16] = 0x07; //13 length
        dhcpInf.options[17] = 0x01; //1
        dhcpInf.options[18] = 0x02; //2
        dhcpInf.options[19] = 0x03; //3
        dhcpInf.options[20] = 0x0f; //15
        dhcpInf.options[21] = 0x06; //6
        dhcpInf.options[22] = 0x0c; //12
        dhcpInf.options[23] = 0x1a; //26

	//option255
	dhcpInf.options[33] = 0xff; 
 
        
        return dhcpInf;
}
```
