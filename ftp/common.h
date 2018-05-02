
#ifndef _COMMON_H_H
#define _COMMON_H_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <dirent.h>

#define NET_SERVER  11
#define NET_CLIENT  22


#define BUFF_LEN 1028//接受发缓冲区大小


#define CMD_LS         11//列出客户端所有文件
#define CMD_SERVER_LS  22//列出服务器所有文件文件
#define CMD_DOWNLOAD   33//下载文件
#define CMD_UPLOAD     44//上传文件
#define CMD_QUIT       55//退出
#define CMD_ERROR      -1//错误


#define E_NOFILE    "ERROR:No such file or directory!\n"
#define E_DODNLOAD  "ERROR:Download error!\n"
#define E_UPLOAD    "ERROR:Upload error!\n"
#define GET_LIST_END "SUCCESS:GET LIST SUCCESS!"

int     network_init(int net_type, const char* IP_ADDRESS,short INET_NUM);


void    ftp_print_help(void);
int     ftp_cmd_analyse(const char* cmd);
int     ftp_getlist(int getsockfd);
int     ftp_putlist(int putsockfd);
int     ftp_getfile(int getsockfd, const char* GET_FILENAME);
int     ftp_putfile(int putsockfd, const char* PUT_FILENAME);

#define BACKLOG 10//最大连接数10


/******************网络初始化*****************/
int network_init(int net_type, const char* NET_IP, short NET_NUM)
{
	int sockfd;
	if(-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))   //创建监听套接字
	{
		perror("Create socket error!");
		//exit(EXIT_FAILURE);
		return -1;
	}


	struct sockaddr_in sockadd;
	memset(&sockadd, 0, sizeof(sockadd));
	sockadd.sin_family = AF_INET;
	sockadd.sin_port = htons(NET_NUM);
	sockadd.sin_addr.s_addr = inet_addr(NET_IP);


	if(NET_CLIENT == net_type)
	{
		//连接服务器
		if(-1 == connect(sockfd, (struct sockaddr*)(&sockadd),\
			sizeof(sockadd)))
		{
			perror("Connect error!");
			//exit(EXIT_FAILURE);
			return -1;
		}
	}
	else
	{
		if(NET_SERVER == net_type)
		{
			//绑定IP地质端口号
			if(-1 == bind(sockfd, (struct sockaddr*)(&sockadd),\
				sizeof(sockadd)))
			{
				perror("Bind error!");
				return -1;
			}

			if(-1 == listen(sockfd, BACKLOG))   //监听客户端
			{
				perror("Listen error!");
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	return sockfd;
}


/*****************命令解析函数************************/
int ftp_cmd_analyse(const char* cmd)
{
	if(NULL == cmd)
	{
		return CMD_ERROR;
	}
	else
	{
		if(0 == strncmp(cmd, "ls",2))//列出本地文件列表指令
			return CMD_LS;
		else
		{
			if(0 == strncmp(cmd, "server ls", 9))   //列出服务器文件列表指令
				return CMD_SERVER_LS;
			else
			{
				if(0 == strcmp(cmd, "quit"))   //断开链接指令
					return CMD_QUIT;
				else
				{
					if(0 == strncmp(cmd, "download ", 9)) //从服务器下载文件指令
						return CMD_DOWNLOAD;
					else
					{
						if(0 == strncmp(cmd, "upload ", 7)) //上传文件到服务器指令
							return CMD_UPLOAD;
						else
						{
							return CMD_ERROR;
						}
					}
				}
			}   
		}
	}
}


/****************从服务器端获取文件列表函数*******/
int ftp_getlist(int getsockfd)
{
	char GET_BUFF[BUFF_LEN];
	int readsize;
	sprintf(GET_BUFF,  "server ls");
	if(-1 == write(getsockfd, GET_BUFF, BUFF_LEN)) //向服务器发送命令
	{
		perror("Send cmd error!");
		return -1;
	}
	else
	{
		while(1)   //循环读取
		{
			readsize = read(getsockfd, GET_BUFF, BUFF_LEN);
			if(readsize <= 0)   //读错误
			{
				perror("Get list error!");
				return -1;
			}
			else
			{
				if(0 == strncmp(GET_BUFF, GET_LIST_END,sizeof(GET_LIST_END)))
                //判断服务器是否发送完毕
				{
					break;   //发送完毕，退出
				}
				else
				{
					printf("%s\n", GET_BUFF);   //服务器发送完毕，显示文件
				}
			}
		}
	}
	return getsockfd;
}


/***********把服务器文件列表发送到客户发送到客户端函数*************/
int ftp_putlist(int putsockfd)
{
	const char* LIST_NAME=".";
	char PUT_BUFF[BUFF_LEN];
	int strn, strm;
	DIR* dp;
	struct dirent *ep;
	struct stat st;
	char LIST_PATH[256];




	dp = opendir(LIST_NAME);


	if(NULL != dp)   //麓貌驴陋脛驴脗录鲁脡鹿娄
	{
		while(ep = readdir(dp))   //脩颅禄路露脕脛驴脗录
		{
			if(ep->d_name[0] != '.')   //脠莽鹿没虏禄脢脟脪镁虏脴脦脛录镁禄貌脛驴脗录
			{
				sprintf(PUT_BUFF,"%s",ep->d_name);
				write(putsockfd, PUT_BUFF, BUFF_LEN);
			}

		}


		if(-1 == write(putsockfd, GET_LIST_END, BUFF_LEN))   //路垄脣脥陆谩脢酶
		{
			perror("Write endstring error!");
			return -1;
		}
	}
	else
	{
		if(-1 == write(putsockfd, GET_LIST_END, BUFF_LEN))   //路垄脣脥陆谩脢酶
		{
			perror("Write endstring error!");
			return -1;
		}
		perror("Can't open the directory!");
		return -1;
	}


	closedir(dp);
	return putsockfd;
}


/*************************脦脛录镁陆脫脢脮潞炉脢媒*************************/
int ftp_getfile(int getsockfd, const char* GET_FILENAME)
{
	int getfilefd;   //麓忙路脜陆脫脢脮脦脛录镁碌脛脦脛录镁脙猫脢枚路没
	int getfilesize;   //脢碌录脢陆脫脢脮碌脛脦脛录镁麓贸脨隆
	char GET_BUFF[BUFF_LEN];   //陆脫脢脮禄潞麓忙




	//麓貌驴陋脪禄赂枚脦脛录镁脙猫脢枚路没脫脙脫毛卤拢麓忙脌麓脳脭路垄脣脥露脣碌脛脦脛录镁
	if(-1  == (getfilefd = open(GET_FILENAME, O_WRONLY|O_CREAT|O_TRUNC, 0666)))
	{
		perror("Can't open or creat file!");
		return -1;
	}
	else
	{
		while(getfilesize = read(getsockfd, GET_BUFF, BUFF_LEN) > 0)     //陆脫脢脮脦脛录镁
		{

			if(0 == strncmp(GET_BUFF, "ERROR:", 6))   //陆脫脢脮脦脛录镁鲁枚麓铆
			{
				printf("%s", GET_BUFF);
				return -1;
			}
			else
			{
				//脠隆鲁枚脢媒戮脻掳眉脥路脰脨掳眉潞卢碌脛脢媒戮脻脟酶麓贸脨隆
				memcpy(&getfilesize, GET_BUFF, 4);


				/*GET_BUFF+4脢脟脪貌脦陋脢媒戮脻掳眉脟掳脣脛赂枚脳脰陆脷麓忙路脜碌脛脢脟脢媒戮脻鲁陇露脠拢卢脰庐潞贸碌脛
				1024赂枚脳脰陆脷虏脜麓忙路脜碌脛脢碌录脢碌脛脢媒戮脻*/
				if(-1 == write(getfilefd, GET_BUFF+4, getfilesize))
				{
					perror("Download error!");   //陆脫脢脮鲁枚麓铆拢卢路碌禄脴
					close(getfilefd);   //鹿脴卤脮脦脛录镁
					return -1;
				}
				if(getfilesize < (BUFF_LEN-4))   //脪脩戮颅露脕脠隆碌陆脦脛录镁脛漏脦虏
					break;   //陆脫脢脮陆谩脢酶拢卢脥脣鲁枚
			}
		}
		close(getfilefd);   //鹿脴卤脮脦脛录镁
		return getfilefd;   //陆脫脢脮脥锚鲁脡拢卢路碌禄脴陆脫脢脮碌陆碌脛脦脛录镁碌脛脦脛录镁脙猫脢枚路没隆拢

	}
}


/*****************************脦脛录镁路垄脣脥潞炉脢媒********************************/
int ftp_putfile(int putsockfd, const char* PUT_FILENAME)
{
	int putfilefd;   //麓忙路脜陆脫脢脮脦脛录镁碌脛脦脛录镁脙猫脢枚路没
	int putfilesize;   //脢碌录脢陆脫脢脮碌脛脦脛录镁麓贸脨隆
	char PUT_BUFF[BUFF_LEN];    


	if(-1 == (putfilefd = open(PUT_FILENAME, O_RDONLY)))   //麓貌驴陋脦脛录镁
	{
		perror("Open error!");
		write(putsockfd, E_NOFILE, BUFF_LEN);   //掳脩麓铆脦贸脨脜脧垄脨麓禄脴隆拢
		/*脠莽鹿没虏禄脨麓禄脴麓铆脦贸脨脜脧垄拢卢路垄脣脥露脣禄谩驴篓脣脌*/
		return -1;
	}
	else
	{

		while((putfilesize = read(putfilefd, PUT_BUFF+4, (BUFF_LEN-4))) \
				>0)
		{
			memcpy(PUT_BUFF, &putfilesize, 4);
			if(-1 == write(putsockfd, PUT_BUFF, BUFF_LEN))
			{
				perror("Put file error!");
				close(putfilefd);
				return -1;
			}
			memset(PUT_BUFF, 0, BUFF_LEN);   //脟氓驴脮禄潞鲁氓脟酶
		}
	}
	close(putfilefd);
	return putfilefd;
}

#endif
