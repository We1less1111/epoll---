#ifndef NETWORKING_H
#define NETWORKING_H
#include "ae.h"
#include "myserver.h"
#define MAX_ACCEPTS_PER_CALL 1000
#define CLIENT_CLOSE_AFTER_REPLY (1<<6) 
#define CONFIG_DEFAULT_MAX_CLIENTS 10000
#define CLIENT_UNIX_SOCKET (1<<11)
#include <stdio.h>
#include <stdlib.h>
#include<time.h>
#include <string.h>
static void freeClient(client *c);
static void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask);
static client *createClient(int fd) ;
static void acceptCommonHandler(int fd, int flags, char *ip);
static void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask); 
static  int writeToClient(int fd, client *c, int handler_installed); 
static int addReplyToBuffer(client *c, const char *s, size_t len) ;
static void processInputBuffer(client *c);

/*获取关键字*/
static char* getKey(char *c)
{
        int i;
        int start = -1;
        int end = -1 ;
		int wordlen = strlen(c);
        for(i=0;i<wordlen;i++)
        {
                if(start == -1 && c[i]>='a' && c[i]<='z')
                        start = i;
                else if(start>=0 && !(c[i]>='a' && c[i]<='z'))
                {
                        end = i - 1;
                        break;
                }
				if(start>=0 && i == wordlen - 1)
                 {   
                end = i;
                 }

        }
        if(start == -1 || end == -1)
                return NULL;
        {
                int len = end - start;
        		char *s = malloc(len+1);
        		s[len]=0;
                memcpy(s,c+start,end-start+1);
                return s;
        }
}


static void freeClient(client *c) {
	if (c->fd != -1) {
        aeDeleteFileEvent(server.el,c->fd,AE_READABLE);
        aeDeleteFileEvent(server.el,c->fd,AE_WRITABLE);
        close(c->fd);
        c->fd = -1;
    }
}

static void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask) {
    client *c = (client*) privdata;
    int nread, readlen;
    size_t qblen;
	
	nread = read(fd, c->querybuf, readlen);
    readlen = 16*1024;
	if(nread <= 0)
	{
		printf("fd=%d close\n\n",fd);
		freeClient(c);
		return;
	}
	c->querybuf_peak = nread;
	c->querybuf[nread] = 0;
    printf("fd:%d--recev<<<%s\n",fd,c->querybuf);
	processInputBuffer(c);
}

static client *createClient(int fd) {
    client *c = zmalloc(sizeof(client));    

    /* passing -1 as fd it is possible to create a non connected client.
     * This is useful since all the commands needs to be executed
     * in the context of a client. When commands are executed in other
     * contexts (for instance a Lua script) we need a non connected client. */
	printf("new client connect,fd=%d\n",fd);
    if (fd != -1) {

        anetNonBlock(NULL,fd);

        anetEnableTcpNoDelay(NULL,fd);

        if (server.tcpkeepalive)

            anetKeepAlive(NULL,fd,server.tcpkeepalive);

        if (aeCreateFileEvent(server.el,fd,AE_READABLE,
            readQueryFromClient, c) == AE_ERR)
        {
            close(fd);
            zfree(c);
            return NULL;
        }
    }
	
    c->id = server.next_client_id++;

    c->fd = fd;
	sprintf(c->name,"name_%d",fd);

    c->bufpos = 0;
  
    c->querybuf_peak = 0;

    c->argc = 0;

    c->argv = NULL;


    c->multibulklen = 0;

    c->bulklen = -1;

    c->sentlen = 0;

    c->flags = 0;
    return c;
}
static void acceptCommonHandler(int fd, int flags, char *ip) {
    client *c;

    if ((c = createClient(fd)) == NULL) {
        printf("Error registering fd event for the new client:  (fd=%d)",fd);
        close(fd); /* May be already closed, just ignore errors */
        return;
    }
	
    /* If the server is running in protected mode (the default) and there
     * is no password set, nor a specific interface is bound, we don't accept
     * requests from non loopback interfaces. Instead we try to explain the
     * user what to do to fix it if needed. */
    server.stat_numconnections++;

    c->flags |= flags;

}

static void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
    int cport, cfd, max = MAX_ACCEPTS_PER_CALL; 
    char cip[46];
	cfd = anetTcpAccept(server.neterr, fd, cip, sizeof(cip), &cport);
	acceptCommonHandler(cfd,0,cip);
	
}
static  int writeToClient(int fd, client *c, int handler_installed) {
    ssize_t nwritten = 0, totwritten = 0;
    size_t objlen;
    size_t objmem;

    while(c->bufpos) {
        if (c->bufpos > 0) {
            nwritten = write(fd,c->buf+c->sentlen,c->bufpos-c->sentlen);
            if (nwritten <= 0) break;
            c->sentlen += nwritten;
            totwritten += nwritten;

            if ((int)c->sentlen == c->bufpos) {

                c->bufpos = 0;
                c->sentlen = 0;
            }
    }

    if (nwritten == -1) {
        if (errno == EAGAIN) {
            nwritten = 0;
        } else {
            freeClient(c);
            return C_ERR;
        }
    }

    if (!(c->bufpos)) {
        c->sentlen = 0;
        if (handler_installed) aeDeleteFileEvent(server.el,c->fd,AE_WRITABLE);
        if (c->flags & CLIENT_CLOSE_AFTER_REPLY) {
            freeClient(c);
            return C_ERR;
        }
		}
	}
    return C_OK;
}
static int addReplyToBuffer(client *c, const char *s, size_t len) {
    size_t available = sizeof(c->buf)-c->bufpos;
    if (c->flags & CLIENT_CLOSE_AFTER_REPLY) return C_OK;

    if (len > available) return C_ERR;
    memcpy(c->buf+c->bufpos,s,len);
    c->bufpos += len;
    return C_OK;
}
static char *setProtocol(char *s)
{
	int len = strlen(s);
	char *result = malloc(len+8);
	sprintf(result,"$%d\r\n%s\r\n",len,s);
	return result;
}
static void  writerToGroups(client *c)
{
	int i;
	char temp[1024];
	int tlength;
	for(i=6;i<=server.el->maxfd;i++)
	{
		if(c->fd != i)
		{
			aeFileEvent *fe = &(server.el->events[i]);
			client *c2 = (void*)(fe->clientData);
			tlength = strlen(c->querybuf) + strlen(c->name)+7;
			sprintf(temp,"【%s】:%s",c->name,c->querybuf);
			temp[tlength] = 0;
			addReplyToBuffer(c2,temp,tlength);
			writeToClient(i,c2,0);
		}
	}
}
static void  writerToOne(client *c,char *name,char *word)
{
	int i;
	char temp[1024];
	int tlength;
	for(i=6;i<=server.el->maxfd;i++)
	{
		aeFileEvent *fe = &(server.el->events[i]);
		client *c2 = (void*)(fe->clientData);
		if(strcmp(name,c2->name)==0)
		{
			tlength = strlen(word) + strlen(c->name)+7;
			sprintf(temp,"【%s】:%s",c->name,word);
			temp[tlength] = 0;
			addReplyToBuffer(c2,temp,tlength);
			writeToClient(i,c2,0);
			break;
		}
	}
}
static void processInputBuffer(client *c) {
	char *name = NULL;
	char *word = NULL;
	char *key = getKey(c->querybuf);
	if(key!= NULL && strcmp(key,"setname") == 0)
	{
		name = getKey(c->querybuf + strlen(key) + 1);
		strcpy(c->name,name);
	}
	else if(key!= NULL && strcmp(key,"pchat") == 0)
	{
		name = getKey(c->querybuf + strlen(key) + 1);
		writerToOne(c,name,c->querybuf + strlen(key) + strlen(name) + 2);
	}
	else
		writerToGroups(c);
}
#endif
