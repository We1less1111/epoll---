#include <sys/resource.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <locale.h>
#include <sys/socket.h>
#include "myserver.h"
#include "ae.h"
#include "anet.h"
#include "networking.c"
#include<stdio.h>
/*
int listenToPort(int port, int *fds, int *count) {
    fds[*count] = anetTcp6Server(server.neterr,port,NULL,
                server.tcp_backlog);
	if (fds[*count] != ANET_ERR) {
                anetNonBlock(NULL,fds[*count]);
                (*count)++;
    return C_OK;
	}
}*/

int listenToPort(int port, int *fds, int *count) {
	fds[*count] = anetTcp6Server(server.neterr,port,NULL,server.tcp_backlog);
	anetNonBlock(NULL,fds[*count]);
	(*count)++;
	fds[*count] = anetTcpServer(server.neterr,port,NULL,server.tcp_backlog);
	anetNonBlock(NULL,fds[*count]);
	(*count)++;
    return C_OK;
}
void initServerConfig(void) {
	server.port = 6666;
    server.bindaddr_count = 0;
    server.ipfd_count = 0;
    server.sofd = -1;
	server.tcp_backlog = 511;
	server.stat_rejected_conn = 0;
	server.protected_mode = 1;
	server.stat_numconnections = 0;
	server.tcpkeepalive = 300;
	server.next_client_id = 0;
	server.unixsocket = NULL;
}
void initServer(void) {
	int j;
	server.el = aeCreateEventLoop(20000);
	listenToPort(server.port,server.ipfd,&server.ipfd_count);
	for (j = 0; j < server.ipfd_count; j++) {
        aeCreateFileEvent(server.el, server.ipfd[j], AE_READABLE,acceptTcpHandler,NULL);
    }
}
void aeMain(aeEventLoop *eventLoop) {
    eventLoop->stop = 0;
	printf("bef\n");
    while (!eventLoop->stop) {
        aeProcessEvents(eventLoop, AE_ALL_EVENTS);
    }
}
int main()
{
	initServerConfig();
	initServer();
	aeMain(server.el);
	return 0;
}
