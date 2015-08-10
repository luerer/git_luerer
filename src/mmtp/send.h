#ifndef NET_MMT_MMTP_SEND_H_
#define NET_MMT_MMTP_SEND_H_

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
/*
    96 bit (12 bytes) pseudo header needed for udp header checksum calculation
*/
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};

unsigned short csum(unsigned short *ptr,int nbytes);
int send_raw_udp(char *UDPbuff,int length);
int send_udp2(int ss,char *UDPbuff,int length,struct sockaddr*to);

#endif
