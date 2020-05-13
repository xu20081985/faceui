#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sock.h"
#include "wrt_common.h"

const int sip_listen_port = 57220;
const int tcp_connect_timeout = 5000;
static const int TCP_TIMEOUT = 45;
static const int TCP_KEEPALIVE = 10;
int sip_udp_socket = 0;/* UDP socket used for SIP datagrams */
int sip_tcp_socket = 0;/* TCP listen socket used for SIP */

/* TCP sockets used for SIP connections (twice the max number of clients) */
struct {
	int fd;						    /* file descriptor, 0=unused */
	struct sockaddr_in dst_addr;	/* remote target of TCP connection */
	time_t traffic_ts;			    /* last 'alive' TS (real SIP traffic) */
	time_t keepalive_ts;			/* last 'alive' TS */
	int    rxbuf_size;
	int    rxbuf_len;
	char   *rx_buffer;
} sip_tcp_cache[2 * URLMAP_SIZE] = {0};


extern int errno;

/* static functions */
static void tcp_expire(void);
static int tcp_add(struct sockaddr_in addr, int fd);
static int tcp_connect(struct sockaddr_in dst_addr);
static int tcp_remove(int idx);


static char *utils_inet_ntoa(struct in_addr in)
{
	return inet_ntoa(in);
}

/*
 * binds to SIP UDP and TCP sockets for listening to incoming packets
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sipsock_listen(void)
{
	struct in_addr ipaddr;

	/* listen on UDP port */
	memset(&ipaddr, 0, sizeof(ipaddr));
	sip_udp_socket = sockbind(ipaddr, sip_listen_port, PROTO_UDP, 1);
	if (sip_udp_socket == 0) 
		return STS_FAILURE;

	/* listen on TCP port */
	memset(&ipaddr, 0, sizeof(ipaddr));
	sip_tcp_socket = sockbind(ipaddr, sip_listen_port, PROTO_TCP, 1);
	if (sip_tcp_socket == 0)
		return STS_FAILURE;

	/* listen max num */
	if (listen(sip_tcp_socket, 10))
	{
		DEBUG_ERROR("errno = %d\n", errno);
		return STS_FAILURE;
	}

	/* initialize the TCP connection cache array */
	memset(&sip_tcp_cache, 0, sizeof(sip_tcp_cache));

	return STS_SUCCESS;
}


/*
 * read a message from SIP listen socket (UDP datagram)
 *
 * RETURNS number of bytes read (=0 if nothing read, <0 timeout)
 *         from is modified to return the sockaddr_in of the sender
 */
int sipsock_waitfordata(char *buf, size_t bufsize, struct sockaddr_in *from, int *protocol)
{
	int i, fd;
	fd_set fdset;
	int highest_fd, num_fd_active;
	static struct timeval timeout={0,0};
	int length;
	socklen_t fromlen;

	if ((timeout.tv_sec == 0) && (timeout.tv_usec == 0))
	{
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
	}

	/* prepare FD set: UDP, TCP listen */
	/*	
	FD_ZERO(&fdset);
	FD_SET (sip_udp_socket, &fdset);
	FD_SET (sip_tcp_socket, &fdset);
	if (sip_udp_socket > sip_tcp_socket)
		highest_fd = sip_udp_socket;
	else
		highest_fd = sip_tcp_socket;
	*/
	
	FD_ZERO(&fdset);
	FD_SET (sip_tcp_socket, &fdset);
	highest_fd = sip_tcp_socket;

	/* prepare FD set: TCP connections */
	for (i = 0; i < (sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++)
	{
		/* active TCP conenction */
		if (sip_tcp_cache[i].fd)
		{
			/* add to FD set */
			FD_SET(sip_tcp_cache[i].fd, &fdset);
			if (sip_tcp_cache[i].fd > highest_fd)
				highest_fd = sip_tcp_cache[i].fd;
		} 
	}

	/* select() on all FD's with timeout */
	num_fd_active = select (highest_fd + 1, &fdset, NULL, NULL, &timeout);

	if (num_fd_active < 0)
	{
		DEBUG_MESSAGE("errno = %d\n", errno);
	}

	/* nothing here = timeout condition */
	if (num_fd_active == 0)
	{
		/* process the active TCP connection list - expire old entries */
		tcp_expire();
		return -1;
	}

	/*
	* Some FD's have signalled that data is available (fdset)
	* Process them:
	* - UDP socket: read data and return
	* - TCP listen socket: Accept connection, update TCP cache and return
	* - TCP connection socket: read data, update alive timestamp & return
	*   In case of disconnected socket (recv error [all but EAGAIN, EINTR])
	*   close connection
	*/

	/* Strategy to get get data from the FD's:
	*  1) check TCP listen socket, if connection pending ACCEPT
	*  2) check UDP socket. If data available, process that & return
	*  3) check TCP sockets, take first in table with data & return
	*/

	/*
	* Check TCP listen socket
	*/
	if (FD_ISSET(sip_tcp_socket, &fdset))
	{
		fromlen = sizeof(struct sockaddr_in);
		if ((fd = accept(sip_tcp_socket, (struct sockaddr *)from, &fromlen)) < 0)
		{
			DEBUG_MESSAGE("accept() fail errno = %d\n", errno);
			return 0;
		}

		if ((i = tcp_add(*from, fd)) < 0)
		{
			DEBUG_ERROR("out of space in TCP connection cache - tcp_add fail\n");
			close(fd);	 
			return 0;
		}
		DEBUG_MESSAGE("TCP connection add ok from:[%s]\n", utils_inet_ntoa(from->sin_addr), fd);

		num_fd_active--;
		if (num_fd_active <= 0)
			return 0;
	}
	/* Check active TCP sockets */
	for (i = 0; i < (sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++)
	{
		if (sip_tcp_cache[i].fd == 0)
			continue;

		/* no more active FD's to be expected, exit the loop */
		if (num_fd_active <= 0)
			break;

		if (FD_ISSET(sip_tcp_cache[i].fd, &fdset))
		{
			num_fd_active--;
			*protocol = PROTO_TCP;
			memcpy(from, &sip_tcp_cache[i].dst_addr, sizeof(struct sockaddr_in));

			length = recv(sip_tcp_cache[i].fd, buf, bufsize, 0);	 
			if (length < 0)
			{
				DEBUG_ERROR("tcp recv date error: %d, TCP connection remove:[%s]\n", errno, utils_inet_ntoa(from->sin_addr));
				length = 0;
				tcp_remove(i);
				continue;
			}
			else if (length == 0)
			{
				/* length=0 indicates a disconnect from remote side */
				DEBUG_WARNING("received TCP disconnect, TCP connection remove:[%s]\n", utils_inet_ntoa(from->sin_addr));
				tcp_remove(i);
				continue;
			}
			else if (length > 0)
			{
				time(&sip_tcp_cache[i].traffic_ts);
				sip_tcp_cache[i].keepalive_ts = sip_tcp_cache[i].traffic_ts;
			}

			/* prematurely check for <CR><LF> keepalives, no need to do any
			work on them... Set length = 0 and done. */
			if (length == 2 && (memcmp(buf, "\x0d\x0a", 2) == 0))
				return -2;
			
			return length;
		}

	}
	
	return 0;
}

/*
 * sends an SIP datagram (UDP or TCP) to the specified destination
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sipsock_send(struct in_addr addr, int port, int protocol, char *buffer, size_t size)
{
	struct sockaddr_in dst_addr;
	int sts;
	int i;

	/* first time: allocate a socket for sending */
	if (sip_tcp_socket == 0 || sip_udp_socket == 0)
	{
		DEBUG_ERROR("socket not allocated fail\n");
		return STS_FAILURE;
	}

	if (buffer == NULL)
	{
		DEBUG_ERROR("sock_send got NULL buffer\n");
		return STS_FAILURE;
	}

	if (protocol == PROTO_UDP)
	{
		/* UDP target */
		dst_addr.sin_family = AF_INET;
		memcpy(&dst_addr.sin_addr, &addr, sizeof(struct in_addr));
		dst_addr.sin_port= htons(port);
		sts = sendto(sip_udp_socket, buffer, size, 0, (const struct sockaddr *)&dst_addr, (int)sizeof(dst_addr));
		if (sts == -1)
		{
			DEBUG_ERROR("sendto() data fail errno: %d\n", errno);
			return STS_FAILURE;	
		}
	} 
	else if (protocol == PROTO_TCP)
	{
		/* TCP target */
		dst_addr.sin_family = AF_INET;
		memcpy(&dst_addr.sin_addr, &addr, sizeof(struct in_addr));
		dst_addr.sin_port = htons(port);
		/* check connection cache for an existing TCP connection */
		i = tcp_find(dst_addr);
		if (i < 0)
			return STS_FAILURE;
		
		/* send data and update alive timestamp */
		time(&sip_tcp_cache[i].traffic_ts);
		sip_tcp_cache[i].keepalive_ts = sip_tcp_cache[i].traffic_ts;

		sts = send(sip_tcp_cache[i].fd, buffer, size, 0);
		if (sts == -1)
		{
			DEBUG_ERROR("send() [%s:%i] send len:%d call failed errno: %d\n",
			utils_inet_ntoa(addr), port, (long)size, errno);
			return STS_FAILURE;
		}

	}
	else
	{
		DEBUG_ERROR("only UDP and TCP supported by now, send fail\n");
		return STS_FAILURE;
	}

	return STS_SUCCESS;
}

/*
 * generic routine to allocate and bind a socket to a specified
 * local address and port (UDP)
 * errflg !=0 log errors, ==0 don't
 *
 * RETURNS socket number on success, zero on failure
 */
int sockbind(struct in_addr ipaddr, int localport, int protocol, int errflg)
{
	struct sockaddr_in my_addr;
	int on = 1;
	int sock;
	u_long set_sock = 1;

	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(localport);
	memcpy(&my_addr.sin_addr, &ipaddr, sizeof(struct in_addr));

	if (protocol == PROTO_UDP)
	{
		sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	else if (protocol == PROTO_TCP)
	{
		sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else
	{
		if (errflg)
			DEBUG_ERROR("invalig protocol: %i\n", protocol);
		return 0;
	}
	
	if (sock < 0)
	{
		DEBUG_ERROR("errno = %d\n", errno);
		return 0;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on , sizeof(on)) < 0)
	{
		if (errflg)
			DEBUG_ERROR("setsockopt returned error [%d]\n", errno);
		close(sock);
		return 0;
	}

	if (bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) != 0)
	{
		if (errflg)
			DEBUG_ERROR("bind failed: %d\n", errno);
		close(sock);
		return 0;
	}

	/* set_sock = 1 no block, = 0 block */
	ioctl(sock, FIONBIO, &set_sock);

	return sock;
}


/*
 * age and expire TCP connections
 *
 * RETURNS: -
 */
static void tcp_expire(void)
{
	int i;
	time_t now = 0;
	time_t to_limit = 0;
	
	time(&now);
	to_limit = now - TCP_TIMEOUT;

	for (i = 0; i < (sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++)
	{
		if (sip_tcp_cache[i].fd == 0) 
			continue;

		if (sip_tcp_cache[i].traffic_ts < to_limit)
		{
			/* TCP has expired, close & cleanup */
			DEBUG_MESSAGE("TCP connection disconnected, tcp_remove: [%s]\n",
			utils_inet_ntoa((&sip_tcp_cache[i].dst_addr)->sin_addr));		
			tcp_remove(i);
		}
		else if ((sip_tcp_cache[i].keepalive_ts + TCP_KEEPALIVE) <= now)
		{
			/* TCP keepalive handling */
			sip_tcp_cache[i].keepalive_ts = now;

			if (send(sip_tcp_cache[i].fd, "\x0d\x0a", 2, 0) == -1)
			{
				DEBUG_ERROR("send keepalive errno = %d\n", errno);
			}
			//DEBUG_MESSAGE("TCP keepalive heartbeat to ip:[%s]\n",utils_inet_ntoa((&sip_tcp_cache[i].dst_addr)->sin_addr));
		}
	}
}

/*
 * find a TCP connection in cache
 *
 * RETURNS: index into TCP cache or -1 on not found
 */
int tcp_find(struct sockaddr_in dst_addr)
{
	int i;

	/* check connection cache for an existing TCP connection */
	for (i = 0; i < (sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++)
	{
		if (sip_tcp_cache[i].fd == 0)
			continue;

		/* address & port compare */
		if ((memcmp(&dst_addr.sin_addr, &sip_tcp_cache[i].dst_addr.sin_addr, sizeof(struct in_addr)) == 0) && 
			 		(dst_addr.sin_port == sip_tcp_cache[i].dst_addr.sin_port))
			break;
	} 

	/* if no TCP connection found return -1 */
	if (i >= (sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])))
		return -1;

	return i;
}

int get_tcp_fd(struct sockaddr_in dst_addr)
{
	int index = -1;

	index = tcp_find(dst_addr);
	if (-1 == index)
		return -1;
	else 
		return sip_tcp_cache[index].fd;
}

/*
 * add a TCP connection into cache
 *
 * RETURNS: index into TCP cache or -1 on failure (out of space)
 */
static int tcp_add(struct sockaddr_in addr, int fd)
{
	int i;

	/* find free entry in TCP cache */
	for (i = 0; i < (sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])); i++)
	{
		if (sip_tcp_cache[i].fd == 0)
			break;
	}
	
	if (i >= (sizeof(sip_tcp_cache)/sizeof(sip_tcp_cache[0])))
		return -1;

	/* store connection data in TCP cache */
	sip_tcp_cache[i].fd = fd;
	memcpy(&sip_tcp_cache[i].dst_addr, &addr, sizeof(struct sockaddr_in));
	time(&sip_tcp_cache[i].traffic_ts);

	sip_tcp_cache[i].keepalive_ts = sip_tcp_cache[i].traffic_ts;

	//DEBUG_MESSAGE("TCP connection tcp_add:[%s]\n", utils_inet_ntoa(addr.sin_addr));

	return i;
}

/*
 * connect to a remote TCP target
 *
 * RETURNS: index into TCP cache or -1 on failure
 */
static int tcp_connect(struct sockaddr_in dst_addr)
{
	int sock;
	int sts;
	int i;
	struct timeval timeout={0, 0};
	fd_set fdset;
	socklen_t optlen;
	u_long set_sock = 1;
	
	/* get socket and connect to remote site */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		DEBUG_ERROR("socket creat failed errno: %d\n", errno);
		return -1;
	}
	
	/* non blocking */
	ioctl(sock, FIONBIO, &set_sock);

	sts = connect(sock, (struct sockaddr *)&dst_addr, sizeof(struct sockaddr_in));
	if ((sts == -1) && (errno == EINPROGRESS))
	{
		/* if non-blocking connect(), wait until connection successful, discarded or timeout */
		DEBUG_MESSAGE("connection in progress, waiting %i msec to succeed\n", tcp_connect_timeout);

		/* timeout for connect */
		timeout.tv_sec  = (tcp_connect_timeout/1000);
		timeout.tv_usec = (tcp_connect_timeout%1000)*1000;

		do {
			/* prepare fd set */
			FD_ZERO(&fdset);
			FD_SET(sock, &fdset);
			sts = select(sock + 1, NULL, &fdset, NULL, &timeout);
			if ((sts < 0) && (errno == EINTR))
			{
				/* select() has been interrupted, do it again */
				continue;
			}
			else if (sts < 0)
			{
				DEBUG_ERROR("TCP connect failed errno: %d\n", errno);
				close(sock);
				return -1;
			}
			else if (sts > 0)
			{
				int valopt;
				optlen = sizeof(valopt);

				/* get error status from delayed connect() */
				if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&valopt, &optlen) < 0)
				{
					DEBUG_ERROR("getsockopt SOL_SOCKET failed errno: %d\n", errno);
					close(sock);
					return -1;
				}

				if (valopt == EINPROGRESS)
				{
					DEBUG_ERROR("connect() returned errno: %d\n", errno);
					continue;
				}

				/* check the returned error value from connect() */
				if (valopt)
				{
					DEBUG_ERROR("delayed TCP connect() failed errno: %d\n", errno);
					close(sock);
					return -1;
				}
				/* all went fine, continue */
				break;
			} 
			else
			{
				DEBUG_ERROR("tcp_connect() timeout\n");
				close(sock);
				return -1;
			}
		} while (1);
	}
	else if (sts == -1)
	{
		DEBUG_ERROR("connect() [%s:%i] call failed errno: %d\n",
		utils_inet_ntoa(dst_addr.sin_addr),ntohs(dst_addr.sin_port), errno);
		close(sock);
		return -1;
	}

	if (tcp_add(dst_addr, sock) < 0)
	{
		DEBUG_ERROR("out of space in TCP connection cache - tcp_add fail\n");
		close(sock);
		return -1;
	}

	DEBUG_MESSAGE("TCP connection connected to:[%s:%i]\n", utils_inet_ntoa(dst_addr.sin_addr), ntohs(dst_addr.sin_port));

	return i;
}


/*
 * clean up resources occupied by a TCP entry
 *
 * RETURNS: 0
 */
static int tcp_remove(int idx)
{
	close(sip_tcp_cache[idx].fd);
	sip_tcp_cache[idx].fd = 0;
	if (NULL != sip_tcp_cache[idx].rx_buffer)
		free(sip_tcp_cache[idx].rx_buffer);	
	sip_tcp_cache[idx].rx_buffer = NULL;
	sip_tcp_cache[idx].rxbuf_size = 0;
	sip_tcp_cache[idx].rxbuf_len = 0;
	
	return 0;
}
