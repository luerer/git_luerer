#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>   /* open(), creat() - and fcntl() */
#include <dirent.h>
#include <signal.h>
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

extern void sig_proccess(int signo);
extern void sig_pipe(int signo);
static int s;

int pathConvert(char *srcPath, char **desPath, char *cfgPath)
{
	char cfg_tmp[MAX_PATH_LEN];
	char des_tmp[MAX_PATH_LEN];
	char src_tmp[MAX_PATH_LEN];
	char tmp[MAX_PATH_LEN];

	int i = 0;
	for(i = strlen(cfgPath); i>0; i--)
		if(cfgPath[i] == '/')
			break;
	memcpy(cfg_tmp,cfgPath,i+1);
	cfg_tmp[i+1] = '\0';

	if(srcPath[0] == '/')
	//absolute path
	{
		memcpy(des_tmp,srcPath,strlen(srcPath));
		des_tmp[strlen(srcPath)] = '\0';
		*desPath =strdup(des_tmp);
	}
	else if(srcPath[0] == '~')
	{
		printf("~ is not a valid value\n");
		exit(0);
	}
	else if(srcPath[0] == '.' && srcPath[1] == '/')
	{
		memcpy(src_tmp,&srcPath[2],strlen(srcPath)-2);
		src_tmp[strlen(srcPath)-2] = '\0';
		strcat(cfg_tmp,src_tmp);
		memcpy(des_tmp,cfg_tmp,strlen(cfg_tmp));
		des_tmp[strlen(cfg_tmp)] = '\0';
//		printf("des =%s\n",des_tmp);

		*desPath = strdup(des_tmp);
	}
	else if(srcPath[0] == '.' && srcPath[1] == '.')
	{

		int j = 0;
		for(j=strlen(cfg_tmp)-2;j>0;j--)
		{
			if(cfg_tmp[j] == '/')
				break;
		}
		memcpy(tmp,cfg_tmp,j+1);
		tmp[j+1] = '\0';

		memcpy(src_tmp,srcPath+3,strlen(srcPath)-3);
		src_tmp[strlen(srcPath)-3] = '\0';
		strcat(tmp,src_tmp);
		memcpy(des_tmp,tmp,strlen(tmp));
		des_tmp[strlen(tmp)] = '\0';

		*desPath = strdup(des_tmp);

	}
	else
	{
		printf("not a correct value for asset_path\n");
		exit(0);
	}
	return 0;
}

int readAssetConf(asset_info_t **asset_info, char *configFile)
{

	printf("readAssetConf %s\n", configFile);
	ini_t *conf = ini_load(configFile);
	if (conf == NULL)
		error(1, errno, "ini_load fail");

	u_int16_t last_time = 0;
	if(ini_read_uint16(conf,(char *)"global",(char *)"last_time",&(last_time),0))
	{
		printf("can't get last_time\n");
		exit(0);
	}

	u_int8_t number_of_assets = 0x00;
	ini_read_uint8(conf, (char *)"MPT", (char *)"number_of_assets", &number_of_assets, 0);
//    printf("number_of_assets: %i\n", number_of_assets);

	asset_info_t* asset_info_temp = (asset_info_t *) malloc(
			sizeof(asset_info_t) * number_of_assets);

	int k = 0;
	for(k=0;k<number_of_assets;k++)
	{
		(asset_info_temp+k)->last_time = last_time;
		(asset_info_temp+k)->assets_count = number_of_assets;
	}

	



	int i = 0;
	for (i = 0; i < number_of_assets; i++)
	{

		char asset_id[MAX_ATTR_LEN];
		sprintf(asset_id, "asset_id%d", i + 1);
		if(ini_read_str(conf, (char *)"MPT", asset_id, &((asset_info_temp + i)->asset_id),NULL))
		{
			printf("can't get asset%d asset_id\n",i+1);
			exit(0);
		}

		char media_type[MAX_ATTR_LEN];
		sprintf(media_type, "media_type%d", i + 1);
		if(ini_read_str(conf, (char *)"MPT", media_type, &((asset_info_temp + i)->media_type),NULL))
		{
			printf("can't get asset%d media_type\n",i+1);
			exit(0);
		}

		char asset_type[MAX_ATTR_LEN];
		sprintf(asset_type, "asset_type%d", i + 1);
		if(ini_read_str(conf, (char *)"MPT", asset_type,&((asset_info_temp + i)->asset_type), NULL))
		{
			printf("can't get asset%d asset_type\n",i+1);
			exit(0);
		}

		char packet_id[MAX_ATTR_LEN];
		sprintf(packet_id, "packet_id%d", i + 1);
		if(ini_read_uint16(conf, (char *)"MPT", packet_id, &((asset_info_temp +i)->packet_id), 0))
		{
			printf("can't get asset%d packet_id\n",i+1);
			exit(0);
		}

		char asset_path[MAX_ATTR_LEN];
		sprintf(asset_path, "asset_path%d", i + 1);
		if(ini_read_str(conf, (char *)"MPT", asset_path, &((asset_info_temp + i)->asset_path), NULL))
		{
			printf("can't get asset%d asset_path\n",i+1);
			exit(0);
		}

		char asset_send_begintime[MAX_ATTR_LEN];
		sprintf(asset_send_begintime,"asset_send_begintime%d", i+1);
		if(ini_read_str(conf,(char *)"MPT",asset_send_begintime,&((asset_info_temp + i)->asset_send_begintime),NULL))
		{
			printf("can't get asset%d asset_send_begintime\n",i+1);
			exit(0);
		}

		char asset_send_endtime[MAX_ATTR_LEN];
		sprintf(asset_send_endtime,"asset_send_endtime%d", i+1);
		if(ini_read_str(conf,(char *)"MPT",asset_send_endtime,&((asset_info_temp + i)->asset_send_endtime),NULL))
		{
			printf("can't get asset%d asset_send_edntime\n",i+1);
			exit(0);
		}
	}


	ini_free(conf);
	int j = 0;
	char asset_tmp_path[MAX_PATH_LEN];
	for(j=0;j<number_of_assets;j++)
	{
		memcpy(asset_tmp_path,(asset_info_temp+j)->asset_path,strlen((asset_info_temp+j)->asset_path));
		asset_tmp_path[strlen((asset_info_temp+j)->asset_path)] = '\0';
		if(pathConvert(asset_tmp_path,&((asset_info_temp+j)->asset_path),configFile))
		{
			printf("path convert error\n");
		}
		printf("asset_path=%s\n",(asset_info_temp+j)->asset_path);

	}

//	printf("asset_tmp_path =%s\n",asset_tmp_path);

	*asset_info = asset_info_temp;
	return 0;
}

int readPIConf(PI_info_t **PI_info,char *configFile)
{
	ini_t *conf = ini_load(configFile);
	if (conf == NULL)
		error(1, errno, "ini_load fail");
	int8_t PI_content_count;
	ini_read_int8(conf, (char *)"MPIT", (char *)"PI_content_count", &PI_content_count, 0);

	PI_info_t *PI_info_temp = (PI_info_t *) malloc(sizeof(PI_info_t) * PI_content_count);
	PI_info_temp->PI_cotent_count = PI_content_count;
	int i = 0;
	for (i = 0; i < PI_content_count; i++) {
		char PI_content_type[MAX_ATTR_LEN];
		sprintf(PI_content_type, "PI_content_type%d", i + 1);
		if(ini_read_str(conf, (char *)"MPIT", PI_content_type,	&((PI_info_temp + i)->PI_content_type), NULL))
		{
			printf("can't get PI%d PI_content_type\n",i+1);
			exit(0);
		}

		char PI_type[MAX_ATTR_LEN];
		sprintf(PI_type, "PI_type%d", i + 1);
		if(ini_read_str(conf, (char *)"MPIT", PI_type,	&((PI_info_temp + i)->PI_type), NULL))
		{
			printf("can't get PI%d PI_type\n",i+1);
			exit(0);
		}

		char PI_content_name[MAX_ATTR_LEN];
		sprintf(PI_content_name, "PI_content_name%d", i + 1);
		if(ini_read_str(conf, (char *)"MPIT", PI_content_name,	&((PI_info_temp + i)->PI_content_name), NULL))
		{
			printf("can't get PI%d PI_content_name\n",i+1);
			exit(0);
		}

		char PI_content_path[MAX_ATTR_LEN];
		sprintf(PI_content_path, "PI_content_path%d", i + 1);
		if(ini_read_str(conf, (char *)"MPIT", PI_content_path,	&((PI_info_temp + i)->PI_content_path), NULL))
		{
			printf("can't get PI%d PI_content_path\n",i+1);
			exit(0);
		}


	}

	ini_free(conf);

	int j = 0;
	char PT_tmp_path[MAX_PATH_LEN];
	for(j=0;j<PI_content_count;j++)
	{
		memcpy(PT_tmp_path,(PI_info_temp+j)->PI_content_path,strlen((PI_info_temp+j)->PI_content_path));
		PT_tmp_path[strlen((PI_info_temp+j)->PI_content_path)] = '\0';
		if(pathConvert(PT_tmp_path,&((PI_info_temp+j)->PI_content_path),configFile))
		{
			printf("path convert error\n");
		}
		printf("PI_path=%s\n",(PI_info_temp+j)->PI_content_path);

	}
	*PI_info = PI_info_temp;
	return 0;
}

int read_signalingConf(asset_info_t **asset_info, PI_info_t **PI_info, char *configFile)
{
	readAssetConf(asset_info,configFile);
	readPIConf(PI_info,configFile);
	return 0;
}

void make_file(char *mpu_name, char *data, int len) {
	FILE *fd = fopen(mpu_name, "wb+");
	if (fd) {
		fwrite(data, len, 1, fd);
		fclose(fd);
	} else {
		printf("not open\n");
	}
}

void sig_proccess(int signo)
{
	printf("catch a exit signal\n");
	close(s);
	exit(0);

}

void  sig_pipe(int signo)
{
	printf("catch a SIGPIPE signal\n");
	close(s);
	exit(0);

}


void  read_config(mmt_config_t *m_config, char * MMTConfigFile)
{
	/* $ export MALLOC_TRACE=malloc.log */
	    mtrace();
	// read the config.ini
	ini_t *conf = ini_load(MMTConfigFile);
	if (conf == NULL)
		{
		error(1, errno, "ini_load fail");
		}
	ini_read_str(conf, (char *)"addr", (char *)"ipv4", &(m_config->to_addr), (char *)"224.1.1.100");
	ini_read_int16(conf, (char *)"addr", (char *)"port", &(m_config->to_port), 6080);
//	ini_free(conf);

	ini_read_int32(conf, (char *)"main", (char *)"session_num", &(m_config->session_num), 5000);
	ini_read_str(conf, (char *)"main", (char *)"mode", &(m_config->mode), (char *)"broadcast");
	ini_read_int16(conf, (char *)"main", (char *)"listen_port", &(m_config->listen_port), 6060);
	ini_read_str(conf, (char *)"main", (char *)"multicast_ip", &(m_config->multicast_ip), (char *)"224.1.1.104");
	ini_read_int16(conf, (char *)"main", (char *)"multicast_port", &(m_config->multicast_port), 6080);
	ini_read_str(conf, (char *)"main", (char *)"broadband_root", &(m_config->broadband_root), NULL);
	ini_read_str(conf, (char *)"main", (char *)"broadcast_root", &(m_config->broadcast_root), NULL);
	ini_read_int32(conf, (char *)"main", (char *)"fec", &(m_config->fec),0);
	ini_read_int32(conf, (char *)"main", (char *)"qos", &(m_config->qos), 0);
	ini_read_str(conf, (char *)"main", (char *)"log_path", &(m_config->log_path), (char *)"mmt_server.log");
	ini_read_str(conf, (char *)"main", (char *)"controller", &(m_config->controller), (char *)"127.0.0.1:8088");
	ini_read_int32(conf, (char *)"main", (char *)"max_client_num", &(m_config->session_num), 500);
	ini_read_str(conf, (char *)"main", (char *)"res_map_file", &(m_config->res_map_file), (char *)"adv.map");
//	ini_read_str(conf, "main", "broadcast_default_ci", &(m_config->broadcast_default_ci), "Triathlon.ci");
//	ini_read_str(conf, "main", "broadcast_default_html", &(m_config->broadcast_default_html), "nav2.html");
	ini_read_int16(conf, (char *)"main", (char *)"multiview_port", &(m_config->multiview_port), 8080);

	ini_free(conf);

/*
	asset_info_t * asset_info = (asset_info_t *)malloc(sizeof(asset_info_t));
	readAssetConf(&asset_info,AssetConfigFile);
	m_config->number_of_assets = asset_info->assets_count;
	int i = 0;
	for(i=0;i<(asset_info->assets_count);i++)
	{
		char * media_type_temp = (asset_info+i)->media_type;
		if(strcmp(media_type_temp,"audio")==0)
		{
			m_config->audio_dir = (asset_info+i)->asset_path;
			m_config->audio_asset_id = (asset_info+i)->asset_id;
			m_config->audio_packet_id = (asset_info+i)->packet_id;
			m_config->audio_type = (asset_info+i)->asset_type;

		}
		else if(strcmp(media_type_temp,"video")==0)
		{
			m_config->video_dir = (asset_info+i)->asset_path;
			m_config->video_asset_id = (asset_info+i)->asset_id;
			m_config->video_packet_id = (asset_info+i)->packet_id;
			m_config->video_type = (asset_info+i)->asset_type;
		}
		else if(strcmp(media_type_temp,"image")==0)
		{
			//图片处理
		}

	}

	int i = 0;
	PI_info_t * PI_info = (PI_info_t *)malloc(sizeof(PI_info_t));
	readPIConf(&PI_info,AssetConfigFile);
	for(i=0;i<PI_info->PI_cotent_count;i++)
	{
		char *PI_type_temp = (PI_info+i)->PI_type;
		if(strcmp(PI_type_temp,"ci")==0)
		{
			m_config->ci_dir = (PI_info+i)->PI_content_path;
			m_config->ci_name = (PI_info+i)->PI_content_name;
		}
		else if(strcmp(PI_type_temp,"html")==0)
		{
			m_config->html_dir = (PI_info+i)->PI_content_path;
			m_config->html_name = (PI_info+i)->PI_content_name;
		}
	}
*/
}

int udp_send(int ss, struct sockaddr *to, mmt_config_t *m_config, asset_info_t * asset_info, PI_info_t *PI_info)
{
//	u_int32_t packet_sequence_number_audio=0;
//	u_int32_t packet_sequence_number_video=0;
	u_int32_t packet_sequence_number_signal=0;
	u_int32_t packet_counter=0;
	u_int32_t packet_sequence_number_media[MAX_SEND_ASSET_NUM] = {0};

	u_int32_t MPU_sequence_number[MAX_SEND_ASSET_NUM] = {0};
	u_int32_t MPU_present_time=0;
	MPU_present_time=get_send_timestamp()+1040;
	struct timeval lasttime,nowtime;
	struct tm timestamp_ptr;
	time_t now_timestamp, begin_timestamp, end_timestamp;
	time_t begin_timestamp4all;	
	gettimeofday(&lasttime, NULL);

//	char *media_dir;
	char **media_list;
	u_int32_t media_count;
//	char *media_mime_type;
	u_int64_t time_count = 0;
	u_int32_t readConfFlag = 0;


	time(&now_timestamp);
	time(&begin_timestamp4all);
	while(now_timestamp < begin_timestamp4all+asset_info->last_time)
	{
		//delay 0.5s send next mpu

		gettimeofday(&nowtime, NULL);
		uint32_t Time_difference;
		Time_difference=(nowtime.tv_sec*1000+nowtime.tv_usec/1000)-(lasttime.tv_sec*1000+lasttime.tv_usec/1000);
		struct timeval delay;
		delay.tv_sec = 0;
		delay.tv_usec = 10 * 1000; // 10 ms
		while(Time_difference<500*time_count)
		{
			select(0, NULL, NULL, NULL, &delay);
			gettimeofday(&nowtime, NULL);
			Time_difference=(nowtime.tv_sec*1000+nowtime.tv_usec/1000)-(lasttime.tv_sec*1000+lasttime.tv_usec/1000);
		}
		time_count++;

		//delay 5 seconds to reread conf
#if 0
		readConfFlag ++;
		if(readConfFlag == 10)
		{
			if(readAssetConf(&asset_info,(char *)"Signaling.ini"))
			{
				printf("reread Signaling.ini error");
			}
			readConfFlag = 0;
		}

#endif
		char *Fullpath;
		u_int32_t packet_id;

		//PA table
		pa_table_t pa_table ;
		unsigned char pa_table_buf[1024];
		pa_table.table_id=0x00;
		pa_table.version=0;
		pa_table.length=0;
		pa_table.pat_content=NULL;
		init_pa_table(&pa_table,pa_table_buf);

		//MP table
		mp_table_t mp_table;

		make_mp_table(&mp_table,asset_info);
		unsigned char *mp_table_buf;
		mp_table_buf= (unsigned  char*) malloc((4+mp_table.length)*sizeof( unsigned  char));
		if(mp_table_buf==NULL)
			{
				puts ("Memory allocation failed.");
				exit (EXIT_FAILURE);
			}

		init_mp_table(&mp_table,&mp_table_buf);

		free_mp_table(&mp_table);
		//MPI table
		mpi_table_t mpi_table;

		make_mpi_table(&mpi_table, PI_info);
		unsigned char *mpi_table_buf;
		mpi_table_buf= (unsigned  char*) malloc((4+mpi_table.length)*sizeof( unsigned  char));
		if(mpi_table_buf==NULL)
			{
				puts ("Memory allocation failed.");
				exit (EXIT_FAILURE);
			}
		init_mpi_table(&mpi_table,&mpi_table_buf);

		free_mpi_table(&mpi_table);

		//PA message

		pa_message_t pa_header ;
		unsigned char PAh[8];
		pa_header.message_id=0x0000;
		pa_header.version=0;
		pa_header.length=0;
		pa_header.number_of_tables=3;

		//*2 because of include each table header twice
		pa_header.length=pa_table.length+mp_table.length+mpi_table.length+table_header_LEN*pa_header.number_of_tables*2+1;


		init_pa_message(&pa_header,PAh);

		unsigned char *pa_message_buf= (unsigned  char*) malloc((7+pa_header.length)*sizeof( unsigned  char));
		if(pa_message_buf==NULL)
			{
				puts ("Memory allocation failed.");
				exit (EXIT_FAILURE);
			}
		u_int32_t pa_seekpoint=0;
		memcpy(&pa_message_buf[pa_seekpoint] , PAh , 8);
		pa_seekpoint=pa_seekpoint+8;
		memcpy(&pa_message_buf[pa_seekpoint] , mp_table_buf , 4);
		pa_seekpoint=pa_seekpoint+4;
		memcpy(&pa_message_buf[pa_seekpoint] , mpi_table_buf , 4);
		pa_seekpoint=pa_seekpoint+4;
		memcpy(&pa_message_buf[pa_seekpoint] , pa_table_buf , 4);
		pa_seekpoint=pa_seekpoint+4;
		memcpy(&pa_message_buf[pa_seekpoint] , mp_table_buf , mp_table.length+4);
		pa_seekpoint=pa_seekpoint+mp_table.length+4;
		memcpy(&pa_message_buf[pa_seekpoint] , mpi_table_buf , mpi_table.length+4);
		pa_seekpoint=pa_seekpoint+mpi_table.length+4;
		memcpy(&pa_message_buf[pa_seekpoint] , pa_table_buf , pa_table.length+4);
		pa_seekpoint=pa_seekpoint+pa_table.length+4;

		send_signal(ss,to,&pa_header,pa_message_buf,&packet_sequence_number_signal,&packet_counter,Signal_PACKET_ID);

//		system("clear");
		u_int32_t asset_counter = 0;
		for(asset_counter=0;asset_counter<asset_info->assets_count;asset_counter++)
		{
//			char * timestamp_tmp = asset_info[asset_counter].asset_send_begintime;
			//converting utc time to timestamp in second
			if(asset_info[asset_counter].asset_send_begintime != NULL)
			{
				strptime(asset_info[asset_counter].asset_send_begintime,"%Y-%m-%dT%H:%M:%S",&timestamp_ptr);
				begin_timestamp = mktime(&timestamp_ptr);
			}
			else
			{
				printf("asset%d send begin time is null\n",asset_counter);
				exit(0);
			}
			if(asset_info[asset_counter].asset_send_endtime != NULL)
			{
				strptime(asset_info[asset_counter].asset_send_endtime,"%Y-%m-%dT%H:%M:%S",&timestamp_ptr);
				end_timestamp = mktime(&timestamp_ptr);
			}
			else
			{
				printf("asset%d send end time is null\n",asset_counter);
				exit(0);
			}


			//get now timestamp
			time(&now_timestamp);

			if(now_timestamp <= begin_timestamp)
			{
				printf("time for %dth asset is early\n",asset_counter+1);
				continue;
			}
			else if(begin_timestamp< now_timestamp && now_timestamp< end_timestamp)
			{
				//sending video and audio
			if(strcmp(asset_info[asset_counter].media_type,"video")==0 || strcmp(asset_info[asset_counter].media_type,"audio")==0)
			{
				getFileNameArray(asset_info[asset_counter].asset_path,&media_count, &media_list);
				packet_id = asset_info[asset_counter].packet_id;
				get_full_path(asset_info[asset_counter].asset_path,media_list[MPU_sequence_number[asset_counter]],&Fullpath);
				send_mpu (ss,to,Fullpath,&packet_sequence_number_media[asset_counter],&packet_counter,MPU_sequence_number[asset_counter],&MPU_present_time,packet_id);
				MPU_sequence_number[asset_counter]++;

				if(MPU_sequence_number[asset_counter] == media_count)
				{
					MPU_sequence_number[asset_counter] = 0;
				}
			}

			//sending image
			else if(strcmp(asset_info[asset_counter].media_type,"image")==0)
			{
				getFileNameArray(asset_info[asset_counter].asset_path,&media_count, &media_list);
				packet_id = asset_info[asset_counter].packet_id;
				get_full_path(asset_info[asset_counter].asset_path,media_list[MPU_sequence_number[asset_counter]],&Fullpath);
				send_nontimed_mpu (ss,to,Fullpath,&packet_sequence_number_media[asset_counter],&packet_counter,MPU_sequence_number[asset_counter],&MPU_present_time,packet_id);
				MPU_sequence_number[asset_counter]++;

				if(MPU_sequence_number[asset_counter] == media_count)
				{
					MPU_sequence_number[asset_counter] = 0;
				}
			}

			
			}

			else
			{
				printf("the %dth asset is out of date\n",asset_counter+1);
			}


		}

		MPU_present_time=MPU_present_time+500;

	}

	return 1;
}


int main(int argc, char *argv[])
{
	mmt_config_t m_config ;
	asset_info_t *asset_info;
	PI_info_t *PI_info;

	read_config (&m_config,(char *)"config.ini");
	read_signalingConf(&asset_info,&PI_info,(char *)"Signaling.ini");

	//初始化
	int ss;

	//挂接sigint信号
	signal(SIGINT,sig_proccess);
	signal(SIGPIPE,sig_pipe);

	struct sockaddr_in sendto_addr;
	//建立套接字
	ss=socket(AF_INET, SOCK_DGRAM,0);
	if(ss<0)
	{
		printf("socket error\n");
		return -1;
	}

	//设置服务器地址
	bzero(&sendto_addr,sizeof(sendto_addr));
	sendto_addr.sin_family=AF_INET;
	//sendto_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	sendto_addr.sin_addr.s_addr=inet_addr(m_config.to_addr);
	sendto_addr.sin_port =htons(m_config.to_port);

    /*
     * 设置发送缓冲区大小
     */
	int snd_size=0;//设置为64K
	socklen_t optlen;    /* 选项值长度 */
	optlen = sizeof(snd_size);
	if(getsockopt(ss, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen)<0)
	{
		printf("获取发送缓冲区大小错误\n");
		exit(2);
	}
	printf(" 发送缓冲区原始大小为: %d 字节\n",snd_size);
	snd_size=64*1024;//设置为64K
	if (setsockopt(ss,SOL_SOCKET,SO_SNDBUF,(const char*)&snd_size,sizeof(int)) < 0)
	{
         printf("fail to change SNDbuf.\n");
         exit(2);
	 }
	 /*g/server: ini_load fail: No such file or directory
	  *
	     * 检查上述缓冲区设置的情况
	     * 获得修改后发送缓冲区大小
	     */


	if(getsockopt(ss, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen)<0)
	{
		printf("获取发送缓冲区大小错误\n");
		exit(2);
	}
	printf(" 发送缓冲区大小为: %d 字节\n",snd_size);
//	//设置BLOADCASE选项
//	int ret;
//	int so_broadcast =1;
//	ret = setsockopt(ss, SOL_SOCKET, SO_BROADCAST, &so_broadcast,sizeof(so_broadcast));
//	if(ret ==-1)
//	{
//		printf("setsockopt failed\n");
//		return -1;
//	}

//	while(1)
	{
		printf("haha\n");
		udp_send(ss,(struct sockaddr*)&sendto_addr,&m_config,asset_info,PI_info);
	}

	close(ss);
	return 0;
}

