/*
 * wrt_ntp.cpp -- Gateway Master Program Processing
 *
 * Copyright (c) Wrt Intelligent Technology Co Ltd. 2017. All Rights Reserved.
 *
 * See the Project file for usage and redistribution requirements
 *
 *	$Id: wrt_ntp.cpp 	2017/07/05   Siny $
 */
 
/******************************** Description *********************************/
 
/*
 *  Through the NTP protocol, the module sends and transmits data frames 
 *  by itself and sends it to the NTP server through UDP packets, and analyzes 
 *  the time information returned by the NTP server to realize the time service.
 */
 
/********************************* Includes ***********************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>
#include <pthread.h>
#include <netdb.h>
#include <stddef.h>
#include <fcntl.h>  
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include "wrt_gpio.h"
#include "wrt_ntp.h"

/********************************* Defines ************************************/

long64 firsttimestamp, finaltimestamp;

/*********************************** Code *************************************/
/*
 *  The encapsulated data frame is sent to the time server 
 */
int sendpkt(int sockfd,struct addrinfo * res)
{
	struct ntp_packet ntppack,newpack;
	//put the date into the ntppack	
	ntppack.li_vn_mode = 0x23;
	ntppack.stratum = 0x02;
	ntppack.poll = 0x04;
	ntppack.precision = 0xec;
	ntppack.root_delay = htonl(1<<16);//root_delay = 1.0sec
	ntppack.root_dispersion = htonl(1<<8);//root_dispersion = 0.0039sec
	ntppack.ref_id = inet_addr(NTP_SERVER_IP);

	firsttimestamp=JAN_1970+time(NULL);//-8*3600 Gets the initial timestamp T1
	ntppack.reftimestamphigh = htonl(firsttimestamp);
	ntppack.oritimestamphigh = htonl(firsttimestamp);
	ntppack.recvtimestamphigh = htonl(firsttimestamp);
	ntppack.trantimestamphigh= htonl(firsttimestamp);

	int i;
	for (i = 0; i < 1; i++)
	{
		int ret = sendto(sockfd,&ntppack,sizeof(ntppack),0, res->ai_addr, res->ai_addrlen);
		if (ret < 0)
		{
			perror("sendto");
			return -1;
		}
	}
	
	return 0;
}

/******************************************************************************/
/*
 *  Parse the information returned by the time server 
 */
int getresponse(int sockfd,struct addrinfo * res,struct ntp_packet rpkt)
{
	fd_set pending_data;
	struct timeval block_time;
	char *refid ;

	/* Call the select () function and set the timeout time to 1s */
	FD_ZERO(&pending_data);
	FD_SET(sockfd, &pending_data);
	/* how time to ask */
	block_time.tv_sec=10;
	block_time.tv_usec=0;
	if (select(sockfd + 1, &pending_data, NULL, NULL, &block_time) > 0)
	{
		int num;
		/* Receive server-side information */
		if ((num = recvfrom(sockfd, &rpkt, sizeof(rpkt), 0, res->ai_addr, &res->ai_addrlen)) < 0)
		{
			perror("recvfrom");
			return 0;
		}

		/* Sets the data structure that receives the NTP packet */
		int mode = PKT_MODE(rpkt.li_vn_mode);
		int version = PKT_VERSION(rpkt.li_vn_mode);
		int leap = PKT_LEAP(rpkt.li_vn_mode);
		int stratum = rpkt.stratum;
		int poll = rpkt.poll;
		int precision = rpkt.precision;

		//Arrival client time stamp T4
		finaltimestamp=time(NULL)+JAN_1970;//-8*3600;
		//The main data sent on the network to form small end
		rpkt.root_dispersion= ntohl(rpkt.root_dispersion);
		rpkt.reftimestamphigh=ntohl(rpkt.reftimestamphigh);
		rpkt.reftimestamplow= ntohl(rpkt.reftimestamplow);
		rpkt.oritimestamphigh= ntohl(rpkt.oritimestamphigh);
		rpkt.oritimestamplow= ntohl(rpkt.oritimestamplow);
		rpkt.recvtimestamphigh= ntohl(rpkt.recvtimestamphigh);
		rpkt.recvtimestamplow= ntohl(rpkt.recvtimestamplow);
		rpkt.trantimestamphigh= ntohl(rpkt.trantimestamphigh);
		rpkt.trantimestamplow= ntohl(rpkt.trantimestamplow);
#ifdef debug
		printf("li=%d,version=%d,mode=%d\n",leap,version,mode);
		printf("stratum=%d,poll=%d,precision=%d\n",stratum,poll,precision);
		printf("################ data ####################\n");
		printf("root_delay=%ld\n",rpkt.root_delay);
		printf("dispersion=%ld\n",rpkt.root_dispersion);
		printf("refh=%lx\n",rpkt.reftimestamphigh);
		printf("relw=%lx\n",rpkt.reftimestamplow);
		printf("orih=%lx\n",rpkt.oritimestamphigh);
		printf("oril=%lx\n",rpkt.oritimestamplow);
		printf("rech=%lx\n",rpkt.recvtimestamphigh);
		printf("recl=%lx\n",rpkt.recvtimestamplow);
		printf("trah=%lx\n",rpkt.trantimestamphigh);
		printf("tral=%lx\n",rpkt.trantimestamplow);
#endif
		long64 diftime,delaytime;
		//Find the time difference between the client and the server = ((T2-T1)+(T3-T4))/2
		diftime=((rpkt.recvtimestamphigh-firsttimestamp)+(rpkt.trantimestamphigh-finaltimestamp))>>1;
		//Find delay
		delaytime=((rpkt.recvtimestamphigh-firsttimestamp)-(rpkt.trantimestamphigh-finaltimestamp))>>1;
		//Find the time stamp of the true time
		struct timeval tv1;
		tv1.tv_sec=time(NULL)+diftime+delaytime;
		tv1.tv_usec=0;
		settimeofday(&tv1,NULL);
#ifdef debug
		printf("\n\ndebug information ...\n\n");
		printf("time(NULL) is %ld\n",time(NULL));
		printf("different time is %ld\n",diftime);
		printf("delaytime is %ld\n",delaytime);
		printf("time(NULL)+diftime+delaytime=%ld\n",time(NULL)+diftime+delaytime);
		printf("tv1.tv_sec is %ld\n\n", tv1.tv_sec);
#endif
		return 1;
	}	
	return 0;
}

/******************************************************************************/
/*
 *  set RTC time func 
 */
int setRtcTime()
{
	int fd;
	time_t t = 0;
	struct tm *tm;
	struct tm __tm;
	//struct timeval tv;
	//time_t timepl;
	
	time(&t);
	tm = localtime(&t);

	if ((fd = open("/dev/rtc", O_RDWR, 0)) < 0)
	{
		printf("open /dev/rtc fail!\n");
		return -1;
	}
	
	//Modify the time setting, RTC time
	__tm.tm_year = tm->tm_year;
	__tm.tm_mon  = tm->tm_mon;
	__tm.tm_mday = tm->tm_mday;
	__tm.tm_hour = tm->tm_hour;
	__tm.tm_min  = tm->tm_min;
	__tm.tm_sec  = tm->tm_sec;
	__tm.tm_wday = tm->tm_wday;	
	//__tm.tm_mon = __tm.tm_mon - 1;
	//__tm.tm_year -= 1900;
	if (ioctl(fd, IOCTL_SET_RTC, &__tm))
	{
		printf("set time fail\n");
		close(fd);
		return -1;
	}
	//timepl = mktime(&__tm);
	//tv.tv_sec = timepl;
	//tv.tv_usec = 0;
	//Set system time consistent with RTC time
	//settimeofday(&tv,(struct timezone*)0);
	close(fd);
	
	return 0;
}

/******************************************************************************/
/*
 *  get RTC time func 
 */
int getRtcTime()
{
	int fd;
	struct tm __tm;
	struct timeval tv;
	struct tm *timenow;
	time_t timep;

	if ((fd = open("/dev/rtc", O_RDWR, 0)) < 0)
	{
		printf("open /dev/rtc fail!\n");
		return -1;
	}
	
	if (ioctl(fd, IOCTL_READ_RTC, &__tm))
	{
		printf("read time fail\n");
		close(fd);
		return -1;
	}
	
	timep = mktime(&__tm);
	tv.tv_sec = timep;
	tv.tv_usec = 0;
	//Set system time consistent with RTC time
	settimeofday(&tv, (struct timezone*)0);
	
	timenow = localtime(&timep);
	printf("Local time is %s\n",asctime(timenow));

	close(fd);
	
	return 0;
}

/******************************************************************************/
/*
 *  The function of the function 
 *  is to achieve the NTP client access to time simple process
 */
int StartNtpTime()
{
	struct addrinfo *res = NULL,hints;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
/*
 *  Call the getaddrinfo () function to get the address information
 */
	int rc = getaddrinfo(NTP_SERVER_IP, NTP_PORT_STR, &hints, &res);
	if (rc != 0)
	{
		perror("getaddrinfo");
		return -1;
	}
/*
 *  Creating a NTP socket connection
 */
	int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd < 0)
	{
		perror("socket");
		return -1; 
	}
/*
 *  send data packet
 */
	if (sendpkt(fd, res))
	{
		perror("sendpkt error\n");
	}
/*
 *  recv data packet
 */
	struct ntp_packet rpkt;
	if (getresponse(fd,res,rpkt))
	{
		if (setRtcTime())
		{
			printf("setRtcTime fail\n");
		}
		printf("ntp time is OK\n");
	}
	else
	{
		printf("ntp time is fail\n");
	}
	close(fd);

	getRtcTime();
	
	return 0;
}


int setLocalTime()
{
	struct tm _tm;
	struct timeval tv;
	time_t timep;

	//set default time 2015-1-1 12:00
	_tm.tm_sec  = 0;
	_tm.tm_min  = 0;
	_tm.tm_hour = 12;
	_tm.tm_mday = 1;
	_tm.tm_mon  = 1 - 1;
	_tm.tm_year = 2015 - 1900;
	
	timep = mktime(&_tm);
	tv.tv_sec = timep;
	tv.tv_usec = 0;
	settimeofday(&tv, (struct timezone*)0);
	return 0;
}

