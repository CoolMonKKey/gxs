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


#define INET_NUM 20021   //端口号
#define IP_ADDRESS "192.168.122.1"   //IP地址


#define BUFF_LEN 1028


#define CMD_LS          11   //列出客户端所有文件
#define CMD_SERVER_LS   22   //列出服务器所有文件
#define CMD_DOWNLOAD    33   //下载文件
#define CMD_UPLOAD      44   //上传文件
#define CMD_QUIT        55   //退出
#define CMD_ERROR       -1   //错误




int main(void)
{
	int sockfd;   //网络套接字
	sockfd = network_init(NET_SERVER, IP_ADDRESS, INET_NUM);   //初始化网络连接
	if(sockfd == -1)
	{
		perror("Network init error!");
		exit(EXIT_FAILURE);
	}
	printf("LISTEN-ing...\n");


	char RCV_BUFF[BUFF_LEN];   //接收缓存
	char SEND_BUFF[BUFF_LEN];  //发送缓存
	int cmd_result;   //命令解析结果
	int connectfd;   //建立连接后用于通信的套接字文件
	int readlen;   //读取到的字节数
	while(1)
	{
		if((connectfd = accept(sockfd, NULL, NULL)) == -1)   //链接出错，给出提示
		{
			perror("Connect error!\n");
			break;
		}
		printf("Connect success!\n");
		while(1)
		{
			readlen = read(connectfd, RCV_BUFF, sizeof(RCV_BUFF));   //接收命令
			if(readlen <0)//接收出错
			{
				perror("Read error!\n");
				break;
			}
			else
			{
				if(readlen == 0)   //客户端关闭文件描述符后就会断开连接
				{
					printf("Welcome to use again!\nQUIT...\n");
					break;
				}
				else
				{
					printf("**************************\n");
					printf("RECV:%s\n",RCV_BUFF);
					cmd_result = ftp_cmd_analyse(RCV_BUFF);   //解析命令
					switch(cmd_result)
					{
					case CMD_ERROR:
						printf("CMD_ERROR!\n");
						break;
					case CMD_SERVER_LS:  //查看服务器文件列表
						if(ftp_putlist(connectfd) == -1)
						{
							printf("List files error!\n");
						}
						else
						{
							printf("List files success!\n");
						}
						break;
					case CMD_DOWNLOAD:   //客户端从服务器下载文件
						printf("Put files:%s\n", RCV_BUFF+9);
						if(ftp_putfile(connectfd, RCV_BUFF+9) == -1)
						{
							printf("Put files error!\n");
						}
						else
						{
							printf("Put files success!\n");
						}
						break;
					case CMD_UPLOAD:   //客户端上传文件到服务器
						printf("Get files:%s\n", RCV_BUFF+7);
						if(ftp_getfile(connectfd, RCV_BUFF+7) == -1)
						{


							printf("Get files error!\n");
						}
						else
						{
							printf("Get files success!\n");
						}
						break;
					default:
						break;
					}
				}
			}
		}
		close(connectfd);//客户端退出，断开连接
	}
	close(sockfd);
	return 0;
}
