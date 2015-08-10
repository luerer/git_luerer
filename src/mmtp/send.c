/*
    Raw UDP sockets
    Silver Moon (m00n.silv3r@gmail.com)
*/
#include<stdio.h> //for printf
#include<string.h> //memset
#include<sys/socket.h>    //for socket ofcourse
#include<stdlib.h> //for exit(0);
#include<errno.h> //For errno - the error number
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/ip.h>    //Provides declarations for ip header
#include<netinet/in.h>
#include "send.h"
#include <arpa/inet.h>


#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <endian.h>//just for little endian
#include <arpa/inet.h>
#include <unistd.h>
#include <mcheck.h>
#include <time.h>
#include "mmt.h"
/*
    Generic checksum calculation function
*/
unsigned short csum(unsigned short *ptr,int nbytes)
{
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return(answer);
}

int send_raw_udp(char *UDPbuff,int length)
{
    //Create a raw socket of type IPPROTO
    int s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);

    if(s == -1)
    {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create raw socket");
        exit(1);
    }

    //Datagram to represent the packet
    char datagram[4096] , source_ip[32] , *data , *pseudogram;

    //zero out the packet buffer
    memset (datagram, 0, 4096);

    //IP header
    struct iphdr *iph = (struct iphdr *) datagram;

    //UDP header
    struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct ip));

    struct sockaddr_in sin;
    struct pseudo_header psh;

    //Data part



    memcpy(&datagram[sizeof(struct iphdr) + sizeof(struct udphdr)],UDPbuff,length);
    data = &datagram[sizeof(struct iphdr) + sizeof(struct udphdr)];
    //some address resolution
    strcpy(source_ip , "172.16.7.44");

    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr ("224.1.1.100");

    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct iphdr) + sizeof (struct udphdr) + length;
    iph->id = htonl (54321); //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 1;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;      //Set to 0 before calculating checksum
    iph->saddr = inet_addr ( source_ip );    //Spoof the source ip address
    iph->daddr = sin.sin_addr.s_addr;

    //Ip checksum
    iph->check = csum ((unsigned short *) datagram, sizeof (struct iphdr));

    //UDP header
    udph->source = htons (6666);
    udph->dest = htons (6080);
    udph->len = htons(8 + length); //tcp header size
    udph->check = 0; //leave checksum 0 now, filled later by pseudo header

    //Now the UDP checksum using the pseudo header
    psh.source_address = inet_addr( source_ip );
    psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + length );

    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + length;
    pseudogram = (char *)malloc(psize);

    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , udph , sizeof(struct udphdr) + length);

    udph->check = csum( (unsigned short*) pseudogram , psize);

    //loop if you want to flood :)
//     while (1)
    {
        //Send the packet
        if (sendto (s, datagram, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
        {
            perror("sendto failed");
        }
        //Data send successfully
        else
        {
            printf ("Packet Send. Length : %d \n" , iph->tot_len);
            close(s);
        }



        smt_packet_header_t smt_header;
        unsigned char SMTh[SMTh_BUFF_LEN];
        smt_header.SPS=0X4D54;
        smt_header.SDC=0X01;
        smt_header.LEN=10+iph->tot_len;
        smt_header.TCH=1;
        smt_header.RSV1=0;
        smt_header.RSV2=0;
        init_smt_header(&smt_header,SMTh);

        char smt_datagram[4096];
        memcpy(&smt_datagram[10],datagram,iph->tot_len);
        memcpy(&smt_datagram[0],SMTh,SMTh_BUFF_LEN);
    	//初始化
    	int ss2;
    	struct sockaddr_in sendto_addr;
    	//建立套接字
    	ss2=socket(AF_INET, SOCK_DGRAM,0);
    	if(ss2<0)
    	{
    		printf("socket error\n");
    		return -1;
    	}
    	//设置服务器地址
    	bzero(&sendto_addr,sizeof(sendto_addr));
    	sendto_addr.sin_family=AF_INET;
//    	sendto_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    	sendto_addr.sin_addr.s_addr=inet_addr("224.1.0.100");
    	sendto_addr.sin_port =htons(6080);

    	int addr_len =sizeof(struct sockaddr_in);

//        send_udp2(ss,datagram,iph->tot_len,sendto_addr);
//    	while(1){
//    		int a =0;
//    		printf("udp\n");
//    		if(sendto(ss2,datagram,iph->tot_len,0,(struct sockaddr *)&sendto_addr,addr_len)<0)
//
//    			printf("send fail error\n");
//    		}

    	sendto(ss2,smt_datagram,smt_header.LEN,0,(struct sockaddr *)&sendto_addr,addr_len);
        close(ss2);

    }

    return 0;
}

//Complete
