#ifndef SOCK_H_
#define SOCK_H_

#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>


#define PROTO_UNKN -1
#define PROTO_UDP  1
#define PROTO_TCP  2
#define URLMAP_SIZE 128

#define STS_SUCCESS	0	/* SUCCESS				*/
#define STS_TRUE	0	/* TRUE					*/
#define STS_FAILURE	1	/* FAILURE				*/
#define STS_FALSE	1	/* FALSE				*/
#define STS_NEED_AUTH	1001	/* need authentication			*/
#define STS_SIP_SENT	2001	/* SIP packet is already sent, end of dialog */
#define BUFFER_SIZE     8196

#ifdef __cplusplus
extern "C" {
#endif

int sipsock_listen(void);						
int sipsock_waitfordata(char *buf, size_t bufsize, struct sockaddr_in *from, int *protocol);
int sipsock_send(struct in_addr addr, int port,	int protocol, char *buffer, size_t size);
int sockbind(struct in_addr ipaddr, int localport, int protocol, int errflg);
int tcp_find(struct sockaddr_in dst_addr);
int get_tcp_fd(struct sockaddr_in dst_addr);

#ifdef __cplusplus
}
#endif


#endif
