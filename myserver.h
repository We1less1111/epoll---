#ifndef __MYSERVER_H__
#define __MYSERVER_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <syslog.h>
#include <netinet/in.h>
#include <signal.h>
#include "zmalloc.h"
#define PROTO_REPLY_CHUNK_BYTES (16*1024) /* 16k output buffer */
#define C_OK                    0
#define C_ERR                   -1


typedef char* sds;

typedef struct client {

    uint64_t id;            /* Client incremental unique ID. */

    int fd;                 /* Client socket. */
           /* Pointer to currently SELECTed DB. */

    int dictid;             /* ID of the currently SELECTed DB. */

    char name[20];             /* As set by CLIENT SETNAME. */


    size_t querybuf_peak;   /* Recent (100ms or more) peak of querybuf size. */

    int argc;               /* Num of arguments of current command. */

    char **argv;            /* Arguments of current command. */

    struct redisCommand *cmd, *lastcmd;  /* Last command executed. */


    int multibulklen;       /* Number of multi bulk arguments left to read. */

    long bulklen;           /* Length of bulk argument in multi bulk request. */
    size_t sentlen;         /* Amount of bytes already sent in the current
                               buffer or object being sent. */
    int flags;              /* Client flags: CLIENT_* macros. */

    
    /* Response buffer */

    int bufpos;
	char querybuf[PROTO_REPLY_CHUNK_BYTES];
    char buf[PROTO_REPLY_CHUNK_BYTES];
} client;
#include "ae.h"
#include "anet.h"
#include "zmalloc.h"
struct redisServer{
	aeEventLoop *el;
	int ipfd[16];
	int port; 
	int bindaddr_count;
	int ipfd_count;             /* Used slots in ipfd[] */
    int sofd; 
	int tcp_backlog;
	char neterr[256]; 
	int stat_rejected_conn;
	int protected_mode;
	int stat_numconnections;
	int tcpkeepalive;
	int next_client_id;
	char *unixsocket; 
};
struct redisServer server; 

#endif