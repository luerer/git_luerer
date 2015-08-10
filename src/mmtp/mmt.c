#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#define BACKLOG 2

#include "../mpu/stream.h"
#include "../mpu/mp4.h"

#include <string.h>
#include <error.h>
#include <errno.h>
#include <inttypes.h>
#include <mcheck.h>
#include "ini.h"

#include "mmt.h"
#include "getfile.h"
#include "send.h"


int get_send_timestamp()
{
	int MMT_timestamp;
	struct timeval tv;
	gettimeofday(&tv, NULL);

	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);

	MMT_timestamp=p->tm_hour*60*60*1000+p->tm_min*60*1000+p->tm_sec*1000+tv.tv_usec/1000;
	return MMT_timestamp;
}
int get_bytes(unsigned char *srcbuff,unsigned char **dstbuff, u_int32_t number)
{
	unsigned char *buff_tmp=(unsigned  char*) malloc(number*sizeof( unsigned   char));
	memset(buff_tmp,0,number);
	memcpy(&buff_tmp[0],srcbuff,number);
	*dstbuff=buff_tmp;
	return 0;

}
int init_mmtp_header(mmt_packet_header_t *mmt_header,unsigned char *MMTPh)
{
//	mmt_packet_header_t mmt_header;

//			mmt_header.version=0;
//			mmt_header.packet_counter_flag=1;
//			mmt_header.FEC_type=0;
//			mmt_header.reserved_1=0;
//			mmt_header.extension_flag=0;
//			mmt_header.RAP_flag=0;
//			mmt_header.reserved_2=0;
//			mmt_header.type=2;
//			mmt_header.packet_id=packet_id;
//			mmt_header.timestamp=MMT_timestamp;
//			mmt_header.packet_sequence_number=0;
//			mmt_header.packet_counter=packet_counter;

//16byte
			memset(MMTPh,0,MMTPh_BUFF_LEN);
			MMTPh[0]=(u_int8_t)mmt_header->version<<6
					|(u_int8_t) mmt_header->packet_counter_flag<<5
					|(u_int8_t) mmt_header->FEC_type<<3
					|(u_int8_t) mmt_header->reserved_1<<2
					|(u_int8_t) mmt_header->extension_flag<<1
					|(u_int8_t) mmt_header->RAP_flag;
			MMTPh[1]=(u_int8_t)mmt_header->reserved_2<<6
					|(u_int8_t) mmt_header->type;
			*((u_int16_t*)&MMTPh[2])=htons(mmt_header->packet_id);
			*((u_int32_t*)&MMTPh[4])=htonl(mmt_header->present_time);
			*((u_int32_t*)&MMTPh[8])=htonl(mmt_header->timestamp);
			*((u_int32_t*)&MMTPh[12])=htonl(mmt_header->packet_sequence_number);
			*((u_int32_t*)&MMTPh[16])=htonl(mmt_header->packet_counter);

return 0;
}

int read_mmtp_header(mmt_packet_header_t *mmt_header, const char *MMTPh)
{
			mmt_header->version=(MMTPh[0]>>6)&0x03;
			mmt_header->packet_counter_flag=(MMTPh[0]>>5)&0x01;
			mmt_header->FEC_type=(MMTPh[0]>>3)&0x03;
			mmt_header->reserved_1=(MMTPh[0]>>2)&0x01;
			mmt_header->extension_flag=(MMTPh[0]>>1)&0x01;
			mmt_header->RAP_flag=MMTPh[0]&0x01;
			mmt_header->reserved_2=(MMTPh[1]>>6)&0x3F;
			mmt_header->type=MMTPh[1]&0x03;
			mmt_header->packet_id=ntohs(*((u_int16_t*)&MMTPh[2]));
			mmt_header->present_time=ntohl(*((u_int32_t*)&MMTPh[4]));
			mmt_header->timestamp=ntohl(*((u_int32_t*)&MMTPh[8]));
			mmt_header->packet_sequence_number=ntohl(*((u_int32_t*)&MMTPh[12]));
			mmt_header->packet_counter=ntohl(*((u_int32_t*)&MMTPh[16]));
			return 0;
}

int init_mpu_header(mpu_packet_header_t *mpu_header,unsigned char *MPUh)
{
			//8byte
			memset(MPUh,0,MPUh_BUFF_LEN);

//			mpu_header->length=0;
//			mpu_header->FT=0;
//			mpu_header->timed_Flag=0;
//			mpu_header->f_i=0;
//			mpu_header->aggregation_flag=0;
//			mpu_header->fragment_counter=0;
//			mpu_header->MPU_sequence_number=0;

			*((u_int16_t*)&MPUh[0])=htons(mpu_header->length);
			MPUh[2]=mpu_header->FT<<4
					|(u_int8_t) mpu_header->timed_Flag<<3
					|(u_int8_t) mpu_header->f_i<<1
					|(u_int8_t) mpu_header->aggregation_flag;

			MPUh[3]=(u_int8_t )mpu_header->fragment_counter;
			*((u_int32_t*)&MPUh[4])=htonl(mpu_header->MPU_sequence_number);

			return 0;
}

int read_mpu_header(mpu_packet_header_t *mpu_header,const char *MPUh)
{
	//8byte
			mpu_header->length=ntohs(*((u_int16_t*)&MPUh[0]));
			mpu_header->FT=(MPUh[2]>>4)&0x0F;
			mpu_header->timed_Flag=(MPUh[2]>>3)&0x01;
			mpu_header->f_i=(MPUh[2]>>1)&0x03;
			mpu_header->aggregation_flag=(MPUh[2])&0x01;
			mpu_header->fragment_counter=MPUh[3];
			mpu_header->MPU_sequence_number=ntohl(*((u_int32_t*)&MPUh[4]));

			return 0;
}

int init_mfu_time_header(mfu_time_packet_header_t *mfu_time_header,unsigned char *MFUh_t)
{
	//14byte
			memset(MFUh_t,0,MPUh_BUFF_LEN);

	//		u_int32_t movie_fragment_sequence_number;
	//			u_int32_t sample_number;
	//			u_int32_t offset;
	//			u_int8_t subsample_priority;
	//			u_int8_t dependency_counter;
//			mfu_time_header->movie_fragment_sequence_number=0;
//			mfu_time_header->sample_number=0;
//			mfu_time_header->offset=0;
//			mfu_time_header->subsample_priority=0;
//			mfu_time_header->dependency_counter=0;

			*((u_int32_t*)&MFUh_t[0])=htonl(mfu_time_header->movie_fragment_sequence_number);
			*((u_int32_t*)&MFUh_t[4])=htonl(mfu_time_header->sample_number);
			*((u_int32_t*)&MFUh_t[8])=htonl(mfu_time_header->offset);
			*((u_int8_t*)&MFUh_t[12])=(u_int8_t)mfu_time_header->subsample_priority;
			*((u_int8_t*)&MFUh_t[13])=(u_int8_t)mfu_time_header->dependency_counter;

			return 0;
}

int read_mfu_time_header(mfu_time_packet_header_t *mfu_time_header,const char *MFUh_t)
{
			mfu_time_header->movie_fragment_sequence_number=ntohl(*((u_int32_t*)&MFUh_t[0]));
			mfu_time_header->sample_number=ntohl(*((u_int32_t*)&MFUh_t[4]));
			mfu_time_header->offset=ntohl(*((u_int32_t*)&MFUh_t[8]));
			mfu_time_header->subsample_priority=MFUh_t[12];
			mfu_time_header->dependency_counter=MFUh_t[13];
			return 0;
}

int init_mfu_non_time_header(mfu_non_time_packet_header_t *mfu_non_time_header,unsigned char *MFUh_nt)
{
	//4byte

			memset(MFUh_nt,0,MFUh_nt_BUFF_LEN);

			//u_int32_t item_ID;
			mfu_non_time_header->item_ID=0;
			*((u_int32_t*)&MFUh_nt[0])=htonl(mfu_non_time_header->item_ID);

			return 0;
}

int read_mfu_non_time_header(mfu_non_time_packet_header_t *mfu_non_time_header,const char *MFUh_nt)
{
	//4byte
			mfu_non_time_header->item_ID=ntohl(*((u_int32_t*)&MFUh_nt[0]));

			return 0;
}

int init_mfu_sample(mfu_sample_t *mfu_sample,unsigned char *mfu_sample_buf)
{
	*((u_int32_t*)&mfu_sample_buf[0])=htonl(mfu_sample->sequence_number);
	mfu_sample_buf[4]=mfu_sample->trackrefindex;
	*((u_int32_t*)&mfu_sample_buf[5])=htonl(mfu_sample->movie_fragment_sequence_number);
	*((u_int32_t*)&mfu_sample_buf[9])=htonl(mfu_sample->sample_number);
	mfu_sample_buf[13]=mfu_sample->priority;
	mfu_sample_buf[14]=mfu_sample->dependency_counter;
	*((u_int32_t*)&mfu_sample_buf[15])=htonl(mfu_sample->offset);
	*((u_int32_t*)&mfu_sample_buf[19])=htonl(mfu_sample->length);
	*((u_int32_t*)&mfu_sample_buf[23])=htonl(mfu_sample->muli_length);
	*((u_int32_t*)&mfu_sample_buf[27])=mfu_sample->muli_name;
	*((u_int16_t*)&mfu_sample_buf[31])=htons(mfu_sample->muli_reserved1);
	*((u_int8_t*)&mfu_sample_buf[33])=mfu_sample->muli_reserved2;
			return 0;
}

int read_mfu_sample(mfu_sample_t *mfu_sample,const char *mfu_sample_buf)
{



	mfu_sample->sequence_number=ntohl(*((u_int32_t*)&mfu_sample_buf[0]));
	mfu_sample->trackrefindex=mfu_sample_buf[4];
	mfu_sample->sample_number=ntohl(*((u_int32_t*)&mfu_sample_buf[5]));
	mfu_sample->movie_fragment_sequence_number=ntohl(*((u_int32_t*)&mfu_sample_buf[9]));
	mfu_sample->priority=mfu_sample_buf[13];
	mfu_sample->dependency_counter=mfu_sample_buf[14];
	mfu_sample->offset=ntohl(*((u_int32_t*)&mfu_sample_buf[15]));
	mfu_sample->length=ntohl(*((u_int32_t*)&mfu_sample_buf[19]));
	mfu_sample->muli_length=ntohl(*((u_int32_t*)&mfu_sample_buf[23]));
	mfu_sample->muli_name=ntohl(*((u_int32_t*)&mfu_sample_buf[27]));
	mfu_sample->muli_reserved1=ntohl(*((u_int16_t*)&mfu_sample_buf[31]));
	mfu_sample->muli_reserved2=ntohl(*((u_int8_t*)&mfu_sample_buf[33]));
			return 0;
}

int init_smt_header(smt_packet_header_t *smt_header,unsigned char *SMTh)
{
	//10 byte
	*((u_int16_t*)&SMTh[0])=htons(smt_header->SPS);
	*((u_int8_t*)&SMTh[2])=(u_int8_t)smt_header->SDC;
	*((u_int16_t*)&SMTh[3])=htons(smt_header->LEN);
	*((u_int16_t*)&SMTh[5])=htons(smt_header->TCH);
	*((u_int16_t*)&SMTh[7])=htons(smt_header->RSV1);
	*((u_int8_t*)&SMTh[9])=(u_int8_t)smt_header->RSV2;
	return 0;

}
int read_smt_header(smt_packet_header_t *smt_header,const char *SMTh)
{
	//10byte
	smt_header->SPS=ntohs(*((u_int16_t*)&SMTh[0]));
	smt_header->SPS=SMTh[2];
	smt_header->LEN=ntohs(*((u_int16_t*)&SMTh[3]));
	smt_header->LEN=ntohs(*((u_int16_t*)&SMTh[5]));
	smt_header->RSV1=ntohs(*((u_int16_t*)&SMTh[7]));
	smt_header->RSV2=SMTh[9];
	return 0;

}

int send_mpu(int ss,struct sockaddr*to,char *Fullpath,u_int32_t *packet_sequence_number,u_int32_t *packet_counter,u_int32_t MPU_sequence_number,u_int32_t *MPU_present_time,u_int32_t packet_id)
{
	mp4_box_t *root = NULL;
	mp4_box_t *moof = NULL;
	stream_t* s = NULL;

	s = create_file_stream();
	if (stream_open(s, Fullpath, MODE_READ) == 0)
	   return -1;

	root = MP4_BoxGetRoot(s);
	//指向文件开头
	stream_seek(s, 0, 0);

	mpu_metadata_t mpu_metadata;
	fragment_metadata_t fragment_metadata;
	mpu_metadata.start=0;
	mpu_metadata.length=root->p_first->p_next->p_next->p_next->i_pos;
	mpu_metadata.number=(mpu_metadata.length+MPU_BUFF_LEN-1)/(unsigned int)MPU_BUFF_LEN;

	fragment_metadata.start=root->p_first->p_next->p_next->p_next->i_pos;
	fragment_metadata.length=root->p_first->p_next->p_next->p_next->i_size+8;
	fragment_metadata.number=(fragment_metadata.length+MPU_BUFF_LEN-1)/MPU_BUFF_LEN;

	uint32_t mdat_start;
	mdat_start=root->p_first->p_next->p_next->p_next->p_next->i_pos;

	//send mpu_metadata
	stream_seek(s, mpu_metadata.start, 0);
	int counter;
	for (counter=0;counter<mpu_metadata.number;counter++)
	{
		mpu_packet_header_t mpu_header;
		unsigned char MPUh[MPUh_BUFF_LEN];

		//no du_length and du_header here
		if(counter==(mpu_metadata.number-1))
		{
			mpu_header.length=mpu_metadata.length%MPU_BUFF_LEN+6;
			if(fragment_metadata.length%MPU_BUFF_LEN==0)
				{
					mpu_header.length=MPU_BUFF_LEN+6;
				}
		}
		else
		{
			mpu_header.length=MPU_BUFF_LEN+6;
		}

		mpu_header.FT=0;
		mpu_header.timed_Flag=1;
		if(mpu_metadata.number>1)
		{	mpu_header.fragment_counter=mpu_metadata.number;
			if(counter==0)
				mpu_header.f_i=1;
			else if(counter==(mpu_metadata.number-1))
			{
				mpu_header.f_i=3;
			}
			else
			{
				mpu_header.f_i=2;
			}
		}
		else
		{
			mpu_header.f_i=0;
			mpu_header.fragment_counter=1;
		}
		mpu_header.aggregation_flag=0;
		mpu_header.MPU_sequence_number=MPU_sequence_number;

		init_mpu_header(&mpu_header,MPUh);

		//16byte
		mmt_packet_header_t mmt_header;
		unsigned char MMTPh[MMTPh_BUFF_LEN];

		mmt_header.version=0;
		mmt_header.packet_counter_flag=1;
		mmt_header.FEC_type=0;
		mmt_header.reserved_1=0;
		mmt_header.extension_flag=0;
		mmt_header.RAP_flag=0;
		mmt_header.reserved_2=0;
		mmt_header.type=0;
		mmt_header.packet_id=packet_id;
		mmt_header.present_time=0;
		mmt_header.timestamp=get_send_timestamp();
		mmt_header.packet_sequence_number=*packet_sequence_number;
		mmt_header.packet_counter=(*packet_counter);
		if(mpu_header.f_i==0 ||mpu_header.f_i==1 )
		{
			mmt_header.present_time=*MPU_present_time;
		}
		init_mmtp_header(&mmt_header,MMTPh);

		char UDPbuff[UDP_BUFF_LEN]={};
		memcpy(UDPbuff,MMTPh,MMTPh_BUFF_LEN);
		memcpy(&UDPbuff[MMTPh_BUFF_LEN],MPUh,MPUh_BUFF_LEN);
		stream_read(s, &UDPbuff[MMTPh_BUFF_LEN+MPUh_BUFF_LEN], mpu_header.length-6);

		send_udp(ss,UDPbuff,MMTPh_BUFF_LEN+MPUh_BUFF_LEN+mpu_header.length-6,to);

		if((*packet_counter)==maximum_value)
			{
				*packet_counter=0;
			}
		else
			{
			(*packet_counter)++;
			}
		if((*packet_sequence_number)==maximum_value)
			{
				*packet_sequence_number=0;
			}
		else
			{
			(*packet_sequence_number)++;
			}



//		printf("send MPU_sequence_number %d mpu_metadata\n",MPU_sequence_number);

	}
	//send fragment_metadata
	stream_seek(s, fragment_metadata.start, 0);
	for (counter=0;counter<fragment_metadata.number;counter++)
	{
		mpu_packet_header_t mpu_header;
		unsigned char MPUh[MPUh_BUFF_LEN];

		if(counter==(fragment_metadata.number-1))
		{
			mpu_header.length=fragment_metadata.length%MPU_BUFF_LEN+6;
			if(fragment_metadata.length%MPU_BUFF_LEN==0)
					{
						mpu_header.length=MPU_BUFF_LEN+6;
					}
		}
		else
		{
			mpu_header.length=MPU_BUFF_LEN+6;
		}

		mpu_header.FT=1;
		mpu_header.timed_Flag=1;
		if(fragment_metadata.number>1)
		{
			mpu_header.fragment_counter=fragment_metadata.number;
			if(counter==0)
				mpu_header.f_i=1;
			else if(counter==(fragment_metadata.number-1))
			{
				mpu_header.f_i=3;
			}
			else
			{
				mpu_header.f_i=2;
			}
		}
		else
		{
			mpu_header.f_i=0;
			mpu_header.fragment_counter=1;
		}
		mpu_header.aggregation_flag=0;
		mpu_header.MPU_sequence_number=MPU_sequence_number;

		init_mpu_header(&mpu_header,MPUh);

		//16byte
		mmt_packet_header_t mmt_header;
		unsigned char MMTPh[MMTPh_BUFF_LEN];

		mmt_header.version=0;
		mmt_header.packet_counter_flag=1;
		mmt_header.FEC_type=0;
		mmt_header.reserved_1=0;
		mmt_header.extension_flag=0;
		mmt_header.RAP_flag=0;
		mmt_header.reserved_2=0;
		mmt_header.type=0;
		mmt_header.packet_id=packet_id;
		mmt_header.present_time=0;
		mmt_header.timestamp=get_send_timestamp();
		mmt_header.packet_sequence_number=*packet_sequence_number;
		mmt_header.packet_counter=(*packet_counter);

		init_mmtp_header(&mmt_header,MMTPh);

		char UDPbuff[UDP_BUFF_LEN]={};
		memcpy(UDPbuff,MMTPh,MMTPh_BUFF_LEN);
		memcpy(&UDPbuff[MMTPh_BUFF_LEN],MPUh,MPUh_BUFF_LEN);
		stream_read(s, &UDPbuff[MMTPh_BUFF_LEN+MPUh_BUFF_LEN], mpu_header.length-6);
		send_udp(ss,UDPbuff,MMTPh_BUFF_LEN+MPUh_BUFF_LEN+mpu_header.length-6,to);

		if((*packet_counter)==maximum_value)
			{
				*packet_counter=0;
			}
		else
			{
			(*packet_counter)++;
			}
		if((*packet_sequence_number)==maximum_value)
			{
				*packet_sequence_number=0;
			}
		else
			{
			(*packet_sequence_number)++;
			}

//		printf("send MPU_sequence_number %d fragment_metadata\n",MPU_sequence_number);

	}
	//send mfu

	// first get sample number
	uint32_t sequence_number;
	uint8_t trackrefindex;
	uint32_t samplenumber;
	u_int32_t movie_fragment_sequence_number;
	uint8_t priority;
	uint8_t dependency_counter;
	uint32_t offset;
	uint32_t length;


	char mulibuff[34]={};
	stream_seek(s, -34,2);
	    int k;
		k=stream_tell(s);
	stream_read(s, mulibuff, 34);
	sequence_number=ntohl(*((u_int32_t*)&mulibuff[0]));//from 1 counter
	trackrefindex=mulibuff[4];
	movie_fragment_sequence_number=ntohl(*((u_int32_t*)&mulibuff[5]));//from 0 counter
	samplenumber=ntohl(*((u_int32_t*)&mulibuff[9]));//from 0 counter
	priority=mulibuff[13];
	dependency_counter=mulibuff[14];
	//counter start from mdat (mdat position is 0)
	offset=ntohl(*((u_int32_t*)&mulibuff[15]));
	length=ntohl(*((u_int32_t*)&mulibuff[19]));

	int sample_total_number;
	sample_total_number=samplenumber+1;

	for (counter=0;counter<sample_total_number;counter++)
	{
		//get the sample
		uint32_t mfu_time_offset=0;
		stream_seek(s, -34*(sample_total_number-counter),2);
		stream_read(s, mulibuff, 34);

		sequence_number=ntohl(*((u_int32_t*)&mulibuff[0]));//from 1 counter
		trackrefindex=mulibuff[4];
		movie_fragment_sequence_number=ntohl(*((u_int32_t*)&mulibuff[5]));//from 0 counter
		samplenumber=ntohl(*((u_int32_t*)&mulibuff[9]));//from 0 counter
		priority=mulibuff[13];
		dependency_counter=mulibuff[14];
		//counter start from mdat (mdat position is 0)
		offset=ntohl(*((u_int32_t*)&mulibuff[15]));
		length=ntohl(*((u_int32_t*)&mulibuff[19]));

		stream_seek(s, offset+mdat_start, 0);

		int sample_frag_number=0;
		int sample_frag_counter=0;

		sample_frag_number=(length+MFU_t_BUFF_LEN-1)/MFU_t_BUFF_LEN;

		 for(sample_frag_counter=0;sample_frag_counter<sample_frag_number;sample_frag_counter++)
		 {
			 //8 byte
			mpu_packet_header_t mpu_header;
			unsigned char MPUh[MPUh_BUFF_LEN];

			if(sample_frag_counter==(sample_frag_number-1))
			{
				mpu_header.length=length%MFU_t_BUFF_LEN+MFUh_t_BUFF_LEN+6;
				if(length%MFU_t_BUFF_LEN==0)
				{
					mpu_header.length=MPU_BUFF_LEN+6;
				}
			}
			else
			{
				mpu_header.length=MPU_BUFF_LEN+6;
			}

			mpu_header.FT=2;
			mpu_header.timed_Flag=1;
			if(sample_frag_number>1)
			{
				mpu_header.fragment_counter=sample_frag_number;
				if(sample_frag_counter==0)
					mpu_header.f_i=1;
				else if(sample_frag_counter==(sample_frag_number-1))
				{
					mpu_header.f_i=3;
				}
				else
				{
					mpu_header.f_i=2;
				}
			}
			else
			{
				mpu_header.f_i=0;
				mpu_header.fragment_counter=1;
			}
			mpu_header.aggregation_flag=0;
			mpu_header.MPU_sequence_number=MPU_sequence_number;

			init_mpu_header(&mpu_header,MPUh);

			//14byte
			mfu_time_packet_header_t mfu_time_header;

			unsigned char MFUh_t[MFUh_t_BUFF_LEN];

			mfu_time_header.movie_fragment_sequence_number=movie_fragment_sequence_number;
			mfu_time_header.sample_number=counter;
			if(sample_frag_number==1)
			{
				mfu_time_header.offset=0;
			}
			else
			{
				mfu_time_header.offset=mfu_time_offset;
				mfu_time_offset=mfu_time_offset+mpu_header.length-6-MFUh_t_BUFF_LEN;


			}
			mfu_time_header.subsample_priority=1;
			mfu_time_header.dependency_counter=0;

			init_mfu_time_header(&mfu_time_header,MFUh_t);

	//
	//		//4byte
	//		mfu_non_time_packet_header_t mfu_non_time_header;
	//		unsigned char MFUh_nt[MPUh_BUFF_LEN];
	//
	//
	//		//u_int32_t Item_ID;
	//		mfu_non_time_header.Item_ID=0;
	//		*((u_int32_t*)&MFUh_nt[0])=htonl(mfu_non_time_header.Item_ID);
	//
	//		mfu_non_time_header.Item_ID=ntohl(*((u_int32_t*)&MFUh_nt[0]));

			//16byte
			mmt_packet_header_t mmt_header;
			unsigned char MMTPh[MMTPh_BUFF_LEN];

			mmt_header.version=0;
			mmt_header.packet_counter_flag=1;
			mmt_header.FEC_type=0;
			mmt_header.reserved_1=0;
			mmt_header.extension_flag=0;
			mmt_header.RAP_flag=0;
			mmt_header.reserved_2=0;
			mmt_header.type=0;
			mmt_header.packet_id=packet_id;
			mmt_header.present_time=0;
			mmt_header.timestamp=get_send_timestamp();
			mmt_header.packet_sequence_number=*packet_sequence_number;
			mmt_header.packet_counter=(*packet_counter);

			init_mmtp_header(&mmt_header,MMTPh);

			char UDPbuff[UDP_BUFF_LEN]={};
			memcpy(UDPbuff,MMTPh,MMTPh_BUFF_LEN);
			memcpy(&UDPbuff[MMTPh_BUFF_LEN],MPUh,MPUh_BUFF_LEN);
			memcpy(&UDPbuff[MMTPh_BUFF_LEN+MPUh_BUFF_LEN],MFUh_t,MFUh_t_BUFF_LEN);
//			int k;
//			k=stream_tell(s);
//			printf("stream_tell 1  %d ",k);
			stream_read(s, &UDPbuff[MMTPh_BUFF_LEN+MPUh_BUFF_LEN+MFUh_t_BUFF_LEN],mpu_header.length-6-MFUh_t_BUFF_LEN);
//			k=stream_tell(s);
//			printf("  %d ",k);
			send_udp(ss,UDPbuff,MMTPh_BUFF_LEN+MPUh_BUFF_LEN+mpu_header.length-6,to);
//			socklen_t len=sizeof(*to);
//			sendto(ss,UDPbuff,MMTPh_BUFF_LEN+MPUh_BUFF_LEN+mpu_header.length,0,to,len);
			if((*packet_counter)==maximum_value)
				{
					*packet_counter=0;
				}
			else
				{
				(*packet_counter)++;
				}
			if((*packet_sequence_number)==maximum_value)
				{
					*packet_sequence_number=0;
				}
			else
				{
				(*packet_sequence_number)++;
				}
			if(MPU_sequence_number==0 && counter ==0&& sample_frag_counter==86 )
			{
//				printf("breakpoint\n");
			}
//			printf("send MPU_sequence_number %d mfu counter%d  total sample %d +sample_frag_counter%d total sample_frag_number%d   fragment.length%d\n",MPU_sequence_number,counter,sample_total_number,sample_frag_counter,sample_frag_number,mpu_header.length-MFUh_t_BUFF_LEN);
		 }
//		 printf("finished send a sample total sample %d\n",sample_total_number);
	  }
	printf("finished send a mpu \n");


   //ATOM BOX name like ATOM+Box_name,for example "ATOM_moof"
   //moof=MP4_BoxSearchBox(root,ATOM_moof);
   //MP4_BoxFree(s, moof);
   //moof=NULL;
   //int i;
   //MP4_BoxSearchBox2(root,&moof,ATOM_moof);
	//		   i=MP4_BoxSearchBox3(root,ATOM_moof);
   free(moof);
   MP4_BoxFree(s, root);

   stream_close(s);
   destory_file_stream(s);

	 return 0;
}

int send_nontimed_mpu(int ss,struct sockaddr*to,char *Fullpath,u_int32_t *packet_sequence_number,u_int32_t *packet_counter,u_int32_t MPU_sequence_number,u_int32_t *MPU_present_time,u_int32_t packet_id)
{
   	mp4_box_t *root;
   	mp4_box_t *SearchResult = NULL;
   	stream_t *s = NULL;

	s = create_file_stream();
	if (stream_open(s, Fullpath, MODE_READ) == 0)
	   return -1;

	root = MP4_BoxGetRoot(s);
	if(!root)
	   return -1;
	
	SearchResult=MP4_BoxSearchBox(root,ATOM_iloc);
	uint16_t item_count;

	item_count=SearchResult->data.p_iloc->item_count;

    uint16_t *item_ID = (uint16_t*)malloc(sizeof(uint16_t)*item_count);    /* theses are arrays */
	uint32_t *base_offset = (uint32_t*)malloc(sizeof(uint32_t)*item_count);
	uint16_t *extent_count = (uint16_t*)malloc(sizeof(uint16_t)*item_count);
    uint32_t *extent_offset = (uint32_t*)malloc(sizeof(uint32_t)*item_count);
	uint32_t *extent_length = (uint32_t*)malloc(sizeof(uint32_t)*item_count);

	//指向文件开头
	stream_seek(s, 0, 0);
	mpu_metadata_t mpu_metadata;
	mpu_metadata.start=0;
	mpu_metadata.length=SearchResult->data.p_iloc->base_offset[0];  ////
	mpu_metadata.number=(mpu_metadata.length+MPU_BUFF_LEN-1)/(unsigned int)MPU_BUFF_LEN;
//	printf("mpu_metadata.length= %d \n",mpu_metadata.length);
//	printf("mpu_metadata.number= %d \n",mpu_metadata.number);

	//send mpu_metadata
	stream_seek(s, mpu_metadata.start, 0);
	int counter;
	for (counter=0;counter<mpu_metadata.number;counter++)
	{
		mpu_packet_header_t mpu_header;
		unsigned char MPUh[MPUh_BUFF_LEN];

		if(counter==(mpu_metadata.number-1))
		{
			mpu_header.length=mpu_metadata.length%MPU_BUFF_LEN+6;
			if(mpu_metadata.length%MPU_BUFF_LEN==0)
				{
					mpu_header.length=MPU_BUFF_LEN+6;
				}
		}
		else
		{
			mpu_header.length=MPU_BUFF_LEN+6;
		}

		mpu_header.FT=0;
		mpu_header.timed_Flag=0;
		if(mpu_metadata.number>1)
		{	mpu_header.fragment_counter=mpu_metadata.number;
			if(counter==0)
				mpu_header.f_i=1;
			else if(counter==(mpu_metadata.number-1))
			{
				mpu_header.f_i=3; 
			}
			else
			{
				mpu_header.f_i=2;
			}
		}
		else
		{
			mpu_header.f_i=0;
			mpu_header.fragment_counter=1; 
		}
		mpu_header.aggregation_flag=0;  ////only one data unit is present in the payload
		mpu_header.MPU_sequence_number=MPU_sequence_number;

		init_mpu_header(&mpu_header,MPUh);

		//16byte
		mmt_packet_header_t mmt_header;
		unsigned char MMTPh[MMTPh_BUFF_LEN];

		mmt_header.version=0;
		mmt_header.packet_counter_flag=1;
		mmt_header.FEC_type=0;
		mmt_header.reserved_1=0;
		mmt_header.extension_flag=0;
		mmt_header.RAP_flag=0;
		mmt_header.reserved_2=0;
		mmt_header.type=0;
		mmt_header.packet_id=packet_id;
		mmt_header.present_time=0;
		mmt_header.timestamp=get_send_timestamp();
		mmt_header.packet_sequence_number=*packet_sequence_number;
		mmt_header.packet_counter=(*packet_counter);
		if(mpu_header.f_i==0 ||mpu_header.f_i==1 )
		{
			mmt_header.present_time=*MPU_present_time;
		}
		init_mmtp_header(&mmt_header,MMTPh);

		char UDPbuff[UDP_BUFF_LEN]={};
		memcpy(UDPbuff,MMTPh,MMTPh_BUFF_LEN);
		memcpy(&UDPbuff[MMTPh_BUFF_LEN],MPUh,MPUh_BUFF_LEN);
		stream_read(s, &UDPbuff[MMTPh_BUFF_LEN+MPUh_BUFF_LEN], mpu_header.length-6);

		send_udp(ss,UDPbuff,MMTPh_BUFF_LEN+MPUh_BUFF_LEN+mpu_header.length-6,to);


		if((*packet_counter)==maximum_value)
			{
				*packet_counter=0;
			}
		else
			{
			(*packet_counter)++;
			}
		if((*packet_sequence_number)==maximum_value)
			{
				*packet_sequence_number=0;
			}
		else
			{
			(*packet_sequence_number)++;
			}
	}

	//send non_timed_mfu
	//read iloc box

//	printf("item_count is %d\n",item_count);

	for (counter= 0; counter < item_count; counter++)
	{
		item_ID[counter] = SearchResult->data.p_iloc->item_ID[counter];
		base_offset[counter] = SearchResult->data.p_iloc->base_offset[counter];
//		printf("base_offset[%d]=%d \n",counter, SearchResult->data.p_iloc->base_offset[counter]);
		extent_count[counter] = 1 ;//solid value 1
		extent_offset[counter] = SearchResult->data.p_iloc->extent_offset[counter];
		extent_length[counter] = SearchResult->data.p_iloc->extent_length[counter];

		stream_seek(s, base_offset[counter], 0); 

		int sample_frag_number=0;
		int sample_frag_counter=0;
		sample_frag_number=(extent_length[counter]+MFU_nt_BUFF_LEN-1)/MFU_nt_BUFF_LEN;
//		printf("extent_offset[%d]= %X \n", counter, extent_offset[counter]);
//		printf("extent_length[%d]= %X \n", counter, extent_length[counter]);
//		printf("sample_frag_number=%d \n", sample_frag_number);
		for(sample_frag_counter=0;sample_frag_counter<sample_frag_number;sample_frag_counter++)
		{
			 //8 byte
			mpu_packet_header_t mpu_header;
			unsigned char MPUh[MPUh_BUFF_LEN];

			////MMTP payload header for MPU mode
			if(sample_frag_counter==(sample_frag_number-1))
			{
				mpu_header.length=extent_length[counter]%MFU_nt_BUFF_LEN+MFUh_nt_BUFF_LEN+6;
				if(extent_length[counter]%MFU_nt_BUFF_LEN == 0)
				{
					mpu_header.length = MFU_nt_BUFF_LEN+4+6;
				}
			}
			else
			{
				mpu_header.length = MFU_nt_BUFF_LEN+4+6;
			}

			mpu_header.FT=2;
			mpu_header.timed_Flag=0;
			if(sample_frag_number>1)
			{
				mpu_header.fragment_counter=sample_frag_number;
				if(sample_frag_counter==0)
					mpu_header.f_i=1;
				else if(sample_frag_counter==(sample_frag_number-1))
				{
					mpu_header.f_i=3;
				}
				else
				{
					mpu_header.f_i=2;
				}
			}
			else
			{
				mpu_header.f_i=0;
				mpu_header.fragment_counter=1;
			}
			mpu_header.aggregation_flag=0;
			mpu_header.MPU_sequence_number=MPU_sequence_number;
			
			init_mpu_header(&mpu_header,MPUh);


			//4byte
			////the DU header for non-timed-media MFU
			mfu_non_time_packet_header_t mfu_non_time_header;
			unsigned char MFUh_nt[MFUh_nt_BUFF_LEN];
			//u_int32_t Item_ID;
			mfu_non_time_header.item_ID=item_ID[counter];

		////printf("mfu_non_time_header.item_ID = %d \n",mfu_non_time_header.item_ID);

			init_mfu_non_time_header(&mfu_non_time_header,MFUh_nt);

			
			////MMTP packet
			mmt_packet_header_t mmt_header;
			unsigned char MMTPh[MMTPh_BUFF_LEN];
			mmt_header.version=0;
			mmt_header.packet_counter_flag=1;
			mmt_header.FEC_type=0;
			mmt_header.reserved_1=0;
			mmt_header.extension_flag=0;
			mmt_header.RAP_flag=0;
			mmt_header.reserved_2=0;
			mmt_header.type=0;
			mmt_header.packet_id=packet_id;
			mmt_header.present_time=0;
			mmt_header.timestamp=get_send_timestamp();
			mmt_header.packet_sequence_number=*packet_sequence_number;
			mmt_header.packet_counter=(*packet_counter);

			init_mmtp_header(&mmt_header,MMTPh);

			char UDPbuff[UDP_BUFF_LEN]={};
			memcpy(UDPbuff,MMTPh,MMTPh_BUFF_LEN);
			memcpy(&UDPbuff[MMTPh_BUFF_LEN],MPUh,MPUh_BUFF_LEN);
			memcpy(&UDPbuff[MMTPh_BUFF_LEN+MPUh_BUFF_LEN],MFUh_nt,MFUh_nt_BUFF_LEN);

			stream_read(s, &UDPbuff[MMTPh_BUFF_LEN+MPUh_BUFF_LEN+MFUh_nt_BUFF_LEN],mpu_header.length-6-MFUh_nt_BUFF_LEN);

			send_udp(ss,UDPbuff,MMTPh_BUFF_LEN+MPUh_BUFF_LEN+mpu_header.length-6,to);

//			FILE *result;
//			if((result=fopen("result.txt","a+"))==NULL)
//			{
//					printf("not open");
//					exit(0);
//			}
//			fprintf(result,"%d : %d\n", mmt_header.packet_counter,mpu_header.MPU_sequence_number);
//
//			fclose(result);

			if((*packet_counter)==maximum_value)
			{
				*packet_counter=0;
			}
			else
			{
				(*packet_counter)++;
			}
			if((*packet_sequence_number)==maximum_value)
			{
				*packet_sequence_number=0;
			}
			else
			{
				(*packet_sequence_number)++;
			}
		}
		 printf("finished send a sample frag sample %d\n",sample_frag_number);
	}
	free(item_ID);
	item_ID = NULL;
	free(base_offset);
	base_offset = NULL;
	free(extent_count);
	extent_count = NULL;
	free(extent_offset);
	extent_offset = NULL;
	free(extent_length);
	extent_length = NULL;
	printf("finished send a mpu \n");

////   free(SearchResult);
	MP4_BoxFree(s, root);
	stream_close(s);
	destory_file_stream(s);
	return 0;
}

int send_mpu_endpacket(int ss,struct sockaddr*to,char *Fullpath,u_int32_t *packet_sequence_number,u_int32_t *packet_counter,u_int32_t MPU_sequence_number,u_int32_t *MPU_present_time,u_int32_t packet_id)
{

	mpu_packet_header_t mpu_header;
	unsigned char MPUh[MPUh_BUFF_LEN];


	mpu_header.length=6;
	mpu_header.FT=3;
	mpu_header.timed_Flag=0;

	mpu_header.f_i=0;
	mpu_header.fragment_counter=0;

	mpu_header.aggregation_flag=0;
	mpu_header.MPU_sequence_number=MPU_sequence_number;

	init_mpu_header(&mpu_header,MPUh);

	//16byte
	mmt_packet_header_t mmt_header;
	unsigned char MMTPh[MMTPh_BUFF_LEN];

	mmt_header.version=0;
	mmt_header.packet_counter_flag=1;
	mmt_header.FEC_type=0;
	mmt_header.reserved_1=0;
	mmt_header.extension_flag=0;
	mmt_header.RAP_flag=0;
	mmt_header.reserved_2=0;
	mmt_header.type=0;
	mmt_header.packet_id=packet_id;
	mmt_header.present_time=0;
	mmt_header.timestamp=0;
	mmt_header.packet_sequence_number=*packet_sequence_number;
	mmt_header.packet_counter=(*packet_counter);

	init_mmtp_header(&mmt_header,MMTPh);

	char UDPbuff[UDP_BUFF_LEN]={};
	memcpy(UDPbuff,MMTPh,MMTPh_BUFF_LEN);
	memcpy(&UDPbuff[MMTPh_BUFF_LEN],MPUh,MPUh_BUFF_LEN);

	send_udp(ss,UDPbuff,MMTPh_BUFF_LEN+MPUh_BUFF_LEN+mpu_header.length-6,to);

	if((*packet_counter)==maximum_value)
		{
			*packet_counter=0;
		}
	else
		{
		(*packet_counter)++;
		}
	if((*packet_sequence_number)==maximum_value)
		{
			*packet_sequence_number=0;
		}
	else
		{
		(*packet_sequence_number)++;
		}
 return 0;
}

int init_pa_message(pa_message_t *pa_header,unsigned char *PAh)
{
//	memset(PAh,0,PAh_BUFF_LEN );
//7 byte
	*((u_int16_t*)&PAh[0])=htons((u_int16_t)pa_header->message_id);
	*((u_int8_t*)&PAh[2])=pa_header->version;
	*((u_int32_t*)&PAh[3])=htonl(pa_header->length);
	*((u_int8_t*)&PAh[7])=pa_header->number_of_tables;
//	*((u_int8_t*)&MFUh_t[12])=(u_int8_t)mfu_time_header->subsample_priority;
    return 0;

}
int read_pa_message_header(pa_message_t *pa_message,const char *pa_message_buf)
{

	pa_message->message_id=*((u_int16_t*)&pa_message_buf[0]);
	pa_message->version=pa_message_buf[2];
	pa_message->length=ntohl(*((u_int32_t*)&pa_message_buf[3]));
	pa_message->number_of_tables=pa_message_buf[7];

    return 0;
}

int read_pa_message(pa_message_t *pa_message,const char *pa_message_buf)
{

	pa_message->message_id=*((u_int16_t*)&pa_message_buf[0]);
	pa_message->version=pa_message_buf[2];
	pa_message->length=ntohl(*((u_int32_t*)&pa_message_buf[3]));
	pa_message->number_of_tables=pa_message_buf[7];
	pa_message->table_header= (table_header_t *)malloc(sizeof(table_header_t )*pa_message->number_of_tables);
	int i;
	for(i=0;i<pa_message->number_of_tables;i++)
	{
		read_table_header(&pa_message->table_header[i] ,&pa_message_buf[PAh_BUFF_LEN+sizeof(table_header_t )*i]);
	}
	//point to first table
	int buff_seek=PAh_BUFF_LEN+sizeof(table_header_t )*pa_message->number_of_tables;
	for(i=0;i<pa_message->number_of_tables;i++)
	{
		//PA table
		if(pa_message->table_header[i].table_id==0x00)
		{
			read_pa_table(&pa_message->pa_table ,&pa_message_buf[buff_seek]);
		}
		//MP table
		if(pa_message->table_header[i].table_id==0x20)
		{
			read_mp_table(&pa_message->mp_table ,&pa_message_buf[buff_seek]);
		}
		//MPI table
		if(pa_message->table_header[i].table_id==0x10)
		{
			read_mpi_table(&pa_message->mpi_table,&pa_message_buf[buff_seek]);
		}
		//point to next table
		buff_seek=buff_seek+table_header_LEN+pa_message->table_header[i].length;

	}
    return 0;
}

int free_pa_message(pa_message_t *pa_message)
{
	free(pa_message->table_header);
	pa_message->table_header = NULL;
	int i;
	for(i=0;i<pa_message->number_of_tables;i++)
	{
		//PA table
		if(pa_message->table_header[i].table_id==0x10)
		{
			free_pa_table(&pa_message->pa_table);
		}
		//MP table
		if(pa_message->table_header[i].table_id==0x40)
		{
			free_mp_table(&pa_message->mp_table );
		}
		//MPI table
		if(pa_message->table_header[i].table_id==0x20)
		{
			free_mpi_table(&pa_message->mpi_table);
		}
	}
	return 0;
}
int init_table_header(table_header_t *table_header ,unsigned char *table_buf)
{
	memset(table_buf,0,1024);
//4 byte
	*((u_int8_t*)&table_buf[0])=table_header->table_id;
	*((u_int8_t*)&table_buf[1])=table_header->version;
	*((u_int16_t*)&table_buf[2])=htons(table_header->length);
    return 0;

}

int read_table_header(table_header_t *table_header ,const char *table_buf)
{

	table_header->table_id=*((u_int8_t*)&table_buf[0]);
	table_header->version=*((u_int8_t*)&table_buf[1]);
	table_header->length=ntohs(*((u_int16_t*)&table_buf[2]));
	return 0;

}


int init_pa_table(pa_table_t *pa_table ,unsigned char *PA_table_buf)
{
	memset(PA_table_buf,0,1024);
//4 byte
	*((u_int8_t*)&PA_table_buf[0])=pa_table->table_id;
	*((u_int8_t*)&PA_table_buf[1])=pa_table->version;
	*((u_int16_t*)&PA_table_buf[2])=htons(pa_table->length);

    return 0;

}

int read_pa_table(pa_table_t *pa_table ,const char *PA_table_buf)
{

	pa_table->table_id=*((u_int8_t*)&PA_table_buf[0]);
	pa_table->version=*((u_int8_t*)&PA_table_buf[1]);
	pa_table->length=ntohs(*((u_int16_t*)&PA_table_buf[2]));
	pa_table->pat_content=(unsigned  char*) malloc(pa_table->length*sizeof( unsigned  char));
	memcpy(pa_table->pat_content,&PA_table_buf[4],pa_table->length);
	//pa_table->pat_content=*((char *)&PA_table_buf[4]);
    return 0;

}
int free_pa_table(pa_table_t *pa_table )
{

	free(pa_table->pat_content);
	pa_table->pat_content = NULL;
    return 0;

}

int init_mp_table(mp_table_t *mp_table, unsigned char **mp_table_buf)
{
//	memset(mp_table_buf,0,1024);
	unsigned char *mp_table_buf_tmp = (unsigned   char*) malloc((4+mp_table->length)*sizeof( unsigned   char));
	memset(mp_table_buf_tmp,0,4+mp_table->length);
	if(mp_table_buf_tmp==NULL)
		{
			puts ("Memory allocation failed.");
			 exit (EXIT_FAILURE);
		}

	*((u_int8_t*)&mp_table_buf_tmp[0])=mp_table->table_id;
	*((u_int8_t*)&mp_table_buf_tmp[1])=mp_table->version;
	*((u_int16_t*)&mp_table_buf_tmp[2])=htons(mp_table->length);
	*((u_int8_t*)&mp_table_buf_tmp[4]) = (mp_table->reserved<<2)|(mp_table->MP_table_mode);

	*((u_int8_t*)&mp_table_buf_tmp[5])=mp_table->number_of_assets;

	u_int32_t  i,seekpoint=6;
	u_int32_t j, location_num;
	for(i=0;i<mp_table->number_of_assets;i++)
	{
		*((u_int8_t*)&mp_table_buf_tmp[seekpoint]) = mp_table->MP_table_asset[i].Identifier_mapping->identifier_type;
		seekpoint += 1;
		//0x00 for asset_id
		if(mp_table->MP_table_asset[i].Identifier_mapping->identifier_type == 0x00)
		{
			u_int8_t length_tmp = mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_length;
			*((u_int32_t*)&mp_table_buf_tmp[seekpoint]) =  htons(mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_scheme);
			*((u_int8_t*)&mp_table_buf_tmp[seekpoint+4]) = length_tmp;
			seekpoint += 5;
			memcpy(&mp_table_buf_tmp[seekpoint],mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_byte,length_tmp);
			seekpoint += length_tmp;
		}
		//0x01 for URL
		//not handled now
		else if(mp_table->MP_table_asset[i].Identifier_mapping->identifier_type == 0x01)
		{
			seekpoint += 0;
		}
		*((u_int32_t*)&mp_table_buf_tmp[seekpoint]) = htons(mp_table->MP_table_asset[i].asset_type);
		*((u_int8_t*)&mp_table_buf_tmp[seekpoint+4]) = ((mp_table->MP_table_asset[i].reserved)<<1)|(mp_table->MP_table_asset[i].asset_clock_relation_flag);
		*((u_int8_t*)&mp_table_buf_tmp[seekpoint+5]) = mp_table->MP_table_asset[i].asset_loaction->location_count;
		seekpoint += 6;

		location_num = mp_table->MP_table_asset[i].asset_loaction->location_count;
		for(j=0;j<location_num;j++)
		{
			*((u_int8_t*)&mp_table_buf_tmp[seekpoint+1]) = mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].location_type;
			seekpoint += 1;
			//0x00 for packet_id
			if(mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].location_type == 0x00)
			{
				*((u_int16_t*)&mp_table_buf_tmp[seekpoint]) = htons(mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].general_location_info_byte->packet_id);
				seekpoint += 2;
			}
			else if(mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].location_type == 0x01)
			{
				seekpoint += 0;
			}
		}

		*((u_int16_t*)&mp_table_buf_tmp[seekpoint]) = mp_table->MP_table_asset[i].asset_descriptors_length;
		memcpy(&mp_table_buf_tmp[seekpoint+2],mp_table->MP_table_asset[i].asset_descriptors_byte,mp_table->MP_table_asset[i].asset_descriptors_length);
		seekpoint += (2+mp_table->MP_table_asset[i].asset_descriptors_length);
	}

	*mp_table_buf = mp_table_buf_tmp;
	return 0;
}


int get_mp_table_length(mp_table_t *mp_table)
{
	int i = 0;
	//table_length here equals table.length +1+1+2
	int table_length = 0,mp_table_asset_len = 0;
	//+table_id+version+length+reserved+MP_table_mode
	table_length = 1+1+2+1+1;

	int asset_num = mp_table->number_of_assets;

	for(i=0;i<asset_num;i++)
	{
		int j = 0;
		int location_num = mp_table->MP_table_asset[i].asset_loaction->location_count;
		int identifier_mapping_length =0 ;
		//0x00 for asset_id
		if(mp_table->MP_table_asset[i].Identifier_mapping->identifier_type == 0x00)
		{
			identifier_mapping_length = 1+4+1+(mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_length);
		}
		// 0x01 for URL
		// not handled now
		else if(mp_table->MP_table_asset[i].Identifier_mapping->identifier_type == 0x01)
		{
			identifier_mapping_length = 1;
		}
		table_length += identifier_mapping_length;
		//+ asset_type+reserved+asset_clock_relation_flag
		table_length += 4+1;

		//asset_location
		//+location_count+location_type
		table_length += 1+1*location_num;
		for(j=0;j<location_num;j++)
		{
			//0x00 for packet_id
			if(mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].location_type == 0x00)
			{
				table_length += 2;
			}
			else
			{
			//other location_type
			//not handled now
				table_length += 0;
			}
		}

		//+asset_des
		table_length += 2+mp_table->MP_table_asset[i].asset_descriptors_length;

	}
	return table_length;
}


int make_mp_table(mp_table_t* mp_table, asset_info_t* asset_info)
{
//	mp_table_t mp_table;
	mp_table->table_id=0x20;		//complete MP table
	mp_table->version=0x00;
	mp_table->length=0;
	mp_table->reserved = 63; //all 1
	mp_table->MP_table_mode = 0x00;
	mp_table->number_of_assets=asset_info->assets_count;

	mp_table->MP_table_asset= (MP_table_asset_t *)malloc(sizeof(MP_table_asset_t )*(mp_table->number_of_assets));

	int count = 0;
	for(count=0; count<(mp_table->number_of_assets);count++)
	{
		mp_table->MP_table_asset[count].Identifier_mapping = (Identifier_mapping_t *)malloc(sizeof(Identifier_mapping_t));

		mp_table->MP_table_asset[count].Identifier_mapping->identifier_type=0x00;		//00 for asset_id
		mp_table->MP_table_asset[count].Identifier_mapping->identifier_mapping_byte = (Identifier_mapping_byte_t *)malloc(sizeof(Identifier_mapping_byte_t));
		mp_table->MP_table_asset[count].Identifier_mapping->identifier_mapping_byte->asset_id = (asset_id_t *)malloc(sizeof(asset_id_t));
		//asset_id_scheme is not correct now
		mp_table->MP_table_asset[count].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_scheme = 0;
		mp_table->MP_table_asset[count].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_length = strlen((asset_info+count)->asset_id);
		mp_table->MP_table_asset[count].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_byte = (unsigned char*)malloc(strlen((asset_info+count)->asset_id)*sizeof(char));
		memcpy(mp_table->MP_table_asset[count].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_byte,(asset_info+count)->asset_id,
				strlen((asset_info+count)->asset_id));

		char type_temp[4] = {*((asset_info+count)->asset_type),*((asset_info+count)->asset_type+1),*((asset_info+count)->asset_type+2),*((asset_info+count)->asset_type+3)};
		mp_table->MP_table_asset[count].asset_type = MP4_FOURCC(type_temp[1],type_temp[2],type_temp[3],type_temp[4]);

		mp_table->MP_table_asset[count].reserved = 0;
		mp_table->MP_table_asset[count].asset_clock_relation_flag= 0;

		mp_table->MP_table_asset[count].asset_loaction = (asset_location_t*)malloc(sizeof(asset_location_t));
		mp_table->MP_table_asset[count].asset_loaction->location_count = 1;
		//00 for packet_id
		mp_table->MP_table_asset[count].asset_loaction->general_location_info = (general_location_info_t *)malloc(sizeof(general_location_info_t));
		mp_table->MP_table_asset[count].asset_loaction->general_location_info->location_type = 0x00;
		mp_table->MP_table_asset[count].asset_loaction->general_location_info->general_location_info_byte = (general_location_info_byte_t *)malloc(sizeof(general_location_info_byte_t));
		mp_table->MP_table_asset[count].asset_loaction->general_location_info->general_location_info_byte->packet_id = (asset_info+count)->packet_id;

		mp_table->MP_table_asset[count].asset_descriptors_length= 0;
		mp_table->MP_table_asset[count].asset_descriptors_byte = NULL;

	}
//		mp_table.length=2+(4+1+6+2)*(mp_table.number_of_assets)+length_temp;
	mp_table->length = get_mp_table_length(mp_table)-4;

	return 0;
}

int read_mp_table(mp_table_t *mp_table ,const char *mp_table_buf)
{

	mp_table->table_id=*((u_int8_t*)&mp_table_buf[0]);
	mp_table->version=*((u_int8_t*)&mp_table_buf[1]);
	mp_table->length=ntohs(*((u_int16_t*)&mp_table_buf[2]));
	mp_table->reserved = (*((u_int8_t*)&mp_table_buf[4]))>>2;
	mp_table->MP_table_mode = (*((u_int8_t*)&mp_table_buf[4]))&(0x03);
	mp_table->number_of_assets=*((u_int8_t*)&mp_table_buf[5]);
	mp_table->MP_table_asset= (MP_table_asset_t *)malloc(sizeof(MP_table_asset_t)*mp_table->number_of_assets);

	u_int32_t  i,seekpoint=6;
	u_int32_t j,location_num;
	for(i=0;i<mp_table->number_of_assets;i++)
	{
		mp_table->MP_table_asset[i].Identifier_mapping = (Identifier_mapping_t *)malloc(sizeof(Identifier_mapping_t));
		mp_table->MP_table_asset[i].Identifier_mapping->identifier_type = *((u_int8_t*)&mp_table_buf[seekpoint]);
		seekpoint += 1;
		mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte = (Identifier_mapping_byte_t *)malloc(sizeof(Identifier_mapping_byte_t));

		//0x00 for asset_id
		if(mp_table->MP_table_asset[i].Identifier_mapping->identifier_type == 0x00)
		{
			u_int8_t length_tmp = 0;
			mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id = (asset_id_t *)malloc(sizeof(asset_id_t));
			mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_scheme = ntohs(*((u_int32_t *)&mp_table_buf[seekpoint]));
			length_tmp = *((u_int8_t *)&mp_table_buf[seekpoint+4]);

			seekpoint += 5;

			mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_length = length_tmp;
			//plus 1 for adding '\0' at the end
			mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_byte = (unsigned char*)malloc((length_tmp+1)*sizeof(char));
			memcpy(mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_byte,&mp_table_buf[seekpoint],length_tmp);
			mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_byte[length_tmp] = '\0';
			seekpoint += length_tmp;
		}
		else if(mp_table->MP_table_asset[i].Identifier_mapping->identifier_type == 0x01)
		{
			seekpoint += 0;
		}
		mp_table->MP_table_asset[i].asset_type = ntohs(*((u_int32_t *)&mp_table_buf[seekpoint]));
		mp_table->MP_table_asset[i].reserved = (*((u_int8_t*)&mp_table_buf[seekpoint+4]))>>1;
		mp_table->MP_table_asset[i].asset_clock_relation_flag = (*((u_int8_t*)&mp_table_buf[seekpoint+4]))&(0x01);

		mp_table->MP_table_asset[i].asset_loaction = (asset_location_t *)malloc(sizeof(asset_location_t));
		mp_table->MP_table_asset[i].asset_loaction->location_count = *((u_int8_t*)&mp_table_buf[seekpoint+5]);
		location_num = *((u_int8_t*)&mp_table_buf[seekpoint+5]);

		seekpoint += 6;

		mp_table->MP_table_asset[i].asset_loaction->general_location_info = (general_location_info_t *)malloc(sizeof(general_location_info_t)*location_num);
		for(j=0;j<location_num;j++)
		{
			mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].location_type = *((u_int8_t*)&mp_table_buf[seekpoint]);
			seekpoint += 1;
			//0x00 for packet_id
			mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].general_location_info_byte =
					(general_location_info_byte_t *)malloc(sizeof(general_location_info_byte_t));

			if(mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].location_type == 0x00)
			{
				mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].general_location_info_byte->packet_id =
						ntohs(*((u_int16_t *)&mp_table_buf[seekpoint]));
				seekpoint += 2;
			}
			//other location type
			else if(mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].location_type == 0x01)
			{
				seekpoint += 0;
			}
		}

		mp_table->MP_table_asset[i].asset_descriptors_length = ntohs(*((u_int16_t *)&mp_table_buf[seekpoint]));
		mp_table->MP_table_asset[i].asset_descriptors_byte = (unsigned char *)malloc((mp_table->MP_table_asset[i].asset_descriptors_length+1)*sizeof(char));
		memcpy(mp_table->MP_table_asset[i].asset_descriptors_byte,&mp_table_buf[seekpoint+2],mp_table->MP_table_asset[i].asset_descriptors_length);
		mp_table->MP_table_asset[i].asset_descriptors_byte[mp_table->MP_table_asset[i].asset_descriptors_length] = '\0';

		seekpoint += (2+mp_table->MP_table_asset[i].asset_descriptors_length);
	}

return 0;
}

/*
int  copy_mp_table(mp_table_t *mp_table_dst ,mp_table_t *mp_table_src)
{

	mp_table_dst->table_id=mp_table_src->table_id;
	mp_table_dst->version=mp_table_src->version;
	mp_table_dst->length=mp_table_src->length;
	mp_table_dst->number_of_assets=mp_table_src->number_of_assets;
	mp_table_dst->MP_table_asset= (MP_table_asset_t *)malloc(sizeof(MP_table_asset_t)*mp_table_src->number_of_assets);

	u_int32_t  i;
	for (i=0;i<mp_table_dst->number_of_assets;i++)
		{
			//memcpy(&mp_table_buf[seekpoint],"URL",strlen("URL"));
			//memcpy(&mp_table_buf[seekpoint+3],"\0",1);
			mp_table_dst->MP_table_asset[i].Identifier_mapping.URL_length=mp_table_src->MP_table_asset[i].Identifier_mapping.URL_length;
			mp_table_dst->MP_table_asset[i].Identifier_mapping.URL_byte=(unsigned  char*) malloc(mp_table_src->MP_table_asset[i].Identifier_mapping.URL_length*sizeof( unsigned  char));
			memcpy(mp_table_dst->MP_table_asset[i].Identifier_mapping.URL_byte,mp_table_src->MP_table_asset[i].Identifier_mapping.URL_byte,mp_table_src->MP_table_asset[i].Identifier_mapping.URL_length);
			mp_table_dst->MP_table_asset[i].asset_type=mp_table_src->MP_table_asset[i].asset_type;
			mp_table_dst->MP_table_asset[i].asset_clock_relation_flag=mp_table_src->MP_table_asset[i].asset_clock_relation_flag;
			mp_table_dst->MP_table_asset[i].location_count=mp_table_src->MP_table_asset[i].location_count;
			mp_table_dst->MP_table_asset[i].asset_descriptors_length=mp_table_src->MP_table_asset[i].asset_descriptors_length;
			mp_table_dst->MP_table_asset[i].asset_descriptors_byte=(unsigned  char*) malloc(mp_table_src->MP_table_asset[i].asset_descriptors_length*sizeof( unsigned  char));
			memcpy(mp_table_dst->MP_table_asset[i].asset_descriptors_byte,mp_table_dst->MP_table_asset[i].asset_descriptors_byte,mp_table_src->MP_table_asset[i].asset_descriptors_length);

		}


return 0;
}
*/

int free_mp_table(mp_table_t *mp_table )
{
	u_int32_t  i, location_num, j;
	for (i=0;i<mp_table->number_of_assets;i++)
		{
//			free(mp_table->MP_table_asset[i].Identifier_mapping.URL_byte);
//			free(mp_table->MP_table_asset[i].asset_descriptors_byte);
			if(mp_table->MP_table_asset[i].Identifier_mapping->identifier_type == 0x00)
			{
				free(mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_byte);
				mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte->asset_id->asset_id_byte = NULL;
			}
			free(mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte);
			mp_table->MP_table_asset[i].Identifier_mapping->identifier_mapping_byte = NULL;
			free(mp_table->MP_table_asset[i].Identifier_mapping);
			mp_table->MP_table_asset[i].Identifier_mapping = NULL;

			location_num = mp_table->MP_table_asset[i].asset_loaction->location_count;
			for(j=0;j<location_num;j++)
			{
				free(mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].general_location_info_byte);
				mp_table->MP_table_asset[i].asset_loaction->general_location_info[j].general_location_info_byte = NULL;
			//	free(&(mp_table->MP_table_asset[i].asset_loaction->general_location_info[j]));
			}
			free(mp_table->MP_table_asset[i].asset_loaction->general_location_info);
			mp_table->MP_table_asset[i].asset_loaction->general_location_info = NULL;
			free(mp_table->MP_table_asset[i].asset_loaction);
			mp_table->MP_table_asset[i].asset_loaction = NULL;
			free(mp_table->MP_table_asset[i].asset_descriptors_byte);
			mp_table->MP_table_asset[i].asset_descriptors_byte = NULL;
		}
	free(mp_table->MP_table_asset);
	mp_table->MP_table_asset = NULL;
	return 0;

}


int init_mpi_table(mpi_table_t *mpi_table, unsigned char **mpi_table_buf)
{
//	memset(mpi_table_buf1,0,1024);

	unsigned char *mpi_table_buf_tmp= (unsigned   char*) malloc((4+mpi_table->length)*sizeof( unsigned   char));
	 if(mpi_table_buf_tmp==NULL)
	 		{
	 			puts ("Memory allocation failed.");
	 			 exit (EXIT_FAILURE);
	 		}

	*((u_int8_t*)&mpi_table_buf_tmp[0])=mpi_table->table_id;
	*((u_int8_t*)&mpi_table_buf_tmp[1])=mpi_table->version;
	*((u_int16_t*)&mpi_table_buf_tmp[2])=htons(mpi_table->length);

	*((u_int8_t*)&mpi_table_buf_tmp[4])=((mpi_table->reserved1<<4)|(mpi_table->PI_mode<<2)|(mpi_table->reserved2));

	*((u_int16_t*)&mpi_table_buf_tmp[5]) = htons(mpi_table->MPIT_descriptor->descriptors_length);
	u_int32_t  i,seekpoint=7;

	if(mpi_table->MPIT_descriptor->descriptors_length != 0)
	{
		memcpy(&mpi_table_buf_tmp[seekpoint],mpi_table->MPIT_descriptor->MPIT_descriptors_byte,mpi_table->MPIT_descriptor->descriptors_length);
	}
//	printf("seekpoint%d\n",seekpoint);
//	printf("seek%s\n",mpi_table_buf_tmp);
//	printf("seek%c\n",mpi_table_buf_tmp[8]);

	seekpoint += mpi_table->MPIT_descriptor->descriptors_length;

	*((u_int8_t*)&mpi_table_buf_tmp[seekpoint]) = mpi_table->PI_content_count;
	seekpoint += 1;

	for (i=0;i<mpi_table->PI_content_count;i++)
	{
		*((u_int8_t*)&mpi_table_buf_tmp[seekpoint]) = mpi_table->PI_content[i].PI_content_type_length;
		memcpy(&mpi_table_buf_tmp[seekpoint+1],mpi_table->PI_content[i].PI_content_type_byte,mpi_table->PI_content[i].PI_content_type_length);
		seekpoint += 1+mpi_table->PI_content[i].PI_content_type_length;

		*((u_int8_t*)&mpi_table_buf_tmp[seekpoint])=mpi_table->PI_content[i].PI_content_name_length;
		memcpy(&mpi_table_buf_tmp[seekpoint+1] , mpi_table->PI_content[i].PI_content_name_byte , mpi_table->PI_content[i].PI_content_name_length);
		seekpoint += 1+mpi_table->PI_content[i].PI_content_name_length;

		*((u_int16_t*)&mpi_table_buf_tmp[seekpoint])=htons(mpi_table->PI_content[i].PI_content_descriptors_length);
		if(mpi_table->PI_content[i].PI_content_descriptors_length != 0)
		{
			memcpy(&mpi_table_buf_tmp[seekpoint+2] , mpi_table->PI_content[i].PI_content_descriptors_byte , mpi_table->PI_content[i].PI_content_descriptors_length);
		}
		seekpoint += 2+mpi_table->PI_content[i].PI_content_descriptors_length;

		*((u_int16_t*)&mpi_table_buf_tmp[seekpoint])=htons(mpi_table->PI_content[i].PI_content_length);
		memcpy(&mpi_table_buf_tmp[seekpoint+2] , mpi_table->PI_content[i].PI_content_byte , mpi_table->PI_content[i].PI_content_length);
		seekpoint += 2+mpi_table->PI_content[i].PI_content_length;
	}
	*mpi_table_buf=mpi_table_buf_tmp;
	return 0;
}


int make_mpi_table(mpi_table_t* mpi_table, PI_info_t* PI_info)
{
	mpi_table->table_id=0x10;
	mpi_table->version=0;
	mpi_table->length=0;
	mpi_table->reserved1=0;
	mpi_table->PI_mode=0;
	mpi_table->reserved2=0;

	//MPI table descriptors is not used now
	mpi_table->MPIT_descriptor = (MPIT_descriptors_t *)malloc(sizeof(MPIT_descriptors_t));
	mpi_table->MPIT_descriptor->descriptors_length=0;
	mpi_table->MPIT_descriptor->MPIT_descriptors_byte=NULL;
	mpi_table->PI_content_count=PI_info->PI_cotent_count;
	mpi_table->PI_content= (PI_content_t *)malloc(sizeof(PI_content_t )*mpi_table->PI_content_count);
		if(mpi_table->PI_content==NULL)
		{
			puts ("Memory allocation failed.");
			 exit (EXIT_FAILURE);
		}
		int pi_count=0, pi_length_tmp=0;

		for(pi_count=0;pi_count<mpi_table->PI_content_count;pi_count++)
		{
			mpi_table->PI_content[pi_count].PI_content_type_length = strlen(PI_info[pi_count].PI_content_type);
			mpi_table->PI_content[pi_count].PI_content_type_byte = (unsigned char*)malloc((mpi_table->PI_content[pi_count].PI_content_type_length)*sizeof(unsigned char));
			memcpy(mpi_table->PI_content[pi_count].PI_content_type_byte,PI_info[pi_count].PI_content_type,mpi_table->PI_content[pi_count].PI_content_type_length);

			mpi_table->PI_content[pi_count].PI_content_name_length=strlen(PI_info[pi_count].PI_content_name);
			mpi_table->PI_content[pi_count].PI_content_name_byte = (unsigned char*)malloc(mpi_table->PI_content[pi_count].PI_content_name_length*sizeof(unsigned char));
			memcpy(mpi_table->PI_content[pi_count].PI_content_name_byte,PI_info[pi_count].PI_content_name,mpi_table->PI_content[pi_count].PI_content_name_length);

			mpi_table->PI_content[pi_count].PI_content_descriptors_length = 0;
			mpi_table->PI_content[pi_count].PI_content_descriptors_byte = NULL;

			int readFlag = ReadFile(strcatex(PI_info[pi_count].PI_content_path,PI_info[pi_count].PI_content_name),(char **)&(mpi_table->PI_content[pi_count].PI_content_byte),
						&mpi_table->PI_content[pi_count].PI_content_length);
			if(readFlag == -1)
			{
				printf("can't find ci or html\n");
				exit(0);
			}

			pi_length_tmp += mpi_table->PI_content[pi_count].PI_content_type_length+mpi_table->PI_content[pi_count].PI_content_name_length+ \
					mpi_table->PI_content[pi_count].PI_content_descriptors_length+mpi_table->PI_content[pi_count].PI_content_length;
		}

		mpi_table->length = 1+2+mpi_table->MPIT_descriptor->descriptors_length+1+6*mpi_table->PI_content_count+pi_length_tmp;
		return 0;
}

int read_mpi_table(mpi_table_t *mpi_table, const char *mpi_table_buf)
{
		mpi_table->table_id=*((u_int8_t*)&mpi_table_buf[0]);
		mpi_table->version=*((u_int8_t*)&mpi_table_buf[1]);
		mpi_table->length=ntohs(*((u_int16_t*)&mpi_table_buf[2]));

		mpi_table->reserved1 = ((*((u_int8_t*)&mpi_table_buf[4]))>>4)&(0x0F);
		mpi_table->PI_mode = ((*((u_int8_t*)&mpi_table_buf[4]))>>2)&(0x03);
		mpi_table->reserved2 = (*((u_int8_t*)&mpi_table_buf[4]))&(0x03);

		mpi_table->MPIT_descriptor = (MPIT_descriptors_t *)malloc(sizeof(MPIT_descriptors_t));
		mpi_table->MPIT_descriptor->descriptors_length = ntohs(*((u_int16_t*)&mpi_table_buf[5]));

		u_int32_t  i,seekpoint=7;
		if(mpi_table->MPIT_descriptor->descriptors_length != 0)
		{
			mpi_table->MPIT_descriptor->MPIT_descriptors_byte = (unsigned char*)malloc((mpi_table->MPIT_descriptor->descriptors_length+1)*sizeof(unsigned char));
			memcpy(mpi_table->MPIT_descriptor->MPIT_descriptors_byte,&mpi_table_buf[seekpoint],mpi_table->MPIT_descriptor->descriptors_length);
			mpi_table->MPIT_descriptor->MPIT_descriptors_byte[mpi_table->MPIT_descriptor->descriptors_length] = '\0';
		}
		seekpoint += mpi_table->MPIT_descriptor->descriptors_length;

		mpi_table->PI_content_count=*((u_int8_t*)&mpi_table_buf[seekpoint]);
		mpi_table->PI_content= (PI_content_t *)malloc(sizeof(PI_content_t )*mpi_table->PI_content_count);
		seekpoint += 1;

		for (i=0;i<mpi_table->PI_content_count;i++)
		{
			mpi_table->PI_content[i].PI_content_type_length = *((u_int8_t*)&mpi_table_buf[seekpoint]);
			mpi_table->PI_content[i].PI_content_type_byte = (unsigned char*)malloc((mpi_table->PI_content[i].PI_content_type_length+1)*sizeof(unsigned char));
			memcpy(mpi_table->PI_content[i].PI_content_type_byte,&mpi_table_buf[seekpoint+1],mpi_table->PI_content[i].PI_content_type_length);
			mpi_table->PI_content[i].PI_content_type_byte[mpi_table->PI_content[i].PI_content_type_length] = '\0';
			seekpoint += 1+mpi_table->PI_content[i].PI_content_type_length;

			mpi_table->PI_content[i].PI_content_name_length = *((u_int8_t*)&mpi_table_buf[seekpoint]);
			mpi_table->PI_content[i].PI_content_name_byte = (unsigned  char*) malloc((mpi_table->PI_content[i].PI_content_name_length+1)*sizeof( unsigned  char));
			memcpy(mpi_table->PI_content[i].PI_content_name_byte , &mpi_table_buf[seekpoint+1] , mpi_table->PI_content[i].PI_content_name_length);
			mpi_table->PI_content[i].PI_content_name_byte[mpi_table->PI_content[i].PI_content_name_length] = '\0';
			seekpoint += 1+mpi_table->PI_content[i].PI_content_name_length;

			mpi_table->PI_content[i].PI_content_descriptors_length = ntohs(*((u_int16_t*)&mpi_table_buf[seekpoint]));
			if(mpi_table->PI_content[i].PI_content_descriptors_length != 0)
			{
				mpi_table->PI_content[i].PI_content_descriptors_byte = (unsigned  char*) malloc((mpi_table->PI_content[i].PI_content_descriptors_length+1)*sizeof( unsigned  char));
				memcpy(mpi_table->PI_content[i].PI_content_descriptors_byte , &mpi_table_buf[seekpoint+1] , mpi_table->PI_content[i].PI_content_descriptors_length);
				mpi_table->PI_content[i].PI_content_descriptors_byte[mpi_table->PI_content[i].PI_content_descriptors_length] = '\0';
			}
			seekpoint += 2+mpi_table->PI_content[i].PI_content_descriptors_length;

			mpi_table->PI_content[i].PI_content_length = ntohs(*((u_int16_t*)&mpi_table_buf[seekpoint]));
			mpi_table->PI_content[i].PI_content_byte=(unsigned  char*) malloc((mpi_table->PI_content[i].PI_content_length+1)*sizeof( unsigned  char));
			memcpy(mpi_table->PI_content[i].PI_content_byte ,&mpi_table_buf[seekpoint+2], mpi_table->PI_content[i].PI_content_length);
			mpi_table->PI_content[i].PI_content_byte[mpi_table->PI_content[i].PI_content_length] = '\0';
			seekpoint += 2+mpi_table->PI_content[i].PI_content_length;

			FILE *mpi;
			if((mpi=fopen((const char*)mpi_table->PI_content[i].PI_content_name_byte,"w+"))==NULL)
					{
						printf("not open");
						exit(0);
					}
			fwrite(mpi_table->PI_content[i].PI_content_byte,mpi_table->PI_content[i].PI_content_length,1,mpi);
			fclose(mpi);
		}
	 return 0;

}

int copy_mpi_table(mpi_table_t *mpi_table_dst, mpi_table_t  *mpi_table_src)
{
		mpi_table_dst->table_id=mpi_table_src->table_id;
		mpi_table_dst->version=mpi_table_src->version;
		mpi_table_dst->length=mpi_table_src->length;

		mpi_table_dst->PI_mode=mpi_table_src->PI_mode;
		//temp var
		mpi_table_dst->PI_content_count=mpi_table_src->PI_content_count;
		mpi_table_dst->PI_content= (PI_content_t *)malloc(sizeof(PI_content_t )*mpi_table_src->PI_content_count);
		u_int32_t  i;

		for (i=0;i<mpi_table_src->PI_content_count;i++)
		{
			//*((u_int8_t*)&mpi_table_buf[seekpoint])=(u_int8_t)(i+1);
			mpi_table_dst->PI_content[i].PI_content_name_length=mpi_table_src->PI_content[i].PI_content_name_length;
			mpi_table_dst->PI_content[i].PI_content_name_byte=(unsigned  char*) malloc((mpi_table_src->PI_content[i].PI_content_name_length+1)*sizeof( unsigned  char));
			memset(mpi_table_dst->PI_content[i].PI_content_name_byte,0,(mpi_table_src->PI_content[i].PI_content_name_length+1));
			memcpy(mpi_table_dst->PI_content[i].PI_content_name_byte , mpi_table_src->PI_content[i].PI_content_name_byte , mpi_table_src->PI_content[i].PI_content_name_length);
			mpi_table_dst->PI_content[i].PI_content_length=mpi_table_src->PI_content[i].PI_content_length;
			mpi_table_dst->PI_content[i].PI_content_byte=(unsigned  char*) malloc((mpi_table_src->PI_content[i].PI_content_length+1)*sizeof( unsigned  char));
			memset(mpi_table_dst->PI_content[i].PI_content_byte,0,(mpi_table_dst->PI_content[i].PI_content_length+1));
			memcpy( mpi_table_dst->PI_content[i].PI_content_byte ,mpi_table_src->PI_content[i].PI_content_byte , mpi_table_src->PI_content[i].PI_content_length);

        }
	 return 0;

}
int free_mpi_table(mpi_table_t *mpi_table)
{
	u_int32_t  i;
//
	if(mpi_table->MPIT_descriptor->MPIT_descriptors_byte!=NULL)
	{
		free(mpi_table->MPIT_descriptor->MPIT_descriptors_byte);
		mpi_table->MPIT_descriptor->MPIT_descriptors_byte =  NULL;
	}
	free(mpi_table->MPIT_descriptor);
	mpi_table->MPIT_descriptor = NULL;

	for (i=0;i<mpi_table->PI_content_count;i++)
	{
		free(mpi_table->PI_content[i].PI_content_type_byte);
		mpi_table->PI_content[i].PI_content_type_byte = NULL;
		free(mpi_table->PI_content[i].PI_content_name_byte);
		mpi_table->PI_content[i].PI_content_name_byte = NULL;
		if(mpi_table->PI_content[i].PI_content_descriptors_byte != NULL)
		{
			free(mpi_table->PI_content[i].PI_content_descriptors_byte);
			mpi_table->PI_content[i].PI_content_descriptors_byte = NULL;
		}
		free(mpi_table->PI_content[i].PI_content_byte);
		mpi_table->PI_content[i].PI_content_byte = NULL;
	}
	free(mpi_table->PI_content);
	mpi_table->PI_content = NULL;

	return 0;
}

int init_signal_header(signal_header_t *signal_header,unsigned char *Signal_h)

{
	*((u_int8_t*)&Signal_h[0])=(u_int8_t)signal_header->f_i<<6
								|(u_int8_t) signal_header->res<<2
								|(u_int8_t) signal_header->H<<1
								|(u_int8_t) signal_header->A;
	*((u_int8_t*)&Signal_h[1])=signal_header->frag_counter;
	*((u_int16_t*)&Signal_h[2])=htons(signal_header->MSG_length1);


	return 0;

}

int read_signal_header(signal_header_t *signal_header,const char *Signal_h)

{
	signal_header->f_i=(Signal_h[0]>>6)&0x03;
	signal_header->res=(Signal_h[0]>>2)&0x0f;
	signal_header->H=(Signal_h[0]>>1)&0x01;
	signal_header->A=(Signal_h[0])&0x01;
	signal_header->frag_counter=Signal_h[1];
	signal_header->MSG_length1=ntohs(*((u_int16_t*)&Signal_h[2]));


	*((u_int8_t*)&Signal_h[0])=(u_int8_t)signal_header->f_i<<6
								|(u_int8_t) signal_header->H<<2
								|(u_int8_t) signal_header->A<<1;
	*((u_int8_t*)&Signal_h[1])=signal_header->frag_counter;
	*((u_int16_t*)&Signal_h[2])=htons(signal_header->MSG_length1);


	return 0;

}


int send_signal(int ss,struct sockaddr*to,pa_message_t *pa_header,unsigned char *pa_message_buf,u_int32_t *packet_sequence_number,u_int32_t *packet_counter,u_int32_t packet_id)
{
		int pa_sequence_number=(pa_header->length+4+MMTP_BUFF_LEN-1)/(unsigned int)MMTP_BUFF_LEN;;
		int pa_length=pa_header->length+PAh_BUFF_LEN;
		u_int32_t buf_seekpoint=0;

		int counter;
		for (counter=0;counter<pa_sequence_number;counter++)
		{
			signal_header_t signal_header;
			if(counter==(pa_sequence_number-1))
			{
				signal_header.MSG_length1=pa_length%signal_BUFF_LEN;
				//有问题
				if(pa_length%MMTP_BUFF_LEN==0)
					{
					signal_header.MSG_length1=signal_BUFF_LEN;
					}
			}
			else
			{
				signal_header.MSG_length1=signal_BUFF_LEN;
			}
			signal_header.f_i=0;
			signal_header.res=0;
			signal_header.H=0;
			signal_header.A=0;
			signal_header.frag_counter=0;

			if(pa_sequence_number>1)
			{	signal_header.frag_counter=pa_sequence_number;
				if(counter==0)
					signal_header.f_i=1;
				else if(counter==(pa_sequence_number-1))
				{
					signal_header.f_i=3;
				}
				else
				{
					signal_header.f_i=2;
				}
			}
			else
			{
				signal_header.f_i=0;
				signal_header.frag_counter=1;
			}
			signal_header.A=0;

			unsigned char Signal_h[Signal_h_BUFF_LEN]={};
			init_signal_header(&signal_header,Signal_h);


			//16byte
			mmt_packet_header_t mmt_header;
			unsigned char MMTPh[MMTPh_BUFF_LEN];
			mmt_header.version=0;
			mmt_header.packet_counter_flag=1;
			mmt_header.FEC_type=0;
			mmt_header.reserved_1=0;
			mmt_header.extension_flag=0;
			mmt_header.RAP_flag=0;
			mmt_header.reserved_2=0;
			mmt_header.type=2;
			mmt_header.packet_id=packet_id;
			mmt_header.present_time=0;
			mmt_header.timestamp=get_send_timestamp();
			mmt_header.packet_sequence_number=*packet_sequence_number;
			mmt_header.packet_counter=(*packet_counter);

			init_mmtp_header(&mmt_header,MMTPh);

			char UDPbuff[UDP_BUFF_LEN]={};

			memcpy(UDPbuff,MMTPh,MMTPh_BUFF_LEN);
			//diyige jia tou
			if (signal_header.f_i==1)
			{
				memcpy(&UDPbuff[MMTPh_BUFF_LEN],Signal_h,Signal_h_BUFF_LEN);
				memcpy(&UDPbuff[MMTPh_BUFF_LEN+Signal_h_BUFF_LEN],&pa_message_buf[buf_seekpoint],signal_header.MSG_length1);
			}
			else
			{
				memcpy(&UDPbuff[MMTPh_BUFF_LEN],Signal_h,Signal_h_BUFF_LEN);
				memcpy(&UDPbuff[MMTPh_BUFF_LEN+Signal_h_BUFF_LEN],&pa_message_buf[buf_seekpoint],signal_header.MSG_length1);

			}
			buf_seekpoint=buf_seekpoint+signal_header.MSG_length1;
//			stream_read(s, &UDPbuff[MMTPh_BUFF_LEN+MPUh_BUFF_LEN], mpu_header.length);

			send_udp(ss,UDPbuff,MMTPh_BUFF_LEN+Signal_h_BUFF_LEN+signal_header.MSG_length1,to);

			if((*packet_counter)==maximum_value)
				{
					*packet_counter=0;
				}
			else
				{
				(*packet_counter)++;
				}
			if((*packet_sequence_number)==maximum_value)
				{
					*packet_sequence_number=0;
				}
			else
				{
				(*packet_sequence_number)++;
				}
		}
		printf("finished send signal\n");
return 0;

}




int send_udp(int ss,char *UDPbuff,int length,struct sockaddr*to)

{

	int n;
	struct timeval udp_delay;
	udp_delay.tv_sec = 0;
	udp_delay.tv_usec = 50; // 50 us
	select(0, NULL, NULL, NULL, &udp_delay);
//	printf("tmp%s/n",*tmp[1]);
	socklen_t len=sizeof(*to);
	n=sendto(ss,UDPbuff,length,0,to,len);
	if(n <=0)
	{
		perror("send error");
		puts ("send error");
		exit (EXIT_FAILURE);
		return -1;
	}

//	send_raw_udp(UDPbuff,length);
	return 0;
}

//int free_mfu_buf(mfu_buf_t *mfu_buf )
//{
//	free(mfu_buf->data);
//	return 0;
//}
int free_mpu_metadata_buf(mpu_metadata_buf_t *mpu_metadata_buf )
{
	free(mpu_metadata_buf->data);
	mpu_metadata_buf->data = NULL; 
	return 0;
}
int free_fragment_metadata_buf(fragment_metadata_buf_t *fragment_metadata_buf )
{
	free(fragment_metadata_buf->data);
	fragment_metadata_buf->data = NULL; 
	return 0;
}
int free_mpu_buf(mpu_buf_t *mpu_buf )
{
	free_mpu_metadata_buf(&mpu_buf->mpu_metadata_buf);
	//  printf("mpu_buf->item_count: %d\n",mpu_buf->item_number);
	//  printf("mpu_buf->mfu_number: %d\n",mpu_buf->mfu_number);
	int i;
	if (mpu_buf->mfu_number >0) 
	{
	    for(i=0;i<mpu_buf->mfu_number;i++)
	    {
		free(mpu_buf->mfu[i].data);
		mpu_buf->mfu[i].data = NULL;
	    }
	    free_fragment_metadata_buf(&mpu_buf->fragment_metadata_buf);
	    
	    free(mpu_buf->mfu);
	    mpu_buf->mfu = NULL;
	    free(mpu_buf->mfu_sample);
	    mpu_buf->mfu_sample = NULL;
	}
	
	if (mpu_buf->item_number >0)
	{
	    for(i=0;i<mpu_buf->item_number;i++)
	    {
		    free(mpu_buf->item[i].data);
		    mpu_buf->item[i].data = NULL;
	    }
	    free(mpu_buf->item);
	    mpu_buf->item = NULL;
	}
	free(mpu_buf->MPU_sequence_data);
	mpu_buf->MPU_sequence_data = NULL;
	init_mpu_buf(mpu_buf);
	return 0;

}

int init_mpu_buf(mpu_buf_t *mpu_buf )
{
	mpu_buf->packet_id=0;
	mpu_buf->MPU_sequence_number=0;
	mpu_buf->flag=0;
	mpu_buf->mfu_number=0;
	mpu_buf->mfu_counter=0;
	mpu_buf->item_number=0;
	mpu_buf->item_counter=0;
	mpu_buf->MPU_sequence_length=0;
	mpu_buf->state=0;
	return 0;
}
