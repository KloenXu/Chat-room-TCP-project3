#include "allhead.h"

char recv_msg1[1024] = {0};//接受到的信息
char send_msg1[1024] = {0};//发送的信息
char temppath[1024] = {0};//中转路径,为了避免recv_msg1或者send_msg1在操作中被改变
char member[6][50] = {0};//用于存放在线信息并显示列表的数组
char windowname[10] = {0};//窗口私聊模式中对方的名字
pthread_mutex_t lock1;//互斥锁,用于防止下载和接受信息的两个recv相冲突的情况
int flag = 0;//功能标识符

/*
上传文件函数 - 用于发送文件至服务器
sendpath - 要上传的文件的路径
sockfd - 与服务器相连的套接字
*/
int uploadfile(char *sendpath, int sockfd)
{
	int sendfd, ret;
	sendfd = open(sendpath, O_RDWR);
	if (sendfd == -1)
	{
		perror("打开文件失败\n");
		return -1;
	}
	char sendfile[1024] = {0};
	while (1)
	{
		bzero(sendfile, sizeof(sendfile));
		ret = read(sendfd, sendfile, sizeof(sendfile));
		if (ret == 0)
		{
			break;
		}
		send(sockfd, sendfile, ret, 0);
	}
	close(sendfd);
	return 0;
}

/*
下载文件函数 - 从服务器下载文件
downloadpath - 要下载的文件的路径
sockfd - 与服务器相连的套接字
*/
int downloadfile(char *downloadpath, int sockfd)
{
	int recvfd, ret;
	recvfd = open(downloadpath, O_RDWR | O_CREAT | O_TRUNC);
	if (recvfd == -1)
	{
		recvfd = open(downloadpath, O_RDWR | O_TRUNC);
	}
	char recvfile[1024] = {0};
	while (1)
	{
		bzero(recvfile, sizeof(recvfile));
		ret = recv(sockfd, recvfile, sizeof(recvfile), 0);
		// printf("%d\n", ret);
		write(recvfd, recvfile, ret);
		if (ret < sizeof(recvfile))
		{
			break;
		}
	}
	close(recvfd);
	return 0;
}

/*
接收信息线程
*/
void *myrecvmsg(void *arg)
{
	int sockfd = *((int *)arg);
	int ret;
	char tempmsg[1024] = {0};
	while (1)
	{
		pthread_mutex_unlock(&lock1);
		bzero(recv_msg1, sizeof(recv_msg1));
		bzero(tempmsg, sizeof(tempmsg));
		ret = recv(sockfd, recv_msg1, sizeof(recv_msg1), 0);
		if (ret == 0)
			exit(0);
		printf("%s", recv_msg1);
		if (strcmp(recv_msg1, "lockon\n") == 0)
		{
			sleep(1);
			pthread_mutex_lock(&lock1);
			printf("unlock\n");
			continue;
		}
		if (strcmp(recv_msg1, "accept_file\n") == 0)
		{
			char ask[10] = {0};
			char path[1024] = {0};
			recv(sockfd, path, sizeof(path), 0);
			printf("%s\n", path);
			strcpy(temppath, path);

			char clientpath[2048] = {0};
			sprintf(clientpath, "client2lib/%s", temppath);
			printf("%s\n", clientpath);
			recv(sockfd, ask, sizeof(ask), 0);
			downloadfile(clientpath, sockfd);
			printf("下载完成\n");
			bzero(temppath, sizeof(temppath));
			continue;
		}
		if (strcmp(recv_msg1, "window\n") == 0)
		{
			bzero(windowname,sizeof(windowname));
			recv(sockfd, windowname, sizeof(windowname), 0);
			printf("windowname:%s\n", windowname);
			flag = 2;
		}
		strcpy(tempmsg, recv_msg1);
		if (flag == 1)
		{
			int i, count = 0;
			for (i = 0; i <= strlen(tempmsg) - 1; i++)
			{
				if (tempmsg[i] == '#')
					count++;
			}
			strcpy(member[0], strtok(tempmsg, "#"));
			for (i = 1; i <= count - 1; i++)
			{
				strcpy(member[i], strtok(NULL, "#"));
			}
			for (i = 0; i <= count - 1; i++)
				printf("%s\n", member[i]);
			flag = 0;
		}
	}
}

int main()
{
	int sockfd;
	int ret;
	pthread_mutex_init(&lock1, NULL);
	//定义ipv4地址结构体变量，用来存放你需要绑定的ip和端口号
	struct sockaddr_in bindaddr;
	bzero(&bindaddr, sizeof(bindaddr));
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY宏定义作用:自动匹配本机IP
	bindaddr.sin_port = htons(10002);			  //你要绑定的端口号，我自己指定的

	//定义ipv4地址结构体变量，用来存放对方的ip和端口号
	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.44"); //对方的ip地址
	serveraddr.sin_port = htons(10000);						//对方的端口号

	//创建套接字--》用来通信(套接字类比手机)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		perror("创建套接字失败!\n");
		return -1;
	}

	//设置端口可复用，不用再等半分钟才能再执行
	int again = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &again, sizeof(again));

	//绑定ip和端口号
	ret = bind(sockfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr));
	if (ret == -1)
	{
		perror("绑定失败!\n");
		return -1;
	}

	//拨号
	ret = connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (ret == -1)
	{
		perror("连接失败!\n");
		return -1;
	}

	//多线程进行双向通信
	pthread_t id;
	pthread_create(&id, NULL, myrecvmsg, &sockfd);

	while (1)
	{
		bzero(send_msg1, sizeof(send_msg1));
		setbuf(stdin, NULL);
		fgets(send_msg1, 1024, stdin);
		if (flag == 2)
		{
			char factmsg[2048] = {0};
			sprintf(factmsg, "/window %s %s", windowname, send_msg1);
			send(sockfd, factmsg, strlen(factmsg), 0);
			if (strcmp(send_msg1, "quit\n") == 0)
			{
				flag = 0;
			}
			continue;
		}
		ret = send(sockfd, send_msg1, strlen(send_msg1), 0);
		if (strcmp(send_msg1, "upload\n") == 0)
		{
			char sendpath[512] = {0};
			printf("请输入文件路径\n");
			scanf("%s", sendpath);
			send(sockfd, sendpath, strlen(sendpath), 0);
			sleep(1);
			uploadfile(sendpath, sockfd);
			printf("上传完成\n");
		}
		else if (strcmp(send_msg1, "download\n") == 0)
		{
			char client1path[1024] = {0};
			char recvpath[512] = {0};
			printf("请输入需要下载的文件名\n");
			scanf("%s", recvpath);
			pthread_mutex_lock(&lock1);
			send(sockfd, recvpath, strlen(recvpath), 0);
			sprintf(client1path, "client2lib/%s", recvpath);
			downloadfile(client1path, sockfd);
			printf("下载完成\n");
			pthread_mutex_unlock(&lock1);
		}
		else if (strcmp(send_msg1, "getlist\n") == 0)
		{
			flag = 1;
		}
		else if (strcmp(send_msg1, "sendfile\n") == 0)
		{
			char sendpath[512] = {0};
			printf("请输入文件路径\n");
			scanf("%s", sendpath);
			send(sockfd, sendpath, strlen(sendpath), 0);
			sleep(1);
			uploadfile(sendpath, sockfd);
			printf("上传至服务器完成\n");

			char name[10] = {0};
			printf("请输入要传输给的对象用户名:\n");
			scanf("%s", name);
			send(sockfd, name, strlen(name), 0);
		}
		if (strcmp(send_msg1, "quit\n") == 0)
			break;
		if (ret == 0)
			break;
	}

	close(sockfd);
	return 0;
}