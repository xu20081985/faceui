#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <signal.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include "sock.h"
#include "wrt_crypt.h"
#include "wrt_common.h"
#include "wrt_MsgQueue.h"
#include "ortp/ortp.h"
#include "smartUac.h"
#include "wrt_cloud.h"
#include "cds/sstr.h"
#include "cds/ptr_vector.h"
#include <vector>
#include "ortp/logging.h"
#include "wrt_cfg.h"
#include "wrt_msgHandler.h"
#include "wrt_network.h"
#include "wrt_ntp.h"

using namespace std;

#define SOCK_CTRL_MAX	(128)
#define PACKET_MAX 		(20)
#define SOCK_MAX_LEN    (4096)
#define HEART_BEAT

typedef struct _deviceInfo{
	char	dev[64];
	char	type[64];
	char	ver[64];
	char	date[64];
}DEVICE_INFO;

typedef struct _businessType{
		string name;
		vector<string> domain;
}BusinessType;

typedef struct _businessMsg{
		string devNum;
		string callNum;
		vector<BusinessType> business_type;
}BusinessMsg;

typedef struct _packetStatus{
		struct sockaddr_in sock_msg;
		char *tmp;
		int packetlen;
		int icopy;
		bool userFlag;
}packetStatus;

DEVICE_INFO           device_info;
static WRTCLOUD       cloud;
BusinessMsg 		  business_msg;
static BusinessMsg 	  temp_business;
static int 			  cloudSendFlag = 0;

int get_variable_key_flag = 0;				//�Ƿ��ǻ�ȡ��Կ
packetStatus packet_status[PACKET_MAX];		//���յİ�����
sip_tcp_ctrl sock_ctrl[2*SOCK_CTRL_MAX]; 	//���յ���������	

extern T_SYSTEMINFO	*pSystemInfo;
extern CWRTMsgQueue	*WRTCmdMsgQueue_g;
CWRTMsgQueue *WRTSockMsgQueue_g = NULL;

extern void get_version_info(char *tmp, int size);
int send_control(const char* name,unsigned char* buf,int buflen);
extern int GenerateMD5(const void *data, int data_len, unsigned char md5[32]);

//======================================================
//** ��������: RC4DecPacket
//** ��������: RC4�������ݰ�
//** �䡡��: buf out_buf buf_len dec_flag
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
int RC4DecPacket(const void *buf, char **out_buf, const int buf_len, bool dec_flag = false)
{
	if (dec_flag)
	{
		char variableKey[17];
		memcpy(variableKey, pSystemInfo->DoorSysInfo.variableKey, 16);
		variableKey[16] = '\0';

		unsigned char *outbuf = NULL;
		outbuf = (unsigned char *)malloc(buf_len);//��̬����
		if (NULL == outbuf)
		{
			*out_buf = NULL;
			return -1;
		}
		if (WRTRC4Decrypt(variableKey, (unsigned char *)buf, buf_len, outbuf) < 0)
		{
			free(outbuf);
			*out_buf = NULL;
			return -2;
		}
		*out_buf = (char *)outbuf;
		return 0;
	}
	*out_buf = (char *)buf;
	return 0;
}

//======================================================
//** ��������: checkMD5
//** ��������: У��MD5
//** �䡡��: data data_len check_flag
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
int checkMD5(const void *data, int data_len, bool check_flag)
{
	if (!check_flag)
	    return 0;
    
	unsigned char md5[32];
	memset(md5, 0, sizeof(md5));
	if (GenerateMD5( data, data_len-16, md5 ) < 0)
	{
		return -1;
	}
	md5[16] = '\0';
	char *ptr = ((char *)data)+data_len-16;
	if (memcmp(ptr, md5, 16))
	{
		DEBUG_ERROR("packet checkMD5 failed!\n");
		return -2;
	}
	return 0;
}

//======================================================
//** ��������: setPacketStatusEmpty
//** ��������: ��յ�ǰ�����������ݰ�
//** �䡡��: index
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
static void setPacketStatusEmpty( const int index )
{
	packet_status[index].icopy = 0;
	packet_status[index].packetlen = 0;
	if ( packet_status[index].tmp )
		free(packet_status[index].tmp);
	packet_status[index].tmp = NULL;
	memset( &packet_status[index].sock_msg, 0, sizeof(struct sockaddr_in) );
	packet_status[index].userFlag = false;
}

//======================================================
//** ��������: RC4DecPacket_KEY_1
//** ��������: RC4����-��Կ1���ݰ�
//** �䡡��: buf out_buf buf_len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
int RC4DecPacket_KEY_1(const void *buf, char **out_buf, const int buf_len)
{
	char variableKey[17];
	memcpy(variableKey, SOCK_KEY_1, 16);
	variableKey[16] = '\0';

	unsigned char *outbuf = NULL;
	outbuf = (unsigned char *)malloc(buf_len);
	if (NULL == outbuf)
	{
		*out_buf = NULL;
		return -1;
	}
	if (WRTRC4Decrypt(variableKey, (unsigned char *)buf, buf_len, outbuf) < 0)
	{
		free(outbuf);
		*out_buf = NULL;
		return -2;
	}
    
	*out_buf = (char *)outbuf;
	
	return 0;
}

//======================================================
//** ��������: AppGetVariableKey
//** ��������: app��ȡ�ɱ���Կ
//** �䡡��: from recv_buf recv_len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
static int AppGetVariableKey(struct sockaddr_in from, void *recv_buf, int recv_len)
{
	DEBUG_MESSAGE("====================>get_variable_key_flag:%d\n", get_variable_key_flag);
	
	int ret = 0;
	char *outbuf = NULL;

	if (recv_len != 56)
	    return -3;
	ret = RC4DecPacket_KEY_1(recv_buf, &outbuf, recv_len);
	switch(ret)
	{
		case -1:
			return -1;
		case -2:
			return -2;
		default:
			break;
	}

	//rc4���ܺ������
	if (outbuf)
	{
		if (checkMD5(outbuf, recv_len, true))
		{
			free(outbuf);
			return -6;
		}

		if (strncmp(outbuf, "WRTI", 4))
		{
			free(outbuf);
			return -4;
		}
		const unsigned short cmd = ntohs(*((unsigned short *)(outbuf+4)));
		if (cmd != NET_GET_VARIABLE_KEY)
		{
			free(outbuf);
			return -5;
		}
		//����
		SOCKET_PACKET3 tpakcet;
		memset(&tpakcet,0,sizeof(tpakcet));
		tpakcet.from.isSIP = GATEWAY_LOCAL_SEND;
		tpakcet.from.sock_msg = from;
		tpakcet.validlen = 56;
		tpakcet.buf = (unsigned char *)outbuf;
		WRT_MsgQueue_s msg;
		memset(&msg, 0, sizeof(WRT_MsgQueue_s));
		msg.myType = MSG_NODE_RECV_MSG;
		memcpy(msg.myText, &tpakcet, sizeof(SOCKET_PACKET3));
		WRTCmdMsgQueue_g->msgSnd(&msg, sizeof(SOCKET_PACKET3));
	}

	return 0;
}

//======================================================
//** ��������: resolutionPacket
//** ��������: ���������
//** �䡡��: packet_index recv_buf recv_len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
static int resolutionPacket(const int packet_index, const void *recv_buf, const int recv_len)
{
	const int recvLen = packet_status[packet_index].icopy+recv_len;
	const int packetLen = packet_status[packet_index].packetlen;
	int i = 0;
	if (recvLen > packetLen)
	{
		if (recvLen == (packetLen+2))
		{
			for (i = 0; i<recv_len; i++)
			{
				const char *tempBuf = (const char *)recv_buf;
				if ((tempBuf[i] == '\r') && (tempBuf[i+1] == '\n'))
				{
					if (i != 0)
					{
						memcpy( packet_status[packet_index].tmp+packet_status[packet_index].icopy, recv_buf, i );
						packet_status[packet_index].icopy += i;
					}
					memcpy( packet_status[packet_index].tmp+packet_status[packet_index].icopy, (char *)recv_buf+i+2, recv_len-2+i );//recv_len-2+i? recv_len-2-i
					packet_status[packet_index].icopy += (recv_len-2+i);
					if( packet_status[packet_index].icopy == packet_status[packet_index].packetlen )
					{
						DEBUG_MESSAGE("recv all data");
						return packet_index;
					}						
					return -2;
				}
				if(i == (recvLen - 2)) 
				{
					DEBUG_MESSAGE("not find \\r\\n!\n");
					memcpy( packet_status[packet_index].tmp+packet_status[packet_index].icopy, recv_buf, recv_len-2 );
					packet_status[packet_index].icopy += (recv_len-2);						
					return packet_index;
				}
			}
		}
		else
		{
			memcpy( packet_status[packet_index].tmp+packet_status[packet_index].icopy, recv_buf, recv_len-(recvLen-packetLen));
			packet_status[packet_index].icopy += (recv_len-(recvLen-packetLen));
			//DEBUG_MESSAGE("icopy:%d\npacketLen:%d\n",packet_status[packet_index].icopy,packet_status[packet_index].packetlen);
			return packet_index;				
		}
	}
	return -1;
}

//======================================================
//** ��������: recvNetworkPacket2
//** ��������: ���������
//** �䡡��: from recv_buf recv_len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
static int recvNetworkPacket2(struct sockaddr_in from, void *recv_buf, int recv_len)
{
	if (get_variable_key_flag)
	{
		printf("\n\n===========get variable key===========\n\n");
		int ret = AppGetVariableKey(from, recv_buf, recv_len);
		switch (ret)
		{
			case -1:
				return -12;
			case -2:
				return -13;
			case -3:
				return -14;
			case -4:
				DEBUG_MESSAGE("\tAppGetVariableKey not wrti head!\n");
				break;
			case -5:
				DEBUG_MESSAGE("\tAppGetVariableKey not get variable key cmd!\n");
				break;
			case -6:
				DEBUG_MESSAGE("\tAppGetVariableKey check md5 failed!\n");
				break;					
			case 0:
				break;
			default:
				break;
		}
		return -11;
	}
	
	int i = 0;
	int find_index = -1;
	bool have_empty = false;
	const bool dec_flag = true;
	char *outbuf_ptr = NULL;

	for (i = 0; i<PACKET_MAX; i++)
	{
		if (false == packet_status[i].userFlag)
		{
			have_empty = true;
		}

		if(memcmp( &from.sin_addr.s_addr, 
		&packet_status[i].sock_msg.sin_addr.s_addr, 
		sizeof(from.sin_addr.s_addr)))
		{
			continue;
		}
		else
		{
			find_index = i;				
			break;
		}
	}

	//�ҵ��Ѵ��ڵĻ�������������������
	if( find_index >= 0 )
	{
		if ((packet_status[find_index].icopy+recv_len) > packet_status[find_index].packetlen)
		{
			return resolutionPacket(find_index,recv_buf,recv_len);
		}	
		memcpy(packet_status[find_index].tmp+packet_status[find_index].icopy, recv_buf, recv_len);
		packet_status[find_index].icopy += recv_len;
		if (outbuf_ptr && dec_flag)
		    free(outbuf_ptr);
		if (packet_status[find_index].icopy == packet_status[find_index].packetlen)
		{
			DEBUG_MESSAGE("recv all data!\n");
			return find_index;
		}
		return -2;
	}

	//�µ�app���ӵ�����,���Ҵ��ڿյĻ�����
	if ((i >= PACKET_MAX) && have_empty)
	{
		for( i=0; i<PACKET_MAX; i++ )
		{
			if( false == packet_status[i].userFlag )
			{
				find_index = i;
				break;
			}
		}
		//
		if (find_index >= 0)
		{
			packet_status[find_index].userFlag = true;
			packet_status[find_index].sock_msg = from;
			//rc4 ���ܣ�falseΪ�����ܣ�trueΪ����
			if (!RC4DecPacket( recv_buf, &outbuf_ptr, recv_len, dec_flag))
			{
				if (packet_status[find_index].tmp)
				{
					DEBUG_MESSAGE("packet_status[%d].tmp not empty!\n",find_index);
					return -9;
				}
				else
				{
					if (strncmp( outbuf_ptr, "WRTI", 4))
					{
						//��ͷ���ԣ���������ʼ��packet_status
						setPacketStatusEmpty(find_index);
						if (outbuf_ptr && dec_flag)
							free(outbuf_ptr);
						return -3;
					}
					else
					{
						//��ͷ��ȷ�������������յ��ĳ����Ƿ�һ��
						int packet_len = 0;
						memcpy( &packet_len, outbuf_ptr+6, sizeof(int));
						if (packet_len == recv_len)
						{
							packet_status[find_index].tmp = (char *)malloc(packet_len);
							if (NULL == packet_status[find_index].tmp)
								return -1;
							memcpy(packet_status[find_index].tmp, recv_buf, packet_len);
							packet_status[find_index].packetlen = packet_len;
							packet_status[find_index].icopy = packet_len;
							if (outbuf_ptr && dec_flag)
								free(outbuf_ptr);
							return find_index;
						}
						else if (packet_len > recv_len)
						{
							packet_status[find_index].tmp = (char *)malloc(packet_len);
							if (NULL == packet_status[find_index].tmp)
								return -1;
							memcpy(packet_status[find_index].tmp, recv_buf, recv_len);
							packet_status[find_index].packetlen = packet_len;
							packet_status[find_index].icopy += recv_len;
							if (outbuf_ptr && dec_flag)
							    free(outbuf_ptr);
							return -2;
						}
						else
						{
							packet_status[find_index].tmp = (char *)malloc(packet_len);
							if (NULL == packet_status[find_index].tmp)
								return -1;
							memcpy(packet_status[find_index].tmp, recv_buf, packet_len);
							packet_status[find_index].packetlen = packet_len;
							packet_status[find_index].icopy = packet_len;
							if (outbuf_ptr && dec_flag)
								free(outbuf_ptr);
							return find_index;				
						}
					}
				}
			}
			else
			{
				//����ʧ��
				if (outbuf_ptr && dec_flag)
					free(outbuf_ptr);
				setPacketStatusEmpty(find_index);
				return -8;						
			}
		}
	}
	else
	{
		return -10;
	}

	return -5;
}

//======================================================
//** ��������: init_sock_packet
//** ��������: ��ʼ�����绺���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
void init_sock_packet()
{
	for (int i = 0; i < (2 * SOCK_CTRL_MAX); i++){
			sock_ctrl[i].fd = 0;
			sock_ctrl[i].keepalive_ts = 0;
			sock_ctrl[i].traffic_ts = 0;
			memset( &sock_ctrl[i].dst_addr, 0, sizeof(struct sockaddr_in) );
	}

	for (int i = 0; i < PACKET_MAX; i++)
	{
		packet_status[i].icopy = 0;
		packet_status[i].packetlen = 0;
		packet_status[i].tmp = NULL;
		memset(&packet_status[i].sock_msg, 0, sizeof(struct sockaddr_in));
		packet_status[i].userFlag = false;
	}
}

//���ֻ�����tcp����,ר��������������
//======================================================
//** ��������: socket_listen_task_gateway
//** ��������: ������ͨ��TCP�����߳�
//** �䡡��: arg
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
void *socket_listen_task_gateway(void *arg)
{
	DEBUG_MESSAGE("socket_listen_task_gateway start!\n");
	init_sock_packet();

	if (0 != sipsock_listen())
	{
		DEBUG_ERROR("sipsock_listen init error!\n");
		exit(1);
	}
    
	struct sockaddr_in from;
	int ret_protocol;
	int temp_len = 0;
	char m_recvbuffer[SOCK_MAX_LEN];
	static const unsigned char x0d0xa[2] = {'\r', '\n'};
	WRT_MsgQueue_s msg;
	
	while (true)
	{
		temp_len = 0;
		ret_protocol = -1;
		memset(m_recvbuffer, 0, sizeof(m_recvbuffer));
		memset(&from, 0, sizeof(from));
		temp_len = sipsock_waitfordata((char *)m_recvbuffer, sizeof(m_recvbuffer), &from, &ret_protocol);//���ܾ���������
		if (-2 == temp_len)
		{
#ifdef HEART_BEAT
			if ((-2 == temp_len) && ('\r' == m_recvbuffer[0]) && ('\n' == m_recvbuffer[1]))
			{
				SOCKET_PACKET3 tpakcet;
				memset(&tpakcet,0,sizeof(tpakcet));
				tpakcet.from.sock_msg = from;
				tpakcet.validlen = sizeof(x0d0xa);
				tpakcet.buf = (unsigned char *)&x0d0xa;			
				memset(&msg,0,sizeof(WRT_MsgQueue_s));
				msg.myType = MSG_NODE_HEARTBEAT;
				memcpy(msg.myText, &tpakcet, sizeof(SOCKET_PACKET3));
				WRTSockMsgQueue_g->msgSnd(&msg, sizeof(SOCKET_PACKET3));
				continue;
			}
#endif
		}
		else if (temp_len > 0)
		{		
			//DEBUG_MESSAGE("from ip:%s, temp_len:%d\n", inet_ntoa(from.sin_addr), temp_len);
			int recv_ret = recvNetworkPacket2(from, m_recvbuffer, temp_len);
			switch (recv_ret)
			{
				case -1:
					DEBUG_MESSAGE("ZENMALLOC failed!\n");
					DEBUG_MESSAGE("ZENMALLOC failed!\n");
					break;
				case -2:
					DEBUG_MESSAGE("continue recv until icopy == packet_len!\n");
					break;
				case -3:
					DEBUG_MESSAGE("not wrti head!\n");
					DEBUG_MESSAGE("not wrti head!\n");
					break;
				case -5:
					DEBUG_MESSAGE("packet_status full!\n");
					DEBUG_MESSAGE("packet_status full!\n");
					break;
				case -6:
					DEBUG_MESSAGE("recv len > packet len!\n");
					DEBUG_MESSAGE("recv len > packet len!\n");
					break;
				case -7:
					DEBUG_MESSAGE("first time recv len < packet len!\n");
					DEBUG_MESSAGE("first time recv len < packet len!\n");
					break;
				case -8:
					DEBUG_MESSAGE("rc4 dec failed\n");
					DEBUG_MESSAGE("rc4 dec failed\n");
					break;
				case -9:
					DEBUG_MESSAGE("recvNetworkPacket error!\n");
					DEBUG_MESSAGE("recvNetworkPacket error!\n");
					break;
				case -10:
					DEBUG_MESSAGE("packet_status full!\n");
					DEBUG_MESSAGE("packet_status full!\n");
					break;
				case -11:
					DEBUG_MESSAGE("try to get variable key!\n");
					DEBUG_MESSAGE("try to get variable key!\n");					
					break;
				case -12:
					DEBUG_MESSAGE("\tAppGetVariableKey malloc failed!\n");
					DEBUG_MESSAGE("\tAppGetVariableKey malloc failed!\n");					
					break;
				case -13:
					DEBUG_MESSAGE("AppGetVariableKey RC4 dec failed!\n");
					DEBUG_MESSAGE("AppGetVariableKey RC4 dec failed!\n");					
					break;
				case -14:
					DEBUG_MESSAGE("AppGetVariableKey recv len != 56......\n");
					DEBUG_MESSAGE("AppGetVariableKey recv len != 56......\n");					
					break;					
				default:
					if ((recv_ret>= 0) && (recv_ret <=19))
					{
						//DEBUG_MESSAGE("icopy : %d 		packetlen:%d		from ip  :0x%x\n", 
						//packet_status[recv_ret].icopy, 
						//packet_status[recv_ret].packetlen, 
						//packet_status[recv_ret].sock_msg.sin_addr.s_addr);
						
						//RC4 ����
						char variableKey[17];
						memcpy( variableKey, pSystemInfo->DoorSysInfo.variableKey, 16 );
						variableKey[16] = '\0';

						unsigned char *decbuf = NULL;
						decbuf = (unsigned char *)malloc(packet_status[recv_ret].packetlen);
						if (NULL == decbuf)
						{
							DEBUG_MESSAGE("ZENMALLOC failed!\n");
							continue;
						}
						WRTRC4Decrypt(variableKey, (unsigned char *)packet_status[recv_ret].tmp, packet_status[recv_ret].packetlen, decbuf);

						//md5У��
						const bool check_flag = true;
						if (checkMD5(decbuf, packet_status[recv_ret].packetlen, check_flag))
						{
							setPacketStatusEmpty(recv_ret);
							free(decbuf);
							continue;
						}
#ifdef HEART_BEAT
						//����������ʱ�䣬����Ⱥ��
						SOCKET_PACKET3 packet;
						packet.from.sock_msg = from;
						packet.validlen = sizeof(x0d0xa);
						packet.buf = (unsigned char *)x0d0xa;
						memset(&msg, 0, sizeof(WRT_MsgQueue_s));
						msg.myType = MSG_NODE_HEARTBEAT;
						memcpy(msg.myText, &packet, sizeof(SOCKET_PACKET3));
						WRTSockMsgQueue_g->msgSnd(&msg, sizeof(SOCKET_PACKET3));
#endif

						SOCKET_PACKET3 tpakcet;
						if (NULL != decbuf)
						{
							memset(&tpakcet,0,sizeof(tpakcet));
							tpakcet.from.isSIP = GATEWAY_LOCAL_SEND;
							tpakcet.from.chat_room_ptr = 0;
							tpakcet.from.sock_msg = from;
							tpakcet.validlen = packet_status[recv_ret].packetlen;
							tpakcet.buf = decbuf;
#if 1				
							printf("\nRecv app data:\n");
							for (int j = 0; j < packet_status[recv_ret].packetlen; j++)
							{
								printf("%02x ", decbuf[j]);
							}
							printf("\n\n");

							//printf("HeartBeat data: ");
							//for (int i = 0; i < packet.validlen; i++)
							//{
							//	printf("%02x ", packet.buf[i]);
							//}
							//printf("\n");
#endif
							memset(&msg, 0, sizeof(WRT_MsgQueue_s));
							msg.myType = MSG_NODE_RECV_MSG;
							memcpy(msg.myText, &tpakcet, sizeof(SOCKET_PACKET3));
							int sndRet = WRTCmdMsgQueue_g->msgSnd(&msg, sizeof(SOCKET_PACKET3));
                            if (0 != sndRet)
                            {
                                DEBUG_MESSAGE("WRTCmdMsgQueue_g->msgSnd error\n");
                                continue;
                            }
							//DEBUG_MESSAGE("WRTCmdMsgQueue_g->msgSnd ok\n");
						}
						setPacketStatusEmpty(recv_ret);
					}
					break;
			}
		}
		else
		{
			continue;
		}
	}	
    
	return NULL;
}

//======================================================
//** ��������: tcp_add
//** ��������: tcp��������
//** �䡡��: addr fd
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
int tcp_add(struct sockaddr_in addr, int fd)
{
	unsigned int i;

	struct sockaddr_in ZERO;
	memset( &ZERO, 0, sizeof(ZERO) );

	for (i=0; i < (sizeof(sock_ctrl)/sizeof(sock_ctrl[0])); i++)
	{
		if ( memcmp(&sock_ctrl[i].dst_addr.sin_addr.s_addr,
		&addr.sin_addr.s_addr,sizeof(addr.sin_addr.s_addr))==0 )
		{
			sock_ctrl[i].fd = 0;
			memcpy(&sock_ctrl[i].dst_addr, &addr, sizeof(struct sockaddr_in));
			time(&sock_ctrl[i].traffic_ts);
			sock_ctrl[i].keepalive_ts=sock_ctrl[i].traffic_ts;
			return i;
		}
	}

	for (i=0; i<(sizeof(sock_ctrl)/sizeof(sock_ctrl[0])); i++)
	{
		if ( memcmp(&sock_ctrl[i].dst_addr,&ZERO,sizeof(ZERO))==0 )
			break;
	}
	if (i >= (sizeof(sock_ctrl)/sizeof(sock_ctrl[0]))) 
	{
		DEBUG_MESSAGE("out of space in TCP cache [%s]!", inet_ntoa(addr.sin_addr));
		return -1;
	}

	sock_ctrl[i].fd = 0;
	memcpy(&sock_ctrl[i].dst_addr, &addr, sizeof(struct sockaddr_in));
	time(&sock_ctrl[i].traffic_ts);
	sock_ctrl[i].keepalive_ts = sock_ctrl[i].traffic_ts;

	DEBUG_MESSAGE("added TCP connection [%s]!\n", inet_ntoa(addr.sin_addr));

	return i;
}

//======================================================
//** ��������: tcp_remove
//** ��������: tcp�����Ƴ�
//** �䡡��: index
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
int tcp_remove(int index)
{
	//	   close(sock_ctrl[index].fd);
	sock_ctrl[index].fd = 0;
	sock_ctrl[index].keepalive_ts = 0;
	sock_ctrl[index].traffic_ts = 0;
	memset( &sock_ctrl[index].dst_addr, 0, sizeof(struct sockaddr_in) );
	return 0;
}

//======================================================
//** ��������: check_keepalive
//** ��������: ���tcp���ӻ�Ծ״̬
//** �䡡��: arg
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
void *check_keepalive(void *arg)
{
	int TCP_TIMEOUT;
	time_t now;
	time_t to_limit;
	unsigned int i;

	while (true)
	{
		TCP_TIMEOUT = 60;
		time(&now);
		to_limit = now - TCP_TIMEOUT;
		struct sockaddr_in ZERO;
		memset(&ZERO, 0, sizeof(ZERO));
		for (i = 0; i < (sizeof(sock_ctrl)/sizeof(sock_ctrl[0])); i++)
		{
			if (memcmp(&sock_ctrl[i].dst_addr, &ZERO, sizeof(ZERO)) == 0)
				continue;
			if (sock_ctrl[i].traffic_ts < to_limit)
			{
				DEBUG_WARNING("tcp disconnecting: [%s]!\n",
				inet_ntoa(sock_ctrl[i].dst_addr.sin_addr));
				tcp_remove(i);
			}
		}
		sleep(2);
	}
	
	return NULL;
}

//======================================================
//** ��������: tcp_keepalive
//** ��������: tcp�����Ծ-ˢ��ʱ���
//** �䡡��: addr fd
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
int tcp_keepalive(struct sockaddr_in addr, int fd)
{
	unsigned int i;

	for (i=0; i<(sizeof(sock_ctrl)/sizeof(sock_ctrl[0])); i++)
	{
		if (memcmp(&sock_ctrl[i].dst_addr,&addr,sizeof(struct sockaddr_in)) == 0)
			break;
	}
	if (i >= (sizeof(sock_ctrl)/sizeof(sock_ctrl[0])))
	{
		DEBUG_MESSAGE("new tcp connection: [%s]!\n", inet_ntoa(addr.sin_addr));
		return -1;
	}

	time(&sock_ctrl[i].traffic_ts);
	sock_ctrl[i].keepalive_ts = sock_ctrl[i].traffic_ts;
	//DEBUG_MESSAGE("tcp_keepalive TCP connection: [%s]!\n", inet_ntoa(addr.sin_addr));

	return i;
}


//======================================================
//** ��������: heartbeat
//** ��������: ��������
//** �䡡��: from buf buf_len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
void heartbeat(const struct sockaddr_in from, const void *buf, const int buf_len)
{
	//DEBUG_MESSAGE("heartbeat from ip:%s\n", inet_ntoa(from.sin_addr);
	
	char sendbuf[32];
	memset(sendbuf, 0, sizeof(sendbuf));
	int i = 0;
	if (buf_len == 2)
	{
		sendbuf[0] = '\r';
		sendbuf[1] = '\n';
		i = tcp_keepalive(from, 0);
		if (i < 0)
		{
			i = tcp_add(from, 0);
			if (i < 0)
			{
				DEBUG_ERROR("heartbeat tcp_add failed!\n");
				return;
			}
		}
	}
}


//======================================================
//** ��������: send2All_TCP_Connection
//** ��������: �������е�TCP����
//** �䡡��: sendbuf buf_len success failed
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
int send2All_TCP_Connection(const void *sendbuf, const int buf_len, int *success, int *failed)
{
	unsigned int i = 0;
	int send_ret = -1;
	struct sockaddr_in ZERO;
	
	for(i = 0; i < (sizeof(sock_ctrl)/sizeof(sock_ctrl[0])); i++)
	{
		if( (sock_ctrl[i].dst_addr.sin_addr.s_addr == 0) || \
		(sock_ctrl[i].dst_addr.sin_addr.s_addr == 0xffffffff) )
		continue;
	}

	memset(&ZERO, 0, sizeof(ZERO));
	for(i = 0; i < (sizeof(sock_ctrl)/sizeof(sock_ctrl[0])); i++)
	{
		if ( memcmp(&sock_ctrl[i].dst_addr, &ZERO, sizeof(ZERO)) == 0 )
			continue;

		send_ret = sipsock_send( sock_ctrl[i].dst_addr.sin_addr, ntohs(sock_ctrl[i].dst_addr.sin_port), PROTO_TCP, (char *)sendbuf, buf_len );
		if (send_ret)
		{
			DEBUG_ERROR("TCP send data failed:[%s]\n", inet_ntoa(sock_ctrl[i].dst_addr.sin_addr));
			(*failed)++;
		}
		else
		{
			(*success)++;
			tcp_keepalive( sock_ctrl[i].dst_addr, 0);
			DEBUG_MESSAGE("TCP send data success:[%s], buf_len:%d\n",inet_ntoa(sock_ctrl[i].dst_addr.sin_addr), buf_len);
		}
	}
    
	DEBUG_MESSAGE("success:%d\n", *success);
	DEBUG_MESSAGE("failed :%d\n", *failed);
	
	return 0;
}
//==========================================

//======================================================
//** ��������: send2All_SIP_Connection
//** ��������: �������е�SIP����
//** �䡡��: arg
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
int send2All_SIP_Connection(const void *arg)
{
	if (!cloudSendFlag)
		return -1;
	const SOCKET_PACKET3 *packetPtr = (const SOCKET_PACKET3 *)(arg);
	//sip send
	char userName[128];
	memset( userName, 0, sizeof(userName) );
	DEBUG_MESSAGE( "packetPtr->from.isSIP:%d    GATEWAY_SIP_SEND:0x%x\n", GATEWAY_SIP_SEND, (unsigned long)packetPtr->from.chat_room_ptr);
	if ((packetPtr->from.isSIP == GATEWAY_SIP_SEND) && (packetPtr->from.chat_room_ptr > 0) )
	{
		SOCKET_PACKET3 packetPtr_temp;
		memset( &packetPtr_temp, 0, sizeof(SOCKET_PACKET3) );
		memcpy( &packetPtr_temp, arg, sizeof(SOCKET_PACKET3) );
		DEBUG_MESSAGE( " arg:%p....send all connection or send single sip connection...remote->name:%s\n", arg, packetPtr_temp.from.remotename );
		send_control(packetPtr_temp.from.remotename,packetPtr_temp.buf,packetPtr_temp.validlen);
	}

	return 0;
}

//======================================================
//** ��������: sendAllConnectionIP
//** ��������: �������е�����-����ѡ��
//** �䡡��: pPacket
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
void sendAllConnectionIP(SOCKET_PACKET3* pPacket)
{
	int success = 0;
	int failed = 0;
	if ((pPacket != NULL) && (pPacket->buf != NULL) && (pPacket->validlen > 0))
	{
		switch (pPacket->from.isSIP)
		{
			case NET_TRANSMIT_DATA_ACK:
			{
				//͸��ָ���������֪���ӷ���
				send2All_TCP_Connection( pPacket->buf, pPacket->validlen, &success, &failed );
				send2All_SIP_Connection(pPacket);
				break;
			}
			case GATEWAY_LOCAL_SEND:
			{
				//local send
				send2All_TCP_Connection( pPacket->buf, pPacket->validlen, &success, &failed );
				break;
			}
			case GATEWAY_SIP_SEND:
			{
				//sip send
				send2All_SIP_Connection(pPacket);	
				break;
			}
			default:
			{
				break;	
			}
		}
		if (NULL != pPacket->buf)
			free(pPacket->buf);
		pPacket->buf = NULL;
	}
}


//�ȴ���Ϣ�����������ݷ��ͣ�������Ⱥ��
//======================================================
//** ��������: wrt_msgSend_thread
//** ��������: ��Ϣ�����߳�
//** �䡡��: arg
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
void *wrt_msgSend_thread(void *arg)
{
	DEBUG_MESSAGE("wrt_msgQueueSnd_test thread start!\n");	
	WRT_MsgQueue_s rcv_msg;
	SOCKET_PACKET3 mymsgText;
	SOCKET_PACKET3 *msgText = &mymsgText;

	while(true)
	{
		memset(&rcv_msg, 0, sizeof(WRT_MsgQueue_s));
		int ret = WRTSockMsgQueue_g->msgRcv(&rcv_msg, MAX_TIMEOUT);
		if(0 == ret)
		{		
			memcpy(msgText, rcv_msg.myText, sizeof(SOCKET_PACKET3));
			switch (rcv_msg.myType)
			{
				case MSG_NODE_TCP_SEND_ALL:
				{
					sendAllConnectionIP(msgText);
					break;
				}
				
				case MSG_NODE_HEARTBEAT:
				{
					if ((msgText != NULL) && (msgText->buf != NULL) && (2 == msgText->validlen))
					{
						heartbeat( msgText->from.sock_msg, msgText->buf, msgText->validlen );
						msgText->buf = NULL; //�������ݲ�û�������ڴ棬����ֵ����������ͷ��ڴ�δ���
					}
					break;
				}
				
				default:
					DEBUG_ERROR("have no cmd!!!---[snd thread]\n");
					break;
			}//switch
			wrt_free_memory(msgText->buf);
		}
		else
			printf("msgRcv err!!!\n");
	}
}

//======================================================
//** ��������: get_chat_room_ptr
//** ��������: ��ȡ�Ự��ָ��
//** �䡡��: remotename[256]
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
static const unsigned long get_chat_room_ptr(const char remotename[256])
{
	unsigned long chatroomptr = -1;
	chatroomptr = 100;
	return chatroomptr;
}

//======================================================
//** ��������: recvSIP_data
//** ��������: ����SIP�����������ݴ���
//** �䡡��: message cmd
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
static int recvSIP_data(const instantMessage *message, globalState cmd)
{
	REMOTE_PACKET *sip_packet = NULL;
	char outbuf[256];

	sip_packet = (REMOTE_PACKET *)malloc(sizeof(REMOTE_PACKET));
	if (sip_packet)
	{
		memset(sip_packet, 0, sizeof(REMOTE_PACKET));
		memset(outbuf, 0, sizeof(outbuf));
		sip_packet->msg = NULL;
		sip_packet->chatroomptr = -1;
		memcpy(sip_packet->remotename, message->remotename, 256);
		/**/
		DEBUG_MESSAGE("sip_packet->remotename:%s\n", sip_packet->remotename);
		DEBUG_MESSAGE("message->remotename:%s\n", message->remotename);
		sip_packet->chatroomptr = get_chat_room_ptr(message->remotename);
		/**/
		memcpy(sip_packet->external_body_url, message->external_url, 256);
		sip_packet->userdata = message->usrdata;
		sip_packet->msgstatus = message->msgstatus;
		sip_packet->msglen = message->msglen;

		if (message->msglen > 0)
		{
			sip_packet->msg = (char *)malloc(message->msglen);
			if (sip_packet->msg)
			{
				memcpy(sip_packet->msg, message->msg, message->msglen);
				unsigned long event[4] = {0,0,0,0};
				event[0] = cmd;
				event[1] = (unsigned long)sip_packet;
				
				WRT_MsgQueue_s msg;
				int dataLen = sizeof(event);
				memset(&msg,0,sizeof(WRT_MsgQueue_s));
				msg.myType = MSG_NODE_SIP_MSG;
				memcpy(msg.myText, event, dataLen);
				WRTCmdMsgQueue_g->msgSnd(&msg, dataLen);

				return 0;
			}
			else
			{
				if(NULL != sip_packet)
					free(sip_packet);
				return -1;
			}
		}
		//�յ��İ���Ϊ0
		if(NULL != sip_packet)
			free(sip_packet);
		return 0;
	}
	return -1;
}

//======================================================
//** ��������: send_control
//** ��������: Զ�̿������ط�������
//** �䡡��: name buf buflen
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
int send_control(const char* name, unsigned char* buf, int buflen)
{
	int ret;
	instantMessage* im = (instantMessage*)malloc(sizeof(instantMessage)+buflen+1);
	memset(im, 0, sizeof(instantMessage)+buflen+1);
	memcpy(im->msg, buf, buflen);
	im->msglen = buflen;
	im->msgstatus = 0;
	im->usrdata= 1;
	strcpy(im->remotename, name);
	ret = smartUacSendMessage(im);
	free(im);	

	return ret;
}

//======================================================
//** ��������: sip_callback_x1
//** ��������: Զ��SIP�������ػص�
//** �䡡��: state buf size
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
static int sip_callback_x1(globalState state,void* buf,int size)
{
	if(state == RegistrationOk)
	{
		cloudSendFlag = 1;
		regState* reg = (regState*)buf;
		DEBUG_MESSAGE("sip_callback_x1-zigbee xi login %s succeeded \n",reg->proxy);
	}
	else if(state == RegistrationFailed)
	{
		cloudSendFlag = 0;
		DEBUG_MESSAGE( "sip_callback_x1 RegistrationFailed-RegistrationFailed-RegistrationFailed-RegistrationFailed\n" );
	}
	else if(state == RegistrationCleared)
	{
		cloudSendFlag = 0;
		DEBUG_MESSAGE( "sip_callback_x1 RegistrationCleared-RegistrationCleared-RegistrationCleared-RegistrationCleared\n" );
	}
	else if(state == RegistrationProgress)
	{
		cloudSendFlag = 0;
		DEBUG_MESSAGE( "sip_callback_x1 RegistrationProgress-RegistrationProgress-RegistrationProgress-RegistrationProgress\n" );
	}
	else if(state == MessageDeliverOk)
	{
		//const instantMessage * im = (instantMessage*)buf;
		DEBUG_MESSAGE("msg is send ok..................................................................\n");
	}
	else if(state == MessageDeliverFailed)
	{
		//instantMessage * im = (instantMessage*)buf;
		DEBUG_MESSAGE("msg is send failed.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	}
	else if(state == SubscribeActive)
	{
		DEBUG_MESSAGE("sub is ok \n");
	}
	else if(state == notifyArrived)
	{
		DEBUG_MESSAGE("notify arrived .....\n");
	}
	else if(state == PublishOK)
	{
		DEBUG_MESSAGE("publish ........ok \n");
	}
	else if(state == NewMessageDeliver)
	{
		DEBUG_MESSAGE("recv new msg!\n");
		const instantMessage * im = (instantMessage*)buf;
		if(im == NULL)
		{
			DEBUG_MESSAGE("recv nothing!\n");
			return 0;
		}
		else
		{
			recvSIP_data(im, state);
		}
		DEBUG_MESSAGE("NewMessage is arrived \n");
	}
	return 0;
}

//======================================================
//** ��������: remote_control_login
//** ��������: Զ�̿��Ƶ�¼
//** �䡡��: cloudResult pCloud
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
//======================================================
static void remote_control_login(WRTCLOUDRESULT* cloudResult,WRTCLOUD* pCloud)
{
#if 1
	struct _auth_info _auth;
	struct _display_name_info _display;
	struct _proxy_info _proxy;
	char temp[1024];
	if(cloudResult == NULL || pCloud == NULL)
		return;
	
	memset(&_auth,0,sizeof(_auth));   
	memcpy(_auth.password,pCloud->devPassword.s,pCloud->devPassword.len);
	memcpy(_auth.userid,pCloud->deviceCode.s,pCloud->deviceCode.len); 
	memcpy(_auth.username ,pCloud->deviceCode.s,pCloud->deviceCode.len);

	memset(&_display,0,sizeof(_display));
	memcpy(_display.displayname,pCloud->deviceCode.s,pCloud->deviceCode.len);
	memcpy(_display.username,pCloud->deviceCode.s,pCloud->deviceCode.len); 

	struct _agent_info agentinfo;
	memset( &agentinfo, 0, sizeof(agentinfo) );
	snprintf(agentinfo.agent_name, sizeof(agentinfo.agent_name), "%s-@%s@", "WRTSIP-W_SMART", device_info.dev);
	snprintf(agentinfo.version, sizeof(agentinfo.version), "V%s", device_info.ver);
	
	//snprintf(agentinfo.agent_name, sizeof(agentinfo.agent_name), "%s-@%s@", "WRTSIP-W_SMART", "ECB-IP-N-901D");
	//snprintf(agentinfo.version, sizeof(agentinfo.version), "V%s", "1.01.01");

	memset(temp, '\0', sizeof(temp));
	snprintf( temp, sizeof(temp), "%s-@%s@-V%s", "WRTSIP-W_SMART", device_info.dev, device_info.ver );
	//snprintf(temp, sizeof(temp), "%s-@%s@-V%s", "WRTSIP-W_SMART", "ECB-IP-N-901D", "1.00.00");
	DEBUG_MESSAGE("set user agent:%s\n", temp );
	//set user agent
	smartUacSetUserAgent(temp);

	DEBUG_MESSAGE( "agentinfo.agent_name:%s\n", agentinfo.agent_name );
	DEBUG_MESSAGE( "agentinfo.version:%s\n", agentinfo.version );

	memset(&_proxy,0,sizeof(_proxy)); 
	_proxy.type = 0;			

#if 1
	business_msg.business_type.clear();
	business_msg.callNum.clear();
	business_msg.devNum.clear();
	void *str = NULL;
	BusinessType bt;
	memset( temp, '\0', sizeof(temp) );

	for( int i=0; i<cloudResult->result.loginResult.serverAddress.element_count; i++ ){
		str = ptr_vector_get(&cloudResult->result.loginResult.serverAddress,i);
		if( NULL == str )
			continue;
		WRTSERVER *server_ptr = (WRTSERVER *)str;
		memset( temp, '\0', sizeof(temp) );
		memcpy( temp, server_ptr->name.s, server_ptr->name.len );
		DEBUG_MESSAGE("name:%s\n", temp);
	
		bt.name.clear();
		bt.domain.clear();
	
		bt.name = temp;
		
		for( int j=0; j<server_ptr->domain.element_count; j++ ){
			void *c_str = ptr_vector_get(&server_ptr->domain, j);
			bt.domain.push_back( (char *)c_str );
			DEBUG_MESSAGE("domain:%s\n", (char *)c_str);
		}
		business_msg.business_type.push_back( bt );
	}
	for(unsigned int i=0; i<business_msg.business_type.size(); i++ ){
		DEBUG_MESSAGE( "name:%s\n", &business_msg.business_type[i].name[0] );
		for(unsigned int j=0; j<business_msg.business_type[i].domain.size(); j++ ){
			DEBUG_MESSAGE( "domain:%s\n", &business_msg.business_type[i].domain[j][0] );
		}
	}	
	temp_business = business_msg;
	const int i_size = temp_business.business_type.size();

	//���˾�����ͬ�ĵ�ַ�Ͷ˿ڵ�domain
	
	for( int i=0; i<i_size; i++ ){
		const int domain_size = temp_business.business_type[i].domain.size();
		if( domain_size ){

			for( int j=0; j<domain_size; j++ ){
				const string addr_port = temp_business.business_type[i].domain[j];

				for( int k=0; k<i_size; k++ ){
					for( int k2=0; k2<domain_size; k2++ ){
						if( (i == k) && (j == k2) )
							continue;
						if( strcmp( &addr_port[0], &temp_business.business_type[k].domain[k2][0]) == 0 ){
							temp_business.business_type[k].domain[k2] = "NULL";
						}
					}
				}
			}
		}
	}

	for(unsigned int i=0; i<temp_business.business_type.size(); i++ ){
		DEBUG_MESSAGE( "temp_business->name:%s\n", &temp_business.business_type[i].name[0] );
		for(unsigned int j=0; j<temp_business.business_type[i].domain.size(); j++ ){
			DEBUG_MESSAGE( "temp_business->domain:%s\n", &temp_business.business_type[i].domain[j][0] );
		}
	}
	char c_domain[128];
	char c_port[64];
	
	for( int j=0; j<i_size ;j++ ){

		string name = temp_business.business_type[j].name;
		if( strcmp( &name[0], "W_TALK" ) == 0 ){
			_proxy.type = 1;
		}else if( strcmp( &name[0], "W_MSG" ) == 0 ){
			_proxy.type = 2;

		}else if( strcmp( &name[0], "W_ALARM" ) == 0 ){
			_proxy.type = 3;

		}else if( strcmp( &name[0], "W_SMART" ) == 0 ){
			_proxy.type = 0;

		}else if( strcmp( &name[0], "W_IPC" ) == 0 ){
			_proxy.type = 4;

		}else{
			_proxy.type = 4;

		}

		const int k_size = temp_business.business_type[j].domain.size();
		for( int k=0; k<k_size; k++ ){
			memset( c_domain, '\0', sizeof(c_domain) );
			memset( c_port, '\0', sizeof(c_port) );
			string s_domain = temp_business.business_type[j].domain[k];
			char *temp2 = strstr( &s_domain[0], ":" );
			if( temp2 ){
					memcpy( c_domain, &s_domain[0], temp2-(&s_domain[0]) );
					snprintf( c_port, sizeof(c_port), "%s", temp2+1 );
					DEBUG_MESSAGE("====================c_domain:%s\n", c_domain);
					DEBUG_MESSAGE("====================port:%s\n", c_port);
					//login sip
					strcpy( _proxy.ip, c_domain );
					//_proxy.port = atoi( c_port );
					//_proxy.protocol = SIP_TRANSPORTS_TLS;
					_proxy.port = atoi( c_port )+1;
					//_proxy.protocol = SIP_TRANSPORTS_TLS;
					//wrt_sip_ioctl(SET_PROXY_SERVER_CMD,(void*)&_proxy,sizeof(_proxy));
					DEBUG_MESSAGE( "s_domain.c_str():%s\n", s_domain.c_str() );
					//����1Զ��sip��������ַ

					snprintf( temp, sizeof(temp), "sip:%s:%d", c_domain, atoi(c_port)+1 );
					printf("===%s  %s %s\n", temp, _auth.username, _auth.password);
					smartUacUserLogin( temp, TransportTLS, _auth.username, _auth.password );
					
					DEBUG_MESSAGE("_proxy.ip:%s\n_proxy.port:%d\n_proxy.protocol:%d\n_proxy.type:%d\ntemp:%s\n",
						_proxy.ip,
						_proxy.port,
						-1000,
						_proxy.type,
						temp
						);
			}
		}
	}
#endif

	DEBUG_MESSAGE("*****************************************\n");
	DEBUG_MESSAGE("proxy.ip = %s\nproxy.port = %d\n", _proxy.ip, _proxy.port);
	DEBUG_MESSAGE("*****************************************\n");

#endif
	
}

//======================================================
//** ��������: load_cloud
//** ��������: �ƶ˼���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��29��
//======================================================
WRTCLOUDRESULT* load_cloud()
{
	//�豸ID
	static char device_id[11];
	memset(device_id, '\0', sizeof(device_id));
	snprintf(device_id, sizeof(device_id), "%s", pSystemInfo->DoorSysInfo.gateWayDeviceID);	
	device_id[10] = '\0';
	
	//�豸����
	char password[64];
	memset(password, '\0', sizeof(password));
	snprintf(password, sizeof(password), "%s$%s", pSystemInfo->DoorSysInfo.gateWayDeviceID, SOCK_KEY_1);

	//MD5У����
	static char md5_password[64];
	memset(md5_password, 0, sizeof(md5_password));
	WRTMD5((unsigned char *)password, strlen(password), (unsigned char *)md5_password, 32);
	md5_password[8] = '\0';

	//��¼�ִ�
	char dev_string[256];
	memset(dev_string, 0, sizeof(dev_string));
	snprintf(dev_string, sizeof(dev_string), "%s-@%s@-V%s", "WRTSIP-W_SMART", device_info.dev, device_info.ver);
	//snprintf(dev_string, sizeof(dev_string), "%s-@%s@-V%s", "WRTSIP-W_SMART", "ECB-IP-N-901D", "1.01.01");

	static char *host_name = "web.wrtrd.net:8080";
	static char *backup_name = "web.wrtrd.com:8080"; 
	static int which = 0;	
	WRTCLOUDRESULT* cloudResult = NULL;
	cloud.host = zt2str(((which++ % 2) ? backup_name : host_name));
	cloud.deviceCode = zt2str(device_id);
	cloud.devPassword = zt2str(md5_password);	
	cloud.devType = zt2str(dev_string);
	cloud.isHaveCa = 1;
	cloudResult = device_login_cloud(&cloud);
	DEBUG_MESSAGE("\ndevice_login_cloud->deviceID:%s\n", device_id);
	DEBUG_MESSAGE("\ndevice_login_cloud->md5password:%s\n", md5_password);
	DEBUG_MESSAGE("\ndevice_login_cloud->dev_string:%s\n", dev_string);
	DEBUG_MESSAGE("\ncloud.isHaveCa->%d\n", cloud.isHaveCa);

	return cloudResult;
}

//======================================================
//** ��������: get_version_info
//** ��������: ��ȡ�汾��Ϣ
//** �䡡��: tmp size
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��29��
//======================================================
void get_version_info(char *tmp, int size)
{
	int index;
	
	for (index = 0; index <= size; index++)
	{
		if (*(tmp + index) == '\r' || *(tmp + index) == '\n')
			*(tmp + index) = '\0';
	}
	memset(&device_info, '\0' ,sizeof(device_info));
	memcpy(device_info.dev, tmp + 4, strlen(tmp + 4));
	tmp = tmp + strlen(tmp) + 2;
	printf("--Dev:%s\n", device_info.dev);
	memcpy(device_info.type, tmp + 5, strlen(tmp + 5));
	tmp = tmp + strlen(tmp) + 2;
	printf("--type:%s\n", device_info.type);
	memcpy(device_info.ver, tmp + 4, strlen(tmp + 4));
	tmp = tmp + strlen(tmp) + 2;
	printf("--ver:%s\n", device_info.ver);
	memcpy(device_info.date, tmp + 5, strlen(tmp + 5)); 
	printf("--date:%s\n", device_info.date);
	memcpy(pSystemInfo->BootInfo.Version, device_info.ver, strlen(device_info.ver));

}

//======================================================
//** ��������: server_time_sync
//** ��������: ������ʱ��ͬ��
//** �䡡��: timebuf
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��29��
//======================================================
void server_time_sync(char *timebuf)
{
	struct tm _tm;
	struct timeval tv;
	time_t timep;

	printf("%s\n", timebuf);
	_tm.tm_year = (timebuf[0]-0x30)*1000 + (timebuf[1]-0x30)*100 + (timebuf[2]-0x30)*10 + (timebuf[3]-0x30);
	_tm.tm_mon  = (timebuf[5]-0x30)*10  + (timebuf[6]-0x30);
	_tm.tm_mday = (timebuf[8]-0x30)*10  + (timebuf[9]-0x30);
	_tm.tm_hour = (timebuf[11]-0x30)*10 + (timebuf[12]-0x30);
	_tm.tm_min  = (timebuf[14]-0x30)*10 + (timebuf[15]-0x30);
	_tm.tm_sec  = (timebuf[17]-0x30)*10 + (timebuf[18]-0x30);

	_tm.tm_year -= 1900;
	_tm.tm_mon -= 1;

	timep = mktime(&_tm);
	tv.tv_sec = timep;
	tv.tv_usec = 0;
	settimeofday(&tv, (struct timezone*)0);	
	//����RTCʱ��
	setRtcTime();
}

int remote_control_init()
{
	const char* caFile = CA_FILE;
	//ortp_init();
	//ortp_set_log_level_mask(ORTP_WARNING|ORTP_ERROR|ORTP_FATAL|ORTP_DEBUG|ORTP_MESSAGE);
	//ortp_init_log_tcp(60000);
	//smartUacSetLogEnabled(0);
	smartUacInit(sip_callback_x1, NULL);
	smartUacSetCAPath(caFile);
	return 0;
}

//======================================================
//** ��������: remoteControlInit
//** ��������: Զ�̿��Ƴ�ʼ��
//** �䡡��: from cmd
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��29��
//======================================================
void *remoteControlInit(void *arg)
{
	DEBUG_MESSAGE("remoteControlInit start!\n");
	//sleep(10);
	WRTCLOUDRESULT* cloudResult = NULL;
	while (true)
	{
		cloudResult = load_cloud();
		if (cloudResult && cloudResult->yes)
		{
			DEBUG_MESSAGE("cloudResult->yes:%d,cloudResult->reason:%s\n",cloudResult->yes,cloudResult->reason.s);
			break;
		}
		else
		{
			DEBUG_ERROR("device login cloud failed!try again after 10 second.....\n reason:%s!\n",
			(cloudResult == NULL)?("cloudResult is NULL"):(cloudResult->reason.s));
			sleep(10);
		}
	}

	if (cloudResult)
	{
		DEBUG_MESSAGE("load_cloud ok \n");
		DEBUG_MESSAGE("currentTime:%s\ndevNum:%s\ncallNum:%s\n",
					  cloudResult->result.loginResult.currentTime.s,
					  cloudResult->result.loginResult.devNum.s,
					  cloudResult->result.loginResult.callNum.s);

		if (str_len(&cloudResult->result.loginResult.currentTime) > 0)
		{ 
			//�ⲽһ��Ҫ������ʱ�䡣ֻ��ϵͳʱ���֤���ʱ�����ssl���ܳɹ���
			//ͬ��ʱ�䣬ʱ�䲻�ԣ���¼��ʧ��
			//StartNtpTime();
			server_time_sync(cloudResult->result.loginResult.currentTime.s);
		}
		remote_control_init();
		remote_control_login(cloudResult, &cloud);
		device_free_cloud_result(cloudResult, LOGIN);
	}

	while (true)
	{
		sleep(120);
	}
}



