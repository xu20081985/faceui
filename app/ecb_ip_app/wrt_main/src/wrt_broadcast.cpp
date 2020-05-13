#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <net/if.h>
#include "sock.h"
#include "wrt_crypt.h"
#include "wrt_common.h"
#include "wrt_MsgQueue.h"
#include "wrt_cfg.h"
#include "wrt_broadcast.h"
#include "wrt_devHandler.h"
#include "wrt_msgHandler.h"

int brdcstSockFd_g;
struct sockaddr_in peeraddr_g;
GateWayConfig gateWayCfg;
extern T_SYSTEMINFO* pSystemInfo;
static const unsigned short group_port 	= 12836;
static const char *group_addr 			= "255.255.255.255";
extern int errno;

//======================================================
//** 函数名称: broadcast_GenerateMD5
//** 功能描述: 广播生成MD5
//** 输　入: data data_len md5
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
static int broadcast_GenerateMD5(const void *data, int data_len, unsigned char md5[32])
{
	const char *md5Head = "$0000000000$";
	int len = 12+data_len+1;
	char *temp_data = (char *)malloc(len);
	memcpy(temp_data, md5Head, 12);
	memcpy(temp_data+12, data, data_len);
	memcpy(temp_data+12+data_len, "$", 1);
	if (WRTMD5((unsigned char *)temp_data, len, md5, 32) < 0){
		DEBUG_ERROR("Generate WRTMD5 failed!\n");
		free(temp_data);
		temp_data = NULL;
		return -1;
	}
	md5[24] = '\0';
	free(temp_data);
	temp_data = NULL;
	return 0;
}

//======================================================
//** 函数名称: sendCmd
//** 功能描述: 发送命令
//** 输　入: sockfd cmd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
static int sendCmd( int sockfd, unsigned short cmd)
{
	char send_buff[256];
	char rc4_buf[256];
    
    cmd = htons(cmd);
	memset(send_buff, 0, sizeof(send_buff));
	const int cmdlen = 40+4+10+16;//cmdHead + ip + id + md5
	strncpy(send_buff, HEAD, 4);
	memcpy(send_buff+4, &cmd, 2);
	memcpy(send_buff+6, &cmdlen, 4);
	memcpy(send_buff+10, "000000000000000", 15);
	memcpy(send_buff+25, "000000000000000", 15);

	const unsigned long _ip = -1;
	memcpy(send_buff+40, &_ip, sizeof(unsigned long));
	memcpy(send_buff+40, &gateWayCfg.gateWayIp, sizeof(unsigned long));
	strncpy(send_buff+40+4, gateWayCfg.gateWayID, 10);

	if (!strncmp(gateWayCfg.gateWayID, "7100000000", 10)){
		DEBUG_ERROR("gateWayID error:7100000000!!!\n");
		return -1;
	}

	unsigned char md5[32];
	memset(md5, '\0', sizeof(md5));
	broadcast_GenerateMD5(send_buff, cmdlen - 16, md5);

	memcpy(send_buff+40+4+10, md5, 16);

	int ret = WRTRC4Encrypt(SOCK_KEY_1, (unsigned char *)send_buff, cmdlen, (unsigned char *)rc4_buf);
    if (ret < 0)
    {
        DEBUG_ERROR("WRTRC4Encrypt error!\n");
        return -1;
    }
    
	struct sockaddr_in broadaddr;
	broadaddr.sin_family = AF_INET;   
	broadaddr.sin_port = htons(group_port);  
	broadaddr.sin_addr.s_addr = inet_addr(group_addr);

	if (sendto(sockfd, rc4_buf, cmdlen, 0, (struct sockaddr *)&broadaddr, sizeof(struct sockaddr_in)) < 0){  
		DEBUG_ERROR("broadcast sendto error!\n");
        return -1;
	}
    
	return 0;
}

//======================================================
//** 函数名称: RecvBroadcastMessage
//** 功能描述: 接收广播消息
//** 输　入: buf len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
static int RecvBroadcastMessage(const void *buf, int len)
{
	char head[5] = {0};
	memcpy(head, buf, 4);
	
	const broadcastPacket *ptr = (const broadcastPacket *)buf;
	unsigned char md5[32];
	memset(md5, '\0', sizeof(md5));
	if (strncmp(ptr->head, "WRTI", 4) != 0)
	{
		DEBUG_ERROR("broadcast data head error\n");
		return -1;
	}

	broadcast_GenerateMD5(buf, 40, md5);
	if (memcmp(md5, (char *)buf+40, 16))
	{
		DEBUG_ERROR("broadcast data md5 error\n");
		return -1;
	}

	const unsigned short cmd = ntohs(ptr->cmd);
	switch (cmd)
	{
		case APP_BROADCAST_0x06F0:
			sendCmd(brdcstSockFd_g, GATEWAY_ACK_0x08F0);
			break;
		default:
			break;
	}
	return 0;
}

//======================================================
//** 函数名称: RecvBroadcastMsgThread
//** 功能描述: 广播接收消息线程
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
void *RecvBroadcastMsgThread(void *arg)
{
	struct sockaddr_in peeraddr;
	socklen_t socklen = sizeof(struct sockaddr_in);
	memcpy((void *)&peeraddr, &peeraddr_g, sizeof(peeraddr));	
	char recmsg[WRT_MAX_BUFFER_SIZE];
	char decmsg[WRT_MAX_BUFFER_SIZE];
	int nread = -1;

	while (true)
	{
		memset(recmsg, 0, WRT_MAX_BUFFER_SIZE);
		memset(decmsg, 0, WRT_MAX_BUFFER_SIZE);
		
		nread = recvfrom(brdcstSockFd_g, recmsg, WRT_MAX_BUFFER_SIZE, 0, (struct sockaddr *)&peeraddr, &socklen);  
		if (nread < 0)
		{  
			DEBUG_ERROR("recvfrom err in udptalk!\n");  
			continue;
		}
		else
		{
			initGateWayCfg(); //ip init;
			if (peeraddr.sin_addr.s_addr == gateWayCfg.gateWayIp)
				continue;

			if (nread != APP_BROADCAST_DATA_LEN){
				continue;
			}
			
			if(WRTRC4Decrypt(SOCK_KEY_1, (unsigned char *)recmsg, nread, (unsigned char *)decmsg) < 0)
			{
				DEBUG_ERROR("WRTRC4Decrypt failed!\n");
				continue;
			}
			
			RecvBroadcastMessage(decmsg, nread);
		}
	}
    
	return NULL;
}

//======================================================
//** 函数名称: initGateWayCfg
//** 功能描述: 初始化网关配置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int initGateWayCfg()
{	
	int sockfd;
	struct ifreq ifr_ip;
	struct sockaddr_in *sin;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		DEBUG_ERROR("[initGateWayCfg]---socket error\n");
		return -1;
	}
	
	memset(&ifr_ip, 0, sizeof(ifr_ip));
	strncpy(ifr_ip.ifr_name, "eth0", sizeof(ifr_ip.ifr_name) - 1); 
	ioctl(sockfd, SIOCGIFADDR, &ifr_ip);
	if (ioctl(sockfd, SIOCGIFADDR, &ifr_ip) < 0)     
	{    
		 DEBUG_ERROR("[initGateWayCfg]---ioctl SIOCGIFADDR error\n");
	     return -1;     
	}

	sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
	//DEBUG_MESSAGE("local ip: %s\n", inet_ntoa(sin->sin_addr));
	gateWayCfg.gateWayIp = sin->sin_addr.s_addr;
	close(sockfd);
	return 0;
}

//======================================================
//** 函数名称: initGroupBroadcast
//** 功能描述: 初始化广播服务
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int initGroupBroadcast(void)
{
	int ret;
		
	if ((brdcstSockFd_g = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		DEBUG_ERROR("socket brdcstSockFd_g creat error\n");
		return -1;
	}

	memset(&peeraddr_g, 0, sizeof(struct sockaddr_in));  
	peeraddr_g.sin_family = AF_INET;   
	peeraddr_g.sin_port = htons(group_port);  
	peeraddr_g.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(brdcstSockFd_g, (struct sockaddr *)&peeraddr_g, sizeof(struct sockaddr_in));
	CHECK_RET(ret, "bind error\n");

	int on = 1;
	ret = setsockopt(brdcstSockFd_g, SOL_SOCKET, SO_BROADCAST, (const char *)&on, sizeof(on));
	CHECK_RET(ret, "setsockopt error");
	pthread_create_t(RecvBroadcastMsgThread);//creat broadcast thread
	return 0;
}

