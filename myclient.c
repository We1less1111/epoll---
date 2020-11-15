#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include<string.h>
#include<stdio.h>
#define MAX_EVENT_NUM           1024
#define BUFFER_SIZE             10
#define true                    1
#define false                   0
#define formstring "user@myclient<<"
char ip[20] = "192.168.112.150";
char port[4] = "6666";
int login = 0;
int connfd;
char querybuf[1024];
char writebuf[1024];
static char *setProtocol(char *s)
{
	int len = strlen(s);
	char *result = malloc(len+8);
	sprintf(result,"$%d\r\n%s\r\n",len,s);
	return result;
}
int checkRule(int argc)
{
	if(argc>2)
		return 0;
	return 1;
}
void initSocket()
{
	struct sockaddr_in serveraddr;
	

	connfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("connfd=%d\n",connfd);
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(port));
	inet_pton(AF_INET, ip, &serveraddr.sin_addr);
	int flag = connect(connfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if(flag<0){
		printf("connect error\n");
		return ;
	}
	printf("connect right\n");
}
void responseClient(char data[])
{
	printf("%s%s",formstring,data);
	fflush(stdout);
}
void *do_fgets(void* tt)
{   
	while(fgets(writebuf, 1024, stdin) != NULL)
	{
		writebuf[strlen(writebuf)-1] = 0;
		write(connfd, writebuf, strlen(writebuf));
		responseClient("");
		
	}
	close(connfd);
}   
int main(int argc, char *argv[])
{
	initSocket();
	responseClient("welcome your login!\n");
	responseClient("");
	pthread_t    tid ;
	int ret ;
	ret = pthread_create(&tid , NULL , do_fgets , NULL);
	if(ret != 0)
	{
			fprintf(stderr , "创建线程失败!\n");
			return -1 ;
	}
	ret = pthread_detach(tid);
	while(read(connfd, querybuf, 1024)){
		printf("\n%s\n",querybuf);
		responseClient("");
		memset(querybuf,0,sizeof(querybuf));
		}
	close(connfd);
	return 0;
}



