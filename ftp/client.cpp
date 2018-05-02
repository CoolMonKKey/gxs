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
#include "common.h"
#include <dirent.h>


#define INET_NUM 20021   //port
#define IP_ADDRESS "192.168.122.1"   //ip


#ifndef BUFF_LEN
#define BUFF_LEN 1028
#endif


#define CMD_LS         11//列出客户端所有文件
#define CMD_SERVER_LS  22//列出服务器所有文件
#define CMD_DOWNLOAD   33//下载文件
#define CMD_UPLOAD     44//上传文件
#define CMD_QUIT       55//退出
#define CMD_ERROR      -1//错误




int main(void)
{
	int sockfd;
	DIR* dp;
	struct dirent *ep;
	sockfd = network_init(NET_CLIENT, IP_ADDRESS, INET_NUM);   //初始化网络连接
	if(sockfd == -1)
	{
		perror("Network init error!");
		exit(EXIT_FAILURE);
	}
	printf("Connect success!\n\n");
	printf("*********Operation Help*********\n");   //操作帮助
	printf("显示本地文件列表:        ls\n");
	printf("显示服务器文件列表:      server ls\n");
	printf("实现xxx文件读取与上传:   upload xxx\n");
	printf("实现xxx文件下载与存储:   download xxx\n");
	printf("断开socket链接:          quit\n");
	printf("*************************************\n\n");


	char SEND_BUFF[BUFF_LEN];   //发送数据缓冲区
	int cmd_result;   //存放命令解析结果
	while(1)
	{
		fgets(SEND_BUFF, sizeof(SEND_BUFF), stdin);   //输入命令
		SEND_BUFF[strlen(SEND_BUFF)-1] = '\0';   //去掉最后输入的回车符

		cmd_result = ftp_cmd_analyse(SEND_BUFF);   //解析输入的指令
		switch(cmd_result)
		{
		case CMD_ERROR:
			printf("ERROR!\n");
			break;

			////////////////////////////////////////////////////////////
		case CMD_LS:    //列出本地文件列表
			dp=opendir (".");
			printf("*********File List of Client*********\n");
			while((ep = readdir(dp)))
			{
				if(ep->d_name[0] != '.')
				{
					printf("%s\n",ep->d_name);
				}
			}
			printf("*************************************\n");  
			printf("List file success!\n");      
			closedir(dp);
			break;
			///////////////////////////////////////////////////////////
		case CMD_SERVER_LS:   //列出服务器端可下载的所有文件
			printf("*********File List of Server*********\n");
			if(ftp_getlist(sockfd) == -1)
			{
				printf("List file error!\n");
			}
			else
			{          
				printf("*************************************\n");
				printf("List file success!\n");
			}
			break;

			////////////////////////////////////////////////////////////
		case CMD_DOWNLOAD:   //从服务器下载文件
			if(write(sockfd, SEND_BUFF, BUFF_LEN) == -1)
			{
				perror("Send cmd error!");
				break;
			}
			if(ftp_getfile(sockfd, SEND_BUFF+9) == -1)   //下载文件
			{
				printf("Download error!\n");
			}
			else
			{
				printf("Download The File Success!!\n");
			}
			break;
			////////////////////////////////////////////////////////////
		case CMD_UPLOAD:   //上传文件到服务器   

			if(write(sockfd, SEND_BUFF, BUFF_LEN) == -1)
			{
				perror("Send cmd error!");
				break;
			}
			if(ftp_putfile(sockfd, SEND_BUFF+7) == -1)//上传文件
			{
				printf("Upload error!\n");
			}
			else
			{
				printf("Upload The File Success!!\n");
			}
			break; 
			////////////////////////////////////////////////////////////
		case CMD_QUIT:   //断开连接
			printf("Welcome to use again!\nQUIT!\n");
			close(sockfd);//客户端关闭文件描述符后就会自动断开连接
			exit(EXIT_SUCCESS);
			break;
			////////////////////////////////////////////////////////////
		default:
			break;
		}
	}
	close(sockfd);//客户端关闭文件描述符后就会自动断开连接
	return 0;
}
