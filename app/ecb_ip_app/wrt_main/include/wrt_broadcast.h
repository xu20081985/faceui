#ifndef __WRT_BROADCAST_H__
#define __WRT_BROADCAST_H__


#define HEAD						"WRTI"
#define APP_BROADCAST_0x06F0		(0x06F0)      //app ¡ú gateway cmd
#define GATEWAY_ACK_0x08F0			(0x08F0)	  //gateway ¡ú app cmd
#define WRT_MAX_BUFFER_SIZE			(1024)
#define APP_BROADCAST_DATA_LEN		(56)

typedef struct _broadcastPacket{
		char 				head[4];
		unsigned short		cmd;
		int					length;
		char				src[15];
		char 				dest[15];
}broadcastPacket;


static int broadcast_GenerateMD5(const void *data, int data_len, unsigned char md5[32]);
static int sendCmd( int sockfd, unsigned short cmd);
static int RecvBroadcastMessage(const void *buf, int len);
void *RecvBroadcastMsgThread(void *arg);
int initGateWayCfg();
int initGroupBroadcast(void);


#endif

