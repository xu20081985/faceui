#ifndef WRT_COMMON_H
#define WRT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include "sqlite3.h"

#define DEBUG_ERROR 							printf
#define DEBUG_WARNING 							printf
#define DEBUG_MESSAGE 							printf

#define RANDOM_ID_LEN							8
#define GW_ID_TYPE								70
#define BUFF_SIZE								256
#define SOCK_KEY_1								"heX6SW4QpJeJx5iK"
#define SOCK_KEY_2								"BnGqwF3EkJgViYNB"

#define CA_FILE									"/FlashDev/config/client.pem"
#define VER_FILE								"/UserDev/version"
#define WRT_CFG_FILE 							"/UserDev/config/cfg_gw"
#define WHITE_LIST_FILE_PATH					"/UserDev/config/white_list"
#define WHITE_LIST_BAK_FILE_PATH				"/UserDev/config/white_list_bak"
#define FAILLIST_FILE_PATH   					"/UserDev/config/fail_cfg"
#define SQLITE_PATH_C 							"/UserDev/config/sql_gw.db"

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

#define NET_GET_VARIABLE_KEY					(0x06F2)//��ȡ��Կ
#define NET_GET_VARIABLE_KEY_ACK				(0x08F2)

#define NET_GROUP_CTRL							(0x061B)//���ַ����
#define NET_GROUP_CTRL_ACK						(0x081B)

#define NET_SCENE_CTRL							(0x0616)//�龰����
#define NET_SCENE_CTRL_ACK						(0x0816)

#define NET_TRANSMIT_DATA						(0x0692)//͸������
#define NET_TRANSMIT_DATA_ACK					(0x0893)

#define NET_CAL_PERCENT_DATA    				(0x0694)//��ȡ�ٷֱ�
#define NET_CAL_PERCENT_DATA_ACK   				(0x0894)

#define NET_GROUP_STATUS_QUERY    				(0x0620)//���ַ״̬��ѯ
#define NET_GROUP_STATUS_QUERY_ACK  			(0x0820)

#define NET_SCENE_STATUS_QUERY    				(0x0621)//�龰״̬��ѯ
#define NET_SCENE_STATUS_QUERY_ACK  			(0x0821)

#define NET_GROUP_STATUS_SYNC    				(0x0622)//���ַ״̬ͬ��
#define NET_GROUP_STATUS_SYNC_ACK  				(0x0822)
	
#define NET_SCENE_STATUS_SYNC    				(0x0624)//�龰״̬ͬ��
#define NET_SCENE_STATUS_SYNC_ACK  				(0x0824)

#define NET_GROUP_STATUS_QUERY_SINGLE    		(0x0626)//���ַ״̬��ѯ�����豸
#define NET_GROUP_STATUS_QUERY_SINGLE_ACK		(0x0826)

#define NET_AIR_COND_STATUS_QUERY   			(0x0627)//�յ��豸״̬��ѯ
#define NET_AIR_COND_STATUS_QUERY_ACK			(0x0827)

#define NET_PUSH_XML_DATA   					(0x0628)//�·�XML�ļ�
#define NET_PUSH_XML_DATA_ACK					(0x0828)

#define NET_XML_DATA_QUERY   					(0x0629)//��ȡXML�ļ�
#define NET_XML_DATA_QUERY_ACK					(0x0829)

#define NET_PUSH_CONFIG_DATA    				(0x062A)//�·���������
#define NET_PUSH_CONFIG_DATA_ACK    			(0x082A)

#define NET_SEARCH_DEVICE    					(0x062C)//�����豸
#define NET_SEARCH_DEVICE_ACK					(0x082C)

#define NET_SET_SENSOR_DATA    					(0x062D)//�·�����������
#define NET_SET_SENSOR_DATA_ACK					(0x082D)

#define NET_RESPLACE_GW_DATA    				(0x062F)//�滻����
#define NET_RESPLACE_GW_DATA_ACK				(0x082F)

#define NET_PUSH_FAIL_CONFIG_DATA    			(0x0630)//�·�����ʧ������
#define NET_PUSH_FAIL_CONFIG_DATA_ACK			(0x0830)

#define NET_FAIL_CONFIG_DATA_QUERY    			(0x0631)//��ѯ����ʧ������
#define NET_FAIL_CONFIG_DATA_QUERY_ACK			(0x0831)

#define NET_MANAGE_WHITE_LIST    				(0x0632)//���������
#define NET_MANAGE_WHITE_LIST_ACK				(0x0832)

#define NET_PUSH_LINKAGE_LIST    				(0x0634)//�·������б�
#define NET_PUSH_LINKAGE_LIST_ACK				(0x0834)

#define NET_SET_LINKAGE_SWTICH    				(0x0635)//������������
#define NET_SET_LINKAGE_SWTICH_ACK				(0x0835)

#define NET_SET_DEFENSE_FLAG    				(0x0636)//�豸��������־
#define NET_SET_DEFENSE_FLAG_ACK				(0x0836)

#define NET_SET_TIMER_LIST    					(0x0639)//���ö�ʱ���б�
#define NET_SET_TIMER_LIST_ACK					(0x0839)

#define NET_SENSOR_STATUS_REPORT    			(0x063A)//������״̬�ϱ�
#define NET_SENSOR_STATUS_REPORT_ACK			(0x083A)

#define EXT_SET_REPORT_ALARM    				(0x0643)//�����ϱ��澯--�ֻ�
#define EXT_SET_REPORT_ALARM_ACK				(0x0843)

#define EXT_SET_DEFENSE_FLAG    				(0x0644)//���ò�������־--�ֻ�
#define EXT_SET_DEFENSE_FLAG_ACK				(0x0844)

#define EXT_GET_DEFENSE_FLAG    				(0x0645)//��ȡ��������־--�ֻ�
#define EXT_GET_DEFENSE_FLAG_ACK				(0x0845)

#define NET_SET_TIME_SYNC    					(0x0647)//ͬ��ʱ��
#define NET_SET_TIME_SYNC_ACK					(0x0847)

#define EXT_SET_GATEWAY_IP    					(0x0666)//��������IP--�ֻ�
#define EXT_SET_GATEWAY_IP_ACK					(0x0866)

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
#define CHECK_RET(a, b)  do{if(0 != a){DEBUG_ERROR("%s\n", b);return -1;}}while(0)
#define CHECK_PTR(a, b)  do{if(NULL == a){DEBUG_ERROR("%s\n", b);return -1;}}while(0)

//Msg enum define
typedef enum
{
	MSG_NODE						= 1024,
	MSG_NODE_SIP_MSG				= MSG_NODE + 1,
	MSG_NODE_RECV_MSG 				= MSG_NODE + 2,
	MSG_NODE_HEARTBEAT				= MSG_NODE + 3,
	MSG_NODE_TCP_SEND_ALL 			= MSG_NODE + 4,
} WRTCmd_e;

////////////////////////////////////////////////////////

enum _RUN_STATUS
{
	NORMAL_STATUS = 1,
	GET_KEY_STATUS,
	READY_REBOOT_STATUS,
};

enum _STATUS_TIME
{
	GET_KEY_STATUS_TIME = 5,
	READY_REBOOT_TIME = 15,
	RESTORY_FACTORY_TIME = 20,
};

struct ethtool_value {  
        unsigned int      cmd;  
        unsigned int      data;  
}; 

#pragma pack(1)

typedef struct _fromInternet{
	unsigned long chatroomptr;
	char data[256];
}fromInternet;

typedef struct _fromLocal{
	struct sockaddr_in sock_msg;
	int fd;
	char data[124];
}fromLocal;

typedef struct _userData{									//����ͨѶ��Ϣ
	int				isSIP;									//�Ƿ�SIP��־
	fromInternet	internet;								//SIPͨѶ��Ϣ
	fromLocal		local;									//TCPͨѶ��Ϣ
}USER_DATA;

typedef struct _netResponse{ 							//����Ӧ���
	int flag; 												//�ص�������־
	unsigned short cmd; 									//TcpӦ��ָ������
	unsigned char *buf; 									//TcpӦ��ָ�����
	int bufLen;												//TcpӦ���������
}netResponse;

typedef struct _netPacket{								//�������ݰ�
	int (*processResult)(netResponse *spn,void* userData);  //�ص�����
	USER_DATA userData;										//�ص���������
	unsigned short cmd;										//Tcp�·�ָ������
	unsigned char* buf; 									//Tcp�·�ָ�����
	int   bufLen;											//Tcp�·���������
	long  timeStamp;										//Tcpʱ���
}netPacket;

typedef struct _uartPacket{ 								//UartӦ���
	unsigned short cmd; 									//UartӦ��ָ������
	unsigned char *buf;     								//UartӦ��ָ������
	int bufLen; 		    								//UartӦ��ָ���
}uartPacket;

#pragma pack()

unsigned int crc32(unsigned char *buf, int len);
void wrt_free_memory(void *data_ptr);
int pthread_create_t(void *(* func)(void *));
int get_version_file();
unsigned int getDataNaBytes(const unsigned char *str, int pos, int num);
unsigned char uartCheck(unsigned char *str);
int get_gateway_version(const char *str, int *mcuVer);
int getFormatTime(unsigned char data[]);
int dec2Hex(const unsigned char *inStr, char *outStr, int len);


#endif

