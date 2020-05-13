#ifndef __WRT_MSGHANDLER_H__
#define __WRT_MSGHANDLER_H__

#include "wrt_common.h"

#define MAX_SIP_PACKET 					(20)
#define WHITE_LIST_NUM_MAX              (50)

//date send way 1:sip 2:local
#define GATEWAY_SIP_SEND				(0x0001)
#define GATEWAY_LOCAL_SEND				(0x0002)

//whitelist info 
typedef struct _WHITE_LIST
{
	unsigned char mobile[64];
	unsigned char mac[12];
	unsigned char name[20];
	unsigned char startTime[14];
	unsigned char finishTime[14];
}WHITE_LIST;

typedef void (*display)(unsigned long event[4]);

typedef struct _sip_tcp_cache{
		int fd;
		struct sockaddr_in dst_addr;
		time_t traffic_ts;
		time_t keepalive_ts;
}sip_tcp_ctrl;

typedef struct _gateWayConfig{
		unsigned long gateWayIp;
		char gateWayID[11];
}GateWayConfig;

typedef struct _remote_packet{
	unsigned long chatroomptr;
	char remotename[256];
	char external_body_url[256];
	int  userdata;
	int msgstatus;
	int  msglen;
	char *msg;
}REMOTE_PACKET;

typedef struct _sip_packet_status{
	unsigned long chatroomptr;
	int	packetLen;
	int	icopy;
	bool userFalg;
	char* tmpBuf;	
}SIP_PACKET_STATUS;

typedef struct _packetTypeMsg{
	int isSIP;
	struct sockaddr_in sock_msg;
	unsigned long chat_room_ptr;
	char remotename[256];
}PACKET_TYPE_MSG;

typedef struct socket_packet_t3{
	PACKET_TYPE_MSG from;
	int  validlen;
	unsigned char* buf;
}SOCKET_PACKET3;

typedef struct sockCmdUtil
{
	unsigned short cmdType;
	int (*sockCmdPrc)(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len);
}sockCmdUtil;

//
enum _user_type
{
	USER_MOBLIE = 1,
	USER_MAC,
	USER_ADMIN,
	USER_MAIL,
};


int initMsgHandler();
int checkWhiteList();
int syncWhiteList(const unsigned char *buf, int size);
int repealWhiteList(unsigned char *buf, int size);
int queryWhiteList(unsigned char *buf, int size);
int flushWhiteList(const unsigned char *buf, int size);
int getFileSize(const char *path);
int GenerateMD5( const void *data, int data_len, unsigned char md5[32] );

#endif

