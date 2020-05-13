/*
 * wrt_msgHandler.cpp -- Gateway Master Program Processing
 *
 * Copyright (c) Wrt Intelligent Technology Co Ltd. 2017. All Rights Reserved.
 *
 * See the Project file for usage and redistribution requirements
 *
 *	$Id: wrt_msgHandler.cpp 	2017/06/20   Siny $
 */
 
/******************************** Description *********************************/
 
/*
 *  The module is mainly to achieve app command processing, 
 *  data filtering, classification sent to the UART processing functions, 
 *  mainly white list management, command processing, data callback package
 */
 
/********************************* Includes ***********************************/

#include "wrt_crypt.h"
#include "wrt_MsgQueue.h"
#include "sock.h"
#include "cds/dstring.h"
#include <cds/ptr_vector.h>
#include "sqlite3.h"
#include "queue.h"
#include "wrt_log.h"
#include "wrt_cfg.h"
#include "wrt_devHandler.h"
#include "wrt_msgHandler.h"

/********************************* Defines ************************************/

SIP_PACKET_STATUS sip_packet_status[MAX_SIP_PACKET]; //sip data packet
WHITE_LIST g_whiteList[WHITE_LIST_NUM_MAX];			 //white list data
CWRTMsgQueue *WRTCmdMsgQueue_g = NULL;				 //cmd queue hangle

extern netPacket g_packet;
extern CWRTMsgQueue *WRTSockMsgQueue_g;
extern T_SYSTEMINFO* pSystemInfo;
extern int get_variable_key_flag;
extern int RC4DecPacket(const void *buf, char **out_buf, const int buf_len, bool dec_flag = false );
extern int checkMD5( const void *data, int data_len, bool check_flag );
extern int send_control(const char* name,unsigned char* buf,int buflen);

/********************************* Code **************************************/
/*
 *	Generate MD5 check
 */
//======================================================
//** 函数名称: GenerateMD5
//** 功能描述: 生成MD5校验
//** 输　入: data data_len md5[32]
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================   
int GenerateMD5( const void *data, int data_len, unsigned char md5[32] )
{
	char md5Head[13];
	snprintf(md5Head, sizeof(md5Head), "$%s$", pSystemInfo->DoorSysInfo.gateWayDeviceID);
	md5Head[12] = '\0';

	int len = 12+data_len+1;
	char *temp_data = (char *)malloc(len);
	memcpy(temp_data, md5Head, 12);
	memcpy(temp_data+12, data, data_len);
	memcpy(temp_data+12+data_len, "$", 1);

	if (WRTMD5((unsigned char *)temp_data, len, md5, 32) < 0)
	{
		DEBUG_ERROR("Generate WRTMD5 failed!\n");
		free(temp_data);
		return -1;
	}

	md5[24] = '\0';
	free(temp_data);
	temp_data = NULL;
	return 0;
}

/*****************************************************************************/
/*
 *	Packet generation, RC4 encryption, MD5 checksum
 */
//======================================================
//** 函数名称: PacketRC4EncAddMD5
//** 功能描述: 数据包加密和校验
//** 输　入: buf in_len out_buf out_len add_dec_md5_flag
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================   
int PacketRC4EncAddMD5(const void *buf, const int in_len, char **out_buf, int *out_len, bool add_dec_md5_flag)
{
	char rc4buf[4096];
	memset(rc4buf, 0, sizeof(rc4buf));

	if (add_dec_md5_flag)
	{
		char variableKey[17];
		memcpy(variableKey, pSystemInfo->DoorSysInfo.variableKey, 16);
		variableKey[16] = '\0';

		unsigned char md5[32];
		memset(md5, 0, sizeof(md5));
		if (GenerateMD5( buf, in_len, md5) < 0)
		{
			return -2;
		}

		memcpy(rc4buf, buf, in_len);
		memcpy(rc4buf+in_len, md5, 16);
		unsigned char *outbuf = NULL;
		outbuf = (unsigned char *)malloc(in_len + 16);
		if (NULL == outbuf)
		{
			DEBUG_ERROR("outbuf ZENMALLOC failed!\n");
			return -1;
		}
		if (WRTRC4Encrypt(variableKey, (unsigned char *)rc4buf, in_len+16, outbuf) < 0)
		{
			DEBUG_ERROR("WRTRC4Encrypt failed!\n");
			free(outbuf);
			outbuf = NULL;
			return -3;
		}
		*out_len = in_len + 16;
		*out_buf = (char *)outbuf;
		return 0;
	}
	*out_len = in_len;
	*out_buf = (char *)buf;
	
	return 0;
}


/*****************************************************************************/
/*
 *	The gateway sends data to app (SIP or local)
 */
//======================================================
//** 函数名称: gateWaySend
//** 功能描述: 网络数据发送
//** 输　入: from sendBuf sendLen
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
int gateWaySend(const PACKET_TYPE_MSG *from, const void *sendBuf, const int sendLen)
{
	if(from->isSIP == GATEWAY_SIP_SEND)
	{
		//sip send
		DEBUG_MESSAGE( "GATEWAY_SIP_SEND->gateWaySend->remote send,remote name:%s\n", from->remotename );
		int ret = send_control(from->remotename, (unsigned char *)sendBuf, sendLen);
		 DEBUG_MESSAGE("remote send data %s:[%s]\n", (ret == 0)?("success"):("failed"), from->remotename);
		return ret;
	}
	else
	{
		//local send
		struct sockaddr_in sock_msg;
		sock_msg = from->sock_msg;
		int ret = sipsock_send( sock_msg.sin_addr, ntohs(sock_msg.sin_port), PROTO_TCP , (char *)sendBuf, sendLen );
        DEBUG_MESSAGE("TCP send data %s:[%s]\n", (ret == 0)?("success"):("failed"), inet_ntoa(sock_msg.sin_addr));
		return ret;
	}

}

/*****************************************************************************/
/*
 *	Does the test data command have control rights?
 */
//======================================================
//** 函数名称: checkCmdValid
//** 功能描述: 检查命令的有效性
//** 输　入: buf
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
int checkCmdValid(const unsigned char *buf)
{
	int index;
	unsigned char usertype;

	usertype = *(buf + 10);

	switch (usertype)
	{
		case USER_MOBLIE:
			for (index = 0; index < WHITE_LIST_NUM_MAX; index++)
			{
				if (0 == memcmp(&g_whiteList[index].mobile[0], &buf[11], 11))
					return 0;
			}
			break;
		case USER_MAC:
			for (index = 0; index < WHITE_LIST_NUM_MAX; index++)
			{
				if (0 == memcmp(&g_whiteList[index].mac[0], &buf[11], 12))
					return 0;
			}
			break;
		case USER_ADMIN:
			return 0;
		case USER_MAIL:
			for (index = 0; index < WHITE_LIST_NUM_MAX; index++)
			{
				if (0 == memcmp(&g_whiteList[index].mobile[0], &buf[11], 14))
					return 0;
			}
			break;
		default:
			break;
	}

	return -1;
}

/*****************************************************************************/
/*
 *	Network packet format for encapsulation gateway processing
 */
//======================================================
//** 函数名称: setNetPacket
//** 功能描述: 封装网络数据包
//** 输　入: datacallback net_packet from buf cmd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
int setNetPacket( void *datacallback, netPacket *net_packet, const PACKET_TYPE_MSG *from, const void *buf, const unsigned short cmd)
{
/*	data format
 *	WRTI + cmd + Length + srcID + destID + data + MD5 check
 * 	4B      2B     4B      15B     15B      xB      16B    
 */
	int data_len;
	int total_len;
	char *ptr = NULL;
	char *pdata = NULL;
	
	if (-1 == checkCmdValid((unsigned char *)buf))
	{
		DEBUG_WARNING("cmd is not allowed, not whilelist\n");
		return -1;
	}
	
	memset(net_packet, 0, sizeof(netPacket));

	pdata = (char *)buf;
	memcpy(&total_len, pdata + 6, sizeof(int));//packet len
	data_len = total_len - 56;					//len(56) = head(40B) + md5(16B)
	if (data_len < 0)
	{
		DEBUG_ERROR("recv data_len < 0 error\n");
		return -1;
	}

	if (data_len > 0)
	{
		ptr = (char *)malloc(data_len);
		CHECK_PTR(ptr, "malloc data_len fail\n");
		memcpy(ptr, pdata + 40, data_len);
	}

	net_packet->processResult = (int (*)(netResponse*,void*))datacallback;//callback function
	net_packet->buf = (unsigned char *)ptr;   			//app data
	net_packet->bufLen = (ptr != NULL) ? data_len : 0;	//app push data len
	net_packet->cmd = cmd;  							//app push cmd
	net_packet->timeStamp = (long)time(NULL);			//Timestamp of packets
	net_packet->userData.isSIP = from->isSIP;			//app network info
	if (GATEWAY_SIP_SEND == from->isSIP)
	{
		net_packet->userData.internet.chatroomptr = from->chat_room_ptr;
		memcpy(net_packet->userData.internet.data, from->remotename, 256);
	}
	else
		net_packet->userData.local.sock_msg = from->sock_msg;

	
	memcpy(&(g_packet.userData), &(net_packet->userData), sizeof(USER_DATA));

	//DEBUG_MESSAGE("ip:%x\n",net_packet->userData.local.sock_msg.sin_addr.s_addr);
	//DEBUG_MESSAGE("port:%d\n",net_packet->userData.local.sock_msg.sin_port);

	return 0;
}

/*****************************************************************************/
/*
 *  Receive app data response --ok
 */
//======================================================
//** 函数名称: ack_ok
//** 功能描述: 应答成功
//** 输　入: from cmd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int ack_ok(const PACKET_TYPE_MSG *from, const unsigned short cmd)
{
	char sendbuf[64];
	memset( sendbuf, 0, sizeof(sendbuf) );
	strncpy( sendbuf, "WRTI", 4 );
	unsigned short key_ack = 0;
	key_ack = htons( cmd );
	sendbuf[4] = key_ack & 0xff;
	sendbuf[5] = (key_ack >> 8) & 0xff;
	sendbuf[6] = 41+16;
	int len = 41+16;	
	*(char *)(sendbuf+40) = 1; //0 failed;1 ok; 2 busy;

    char *outbuf = NULL;	
	int out_len = 0;
	if( !PacketRC4EncAddMD5( sendbuf, len-16, &outbuf, &out_len, true ) )
	{
		int ret = gateWaySend(from, outbuf, out_len);
		wrt_free_memory(outbuf);
		return ret;
	}
	else
	{
		DEBUG_ERROR("PacketRC4EncAddMD5 error!\n");
		return -1;
	}
	
	return 0;
}

/*****************************************************************************/
/*
 *  Receive app data response --busy
 */
//======================================================
//** 函数名称: ack_busy
//** 功能描述: 应答忙
//** 输　入: from cmd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int ack_busy(const PACKET_TYPE_MSG *from, const unsigned short cmd)
{
	char sendbuf[64];
	memset(sendbuf, 0, sizeof(sendbuf));
	strncpy(sendbuf, "WRTI", 4);	
	unsigned short key_ack = 0;
	key_ack = htons(cmd);
	sendbuf[4] = key_ack & 0xff;
	sendbuf[5] = (key_ack >> 8) & 0xff;
	sendbuf[6] = 41+16;	
	int len = 41+16;	
	*(char *)(sendbuf+40) = 2; //0 failed;1 ok; 2 busy;
	
	char *outbuf = NULL;
	int out_len = 0;
	if (!PacketRC4EncAddMD5(sendbuf, len-16, &outbuf, &out_len, true))
	{
		int ret = gateWaySend(from, outbuf, out_len);
		wrt_free_memory(outbuf);
		return ret;
	}
	else
	{
		DEBUG_ERROR("PacketRC4EncAddMD5 error!\n");
		return -1;
	}
	
	return 0;
}

/*****************************************************************************/
/*
 *  Calculate file size
 */
//======================================================
//** 函数名称: getFileSize
//** 功能描述: 获取文件大小
//** 输　入: buf size
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
int getFileSize(const char *path)
{
	struct stat statbuf;
	int size;
	
	if (access(path, F_OK) < 0)
		return 0;

	stat(path,&statbuf);
	size = statbuf.st_size;

	return size;
}

/*****************************************************************************/
/*
 *  Check whitelist list
 */
//======================================================
//** 函数名称: checkWhiteList
//** 功能描述: 检查白名单列表
//** 输　入: buf size
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int checkWhiteList()
{
	FILE *fd = NULL;
	fd = fopen(WHITE_LIST_FILE_PATH, "ab");
	if (NULL == fd)
	{
		DEBUG_ERROR("fopen whitelist file error");
		return -1;
	}
	
	fclose(fd);

	fd = fopen(WHITE_LIST_BAK_FILE_PATH, "ab");
	if (NULL == fd)
	{
		DEBUG_ERROR("fopen whitelist file error");
		return -1;
	}
	
	fclose(fd);
	
	return 0;
}

/*****************************************************************************/
/*
 *  Query whitelist list
 */
//======================================================
//** 函数名称: queryWhiteList
//** 功能描述: 查询白名单列表
//** 输　入: buf size
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
int queryWhiteList(unsigned char *buf, int size)
{
	FILE *fd = NULL;

	if ((fd = fopen(WHITE_LIST_FILE_PATH, "rb")) == NULL)
	{
		DEBUG_ERROR("fopen whitelist file error");
		return -1;
	}

	if (fread(buf, 1, size, fd) < 0)
	{		
		DEBUG_ERROR("fread whitelist file error");
		fclose(fd);
		return -1;
	}

	fclose(fd);

	return 0;
}

/*****************************************************************************/
/*
 *  Sync whitelist list
 */
//======================================================
//** 函数名称: syncWhiteList
//** 功能描述: 同步白名单列表
//** 输　入: buf size
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================   
int syncWhiteList(const unsigned char *buf, int size)
{
	FILE *fd = NULL;
	char cmd[BUFF_SIZE] = {0};

	sprintf(cmd, "cp %s %s",WHITE_LIST_FILE_PATH, WHITE_LIST_BAK_FILE_PATH);
	system(cmd);

	if ((fd = fopen(WHITE_LIST_FILE_PATH, "wb")) == NULL)
	{
		DEBUG_ERROR("fopen whitelist file error");
		return -1;
	}

	if (fwrite(buf, size, 1, fd) <= 0)
	{
		DEBUG_ERROR("fwrite whitelist file error");
		fclose(fd);
		return -1;
	}
	
	fclose(fd);
	system("sync");
	
	return 0;
}

/*****************************************************************************/
/*
 *  Repeal whitelist list
 */
//======================================================
//** 函数名称: repealWhiteList
//** 功能描述: 替换白名单列表
//** 输　入: buf size
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
int repealWhiteList(unsigned char *buf, int size)
{
	FILE *fd = NULL;
	char cmd[256] = {0};

	sprintf(cmd, "cp %s %s", WHITE_LIST_BAK_FILE_PATH, WHITE_LIST_FILE_PATH);
	system(cmd);
    
	if ((fd = fopen(WHITE_LIST_FILE_PATH, "rb")) == NULL)
	{
		DEBUG_ERROR("fopen whitelist file error");
		return -1;
	}

	if (fread(buf, size, 1, fd) <= 0)
	{
		fclose(fd);
		DEBUG_ERROR("fread whitelist file error");
		return -1;
	}

	fclose(fd);

	return 0;
}

/*****************************************************************************/
/*
 *  Refresh whitelist list
 */
//======================================================
//** 函数名称: flushWhiteList
//** 功能描述: 刷新白名单列表
//** 输　入: buf size
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
int flushWhiteList(const unsigned char *buf, int size)
{
	if (NULL == buf || 0 == size)
		return -1;
	
	int count = 0;	
	int num = 0;
	
	num = (size - 1)/sizeof(WHITE_LIST);//Calculate white list number
	setWhiteListCount(num);
	memset(g_whiteList, 0, sizeof(g_whiteList));//Initialize the table first, and then reset it
	
	while (count != num)
	{
		memcpy((unsigned char *)&g_whiteList[count], &buf[1+(count * sizeof(WHITE_LIST))], sizeof(WHITE_LIST));
		if (++count >= WHITE_LIST_NUM_MAX - 1)
		{
			break;
		}
	}
	
	return 0;
}

/*****************************************************************************/
/*
 *  Initialize white list file
 */
//======================================================
//** 函数名称: initWhiteList
//** 功能描述: 初始化白名单
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int initWhiteList()
{
	int size;
	unsigned char databuf[8192] = {0};
	
	if (checkWhiteList() < 0)
	{
		return -1;
	}
	
	size = getFileSize(WHITE_LIST_FILE_PATH);
	if (size > 0 && size < sizeof(databuf))
	{
		if (queryWhiteList(databuf, size) < 0)
		{
			return -1;
		}
		if (flushWhiteList(databuf, size) < 0)
		{
			return -1;
		}
	}

	return 0;
}

/************************ Processing the app command **************************/
/*
 *   Reset variable key, Random password generation
 */
//======================================================
//** 函数名称: resetVariableKey
//** 功能描述: 复位密钥
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
void resetVariableKey()
{
	unsigned char randPwd[32];
	unsigned char randPwd2[32];
	memset( randPwd, 0, sizeof(randPwd) );
	memset( randPwd2, 0, sizeof(randPwd2) );

	snprintf( (char *)randPwd, sizeof(randPwd),"%05d%05d%05d%05d%05dwRtI", rand(), rand(), rand(), rand(), rand() );

	if(WRTMD5( randPwd, sizeof(randPwd), randPwd2, sizeof(randPwd2) ) < 0)
	{
		DEBUG_ERROR("WRTGetRandPwd failed!\n");
		return;
	}
	randPwd2[16] = '\0';
	
	memcpy((void *)(pSystemInfo->DoorSysInfo.variableKey), randPwd2, 17);

	DEBUG_MESSAGE("default key:%s\n", (char *)pSystemInfo->DoorSysInfo.variableKey);
	//Write configuration file
	write_system_info();
}

/*****************************************************************************/
/*
 *   The response to obtain the variable key 
 *   command only happens when the settings key is pressed
 */
//======================================================
//** 函数名称: variable_key
//** 功能描述: 获取可变密钥
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int variable_key(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	if (get_variable_key_flag == false)//只在按下设置键后才会响应获取可变密钥命令
	{
		DEBUG_MESSAGE("get variable key flag = false\n");	
		return -1;
	}

	resetVariableKey();
	char sendbuf2[256];
	unsigned char md5[32];
	memset(md5, 0, sizeof(md5));
	memset(sendbuf2, 0, 256 );
	strncpy(sendbuf2, "WRTI", 4);
	
	unsigned short key_ack =0;
	key_ack = htons(NET_GET_VARIABLE_KEY_ACK);
	sendbuf2[4] = key_ack & 0xff;
	sendbuf2[5] = (key_ack >>8) & 0xff;
	sendbuf2[6] = 40+16+16;
	int len = 40+16+16;
	
	memcpy(sendbuf2+40, (char *)(pSystemInfo->DoorSysInfo.variableKey), 16);
	
	char outbuf2[256];
	memset(outbuf2, 0, 256);
	GenerateMD5(sendbuf2, len-16, md5);	
	memcpy(sendbuf2+40+16, md5, 16);

	WRTRC4Encrypt(SOCK_KEY_1, (unsigned char *)sendbuf2, len, (unsigned char *)outbuf2);
	
	int ret = gateWaySend(from, outbuf2, len);
	if (0 == ret)
	{
		get_variable_key_flag = false;
		DEBUG_MESSAGE("variable_key->get_variable_key ok! ip:0x%x\n", from->sock_msg.sin_addr.s_addr);			
	}
	
	return 0;
}

//======================================================
//** 函数名称: group_addr_ctrl
//** 功能描述: 组地址控制
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int group_addr_ctrl(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
    if (uartGetIsSearching() || uartGetIsConfiging()) 
	{
   		ack_busy(from, NET_GROUP_CTRL_ACK);
		return 0;
	} 
	ack_ok(from, NET_GROUP_CTRL_ACK);
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_GROUP_CTRL))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: scene_ctrl
//** 功能描述: 情景控制
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int scene_ctrl(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
    if (uartGetIsSearching() || uartGetIsConfiging()) 
	{
   		ack_busy(from, NET_SCENE_CTRL_ACK);
		return 0;
	} 
	ack_ok(from, NET_SCENE_CTRL_ACK);
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_SCENE_CTRL))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: send_data_to_gw
//** 功能描述: 透传数据
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int send_data_to_gw(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_TRANSMIT_DATA))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);

	return ret;
}

int send_xml_to_gateway(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{

}

int check_get_xml_to_gateway(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{

}

//======================================================
//** 函数名称: check_scene_status
//** 功能描述: 检查组地址状态
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int check_group_status(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
    if (uartGetIsSearching() || uartGetIsConfiging()) 
	{
   		ack_busy(from, NET_GROUP_STATUS_QUERY_ACK);
		return 0;
	} 

	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_GROUP_STATUS_QUERY))
		ret = net_data_ctrl(&net_packet);
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: check_scene_status
//** 功能描述: 检查情景状态
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int check_scene_status(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	if (uartGetIsSearching() || uartGetIsConfiging()) 
	{
		ack_busy(from, NET_SCENE_STATUS_QUERY_ACK);
		return 0;
	} 

	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_SCENE_STATUS_QUERY))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: sync_group_addr
//** 功能描述: 同步组地址状态
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int sync_group_addr(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_GROUP_STATUS_SYNC))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: sync_scene_cfg
//** 功能描述: 同步情景配置
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int sync_scene_cfg(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_SCENE_STATUS_SYNC))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: check_single_group_status
//** 功能描述: 检查单个组地址状态
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int check_single_group_status(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	if (uartGetIsSearching() || uartGetIsConfiging()) 
	{
		ack_busy(from, NET_GROUP_STATUS_QUERY_SINGLE_ACK);
		return 0;
	}
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_GROUP_STATUS_QUERY_SINGLE))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: set_config
//** 功能描述: 下发配置
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int set_config(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	if (uartGetIsSearching() || uartGetIsConfiging()) 
	{
		ack_busy(from, NET_PUSH_CONFIG_DATA_ACK);
		return 0;
	}
	ack_busy(from, NET_PUSH_CONFIG_DATA_ACK);
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_PUSH_CONFIG_DATA))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: search_device
//** 功能描述: 搜索设备功能
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int search_device(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;

	usleep(100 * 1000);
	if (uartGetIsSearching() || uartGetIsConfiging()) 
	{
		ack_busy(from, NET_SEARCH_DEVICE_ACK);
		return 0;
	}
	ack_busy(from, NET_SEARCH_DEVICE_ACK);
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_SEARCH_DEVICE))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: set_sensor
//** 功能描述: 设置传感器列表
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int set_sensor(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_SET_SENSOR_DATA))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: check_air_condition_status
//** 功能描述: 检查空调状态
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int check_air_condition_status(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	if (uartGetIsSearching() || uartGetIsConfiging())  
	{
		ack_busy(from, NET_AIR_COND_STATUS_QUERY_ACK);
		return 0;
	}
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_AIR_COND_STATUS_QUERY))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: replace_gateway
//** 功能描述: 替换网关功能
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int replace_gateway(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;

	if (uartGetIsSearching() || uartGetIsConfiging())  
	{
		ack_busy(from, NET_RESPLACE_GW_DATA_ACK);
		return 0;
	} 
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_RESPLACE_GW_DATA))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: set_config_failed
//** 功能描述: 设置配置失败指令
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================
int set_config_failed(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	if (uartGetIsSearching() || uartGetIsConfiging()) 
	{
		ack_busy(from, NET_PUSH_FAIL_CONFIG_DATA_ACK);
		return 0;
	}
	//ack_busy(from, NET_PUSH_FAIL_CONFIG_DATA_ACK);
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_PUSH_FAIL_CONFIG_DATA))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: query_failed_config
//** 功能描述: 查询失败指令
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int query_failed_config(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	if (uartGetIsSearching() || uartGetIsConfiging()) 
	{
		ack_busy(from, NET_FAIL_CONFIG_DATA_QUERY_ACK);
		return 0;
	} 
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_FAIL_CONFIG_DATA_QUERY))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: manage_white_list
//** 功能描述: 管理白名单列表
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int manage_white_list(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
    int ret;
    int len = 0;
	int size = 0;
    unsigned char flag;
    unsigned char *tmpBuf = NULL;

    flag = ((unsigned char *)buf)[40];

    if (WHITE_LIST_QUERY == flag)//Query white list
    {
        size = getFileSize(WHITE_LIST_FILE_PATH);
        if (size < 100)
            size = 1;
		
        tmpBuf = (unsigned char *)malloc(size);
        if (NULL == tmpBuf)
        {
            DEBUG_ERROR("malloc tmpBuf error");
            return -1;
        }
        
        ret = queryWhiteList(tmpBuf, size);
        if (-1 == ret)
        {
            DEBUG_ERROR("query WhiteList error");
            return -1;
        }
    }
    else if (WHITE_LIST_SYNC == flag)//sync white list
    {
        ret = syncWhiteList(((unsigned char *)buf + 40), buf_len - 40 - 16);
        if (-1 == ret)
            return -1;

        flushWhiteList((unsigned char *)((char *)buf + 40), buf_len - 40 - 16);
    }
    else if (WHITE_LIST_RECOV == flag)//recovery white list
    {
        size = getFileSize(WHITE_LIST_BAK_FILE_PATH);
		if (size < 100)
            size = 1;

        tmpBuf = (unsigned char *)malloc(size);
        if (NULL == tmpBuf)
        {
			DEBUG_ERROR("malloc tmpBuf error");
            return -1;
        }
        repealWhiteList(tmpBuf, size);
        flushWhiteList(tmpBuf, size);
    }

    char sendbuf[1024 * 10] = {0};
    memset(sendbuf, 0, 1024 * 10);
    strncpy(sendbuf, "WRTI", 4);

    *(unsigned short *)(sendbuf+4) = htons(NET_MANAGE_WHITE_LIST_ACK);

    if (WHITE_LIST_SYNC == flag)
    {
        int temp_len = 40 + 16 + 1;
        memcpy( &sendbuf[6], &temp_len, 4 );
        len = temp_len;
        *(char *)(sendbuf + 40) = flag;
    }
    else if (WHITE_LIST_QUERY == flag || WHITE_LIST_RECOV == flag)
    {
        int temp_len = 40 + 16 + size;
        memcpy(&sendbuf[6], &temp_len, 4);
        len = temp_len;
        memcpy(&sendbuf[40], tmpBuf, size);
        *(sendbuf + 40) = 4;//Compatible version
    }
	wrt_free_memory(tmpBuf);

    char *outbuf = NULL;
    int out_len = 0;

    if (!PacketRC4EncAddMD5(sendbuf, len-16, &outbuf, &out_len, true))
    {     
        gateWaySend(from, outbuf, out_len); 
		wrt_free_memory(outbuf);
    }
	return 0;
}

//======================================================
//** 函数名称: syn_linkage_list
//** 功能描述: 同步联动列表
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int syn_linkage_list(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
		
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_PUSH_LINKAGE_LIST))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: set_linkage_switch
//** 功能描述: 设置联动开关
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int set_linkage_switch(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
		
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_PUSH_LINKAGE_LIST))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: set_def_flag
//** 功能描述: 设置布撤防标志
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int set_def_flag(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_SET_DEFENSE_FLAG))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: set_timer_list
//** 功能描述: 设置定时器列表
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int set_timer_list(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;

	ack_ok(from, NET_SET_TIMER_LIST_ACK);
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_SET_TIMER_LIST))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: sensor_status_report
//** 功能描述: 传感器状态上报
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int sensor_status_report(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_SENSOR_STATUS_REPORT))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: set_report_alarm
//** 功能描述: 设置上报告警
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int set_report_alarm(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	ack_ok(from, EXT_SET_REPORT_ALARM_ACK);	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, EXT_SET_REPORT_ALARM))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: set_defense_flag
//** 功能描述: 设置布撤防标志--分机用
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int set_defense_flag(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	ack_ok(from, EXT_SET_DEFENSE_FLAG_ACK);	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, EXT_SET_DEFENSE_FLAG))
		ret = net_data_ctrl(&net_packet);
	
	wrt_free_memory(net_packet.buf);
	
	return ret;
}

//======================================================
//** 函数名称: set_sync_time
//** 功能描述: 设置同步时间
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int set_sync_time(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;

	if (uartGetIsSearching() || uartGetIsConfiging())
	{
   		ack_busy(from, NET_SET_TIME_SYNC_ACK);
		return 0;
	} 	
	ack_ok(from, NET_SET_TIME_SYNC_ACK);

	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, NET_SET_TIME_SYNC))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	return ret;
}

//======================================================
//** 函数名称: set_gateway_ip
//** 功能描述: 设置网关IP--分机用
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int set_gateway_ip(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	int ret;
	netPacket net_packet;
	
	if (uartGetIsSearching() || uartGetIsConfiging())
	{
   		ack_busy(from, EXT_SET_GATEWAY_IP_ACK);
		return 0;
	} 	
	ack_ok(from, EXT_SET_GATEWAY_IP_ACK);
	
	memset(&net_packet, 0, sizeof(net_packet));
	if (!setNetPacket(NULL, &net_packet, from, buf, EXT_SET_GATEWAY_IP))
		ret = net_data_ctrl(&net_packet);

	wrt_free_memory(net_packet.buf);
	
	return ret;
}

/*
 *  Network SOCK instruction type and function pointer mapping table
 */
sockCmdUtil sockCmdUtils[] = 
{
	{0x06F2, variable_key},
	{0x061B, group_addr_ctrl},
	{0x0616, scene_ctrl},
	{0x0620, check_group_status},
	{0x0621, check_scene_status},
	{0x0622, sync_group_addr},
	{0x0624, sync_scene_cfg},
	{0x0626, check_single_group_status},
	{0x062A, set_config},
	{0x062C, search_device},
	{0x062D, set_sensor},
	{0x0692, send_data_to_gw},
	{0x0627, check_air_condition_status},
	{0x0628, send_xml_to_gateway},
	{0x0629, check_get_xml_to_gateway},		
	{0x062F, replace_gateway},
	{0x0630, set_config_failed},
	{0x0631, query_failed_config},
	{0x0632, manage_white_list},	
	{0x0634, syn_linkage_list},
	{0x0635, set_linkage_switch},
	{0x0636, set_def_flag},
	{0x0639, set_timer_list},
	{0x063A, sensor_status_report},		
	{0x0643, set_report_alarm},
	{0x0644, set_defense_flag},
	{0x0647, set_sync_time},
	{0x0666, set_gateway_ip},
};

/******************************************************************************/
/*
 *  Network command prc
 */
//======================================================
//** 函数名称: ProcessCmd
//** 功能描述: 处理命令分类
//** 输　入: from buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int ProcessCmd(const PACKET_TYPE_MSG *from, const void *buf, const int buf_len)
{
	const char *ptr = (char *)buf;
	const unsigned short cmd = ntohs(*((unsigned short *)(ptr + 4)));
	for (int index = 0; index < sizeof(sockCmdUtils)/sizeof(sockCmdUtil); index++)
	{
		if (sockCmdUtils[index].cmdType == cmd)
		{	
			int ret = sockCmdUtils[index].sockCmdPrc(from, buf, buf_len);
			return ret;
		}
	}
		
	return 0;
}

/******************************************************************************/
/*
 *  Init sip packet
 */
//======================================================
//** 函数名称: InitSIP_Packet
//** 功能描述: 初始化缓存包
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
void InitSIP_Packet()
{
	for(int i=0; i<MAX_SIP_PACKET; i++)
	{
		sip_packet_status[i].chatroomptr = 0;
		sip_packet_status[i].icopy = 0;
		sip_packet_status[i].packetLen = 0;
		sip_packet_status[i].tmpBuf = NULL;
		sip_packet_status[i].userFalg = false;
	}
}

/******************************************************************************/
/*
 *  Clear SIP packet
 */
//======================================================
//** 函数名称: clearSIP_Packet
//** 功能描述: 清除缓存包
//** 输　入: index
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
void clearSIP_Packet(const int index)
{
	sip_packet_status[index].chatroomptr = 0;
	sip_packet_status[index].icopy = 0;
	sip_packet_status[index].packetLen = 0;
	if(sip_packet_status[index].tmpBuf)
		free(sip_packet_status[index].tmpBuf);
	sip_packet_status[index].tmpBuf = NULL;
	sip_packet_status[index].userFalg = false;
}

/******************************************************************************/
/*
 *  Is there the user?
 */
//======================================================
//** 函数名称: isExist
//** 功能描述: 缓存是否存在数据包
//** 输　入: chatroomptr sipPacket
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int isExist(const unsigned long chatroomptr, const REMOTE_PACKET *sipPacket)
{
	if( (chatroomptr <= 0) || (NULL == sipPacket))
	{
		return -3;
	}
	int i = 0;
	for (i= 0; i < MAX_SIP_PACKET; i++)
	{
		if (sip_packet_status[i].chatroomptr == chatroomptr)
			break;
	}
	if (i >= MAX_SIP_PACKET)
		return -1;
	if (sip_packet_status[i].icopy + sipPacket->msglen > sip_packet_status[i].packetLen)
	{
		clearSIP_Packet(i);
		return -4;
	}
	memcpy( sip_packet_status[i].tmpBuf+sip_packet_status[i].icopy, sipPacket->msg, sipPacket->msglen );
	sip_packet_status[i].icopy += sipPacket->msglen;
	if (sip_packet_status[i].icopy == sip_packet_status[i].packetLen)
		return i;
	else
		return -2;
}

/******************************************************************************/
/*
 *  Add new SIP packets
 */
//======================================================
//** 函数名称: addSIP_Packet
//** 功能描述: 添加新的SIP数据包
//** 输　入: sipPacket
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
int addSIP_Packet(const REMOTE_PACKET *sipPacket)
{
	int i = 0;
	for (i = 0; i < MAX_SIP_PACKET; i++)
	{
		if (false == sip_packet_status[i].userFalg)
		    break;
	}
    
	if (i >= MAX_SIP_PACKET)
	{
		//packet max
		return -1;
	}

	char *outbuf_ptr = NULL;
	const bool dec_flag = true;
	if( sipPacket->msglen < 10 )
	{
		return -2;
	}

	if( !RC4DecPacket( sipPacket->msg, &outbuf_ptr, sipPacket->msglen, dec_flag ) )
	{
		if(sip_packet_status[i].tmpBuf)
		{
			clearSIP_Packet(i);
			return -3;
		}
		if(strncmp( outbuf_ptr, "WRTI", 4 ))
		{
			if( outbuf_ptr && dec_flag )
			{
				free(outbuf_ptr);
				outbuf_ptr = NULL;
			}
			return -4;
		}
		else
		{
/*
 *      packet head is correct, check the length of 
 *      the package and the length of the receipt is consistent
 */
			int packet_len = 0;
			memcpy( &packet_len, outbuf_ptr+6, sizeof(int) );
			sip_packet_status[i].userFalg = true;
			if( packet_len == sipPacket->msglen )
			{
				sip_packet_status[i].tmpBuf = (char *)malloc(sipPacket->msglen);
				if( NULL == sip_packet_status[i].tmpBuf )
				{
					if( outbuf_ptr && dec_flag )
						free(outbuf_ptr);
					outbuf_ptr = NULL;
					clearSIP_Packet(i);
					return -5;
				}
				memcpy( sip_packet_status[i].tmpBuf, sipPacket->msg, sipPacket->msglen );
				sip_packet_status[i].icopy += sipPacket->msglen;
				sip_packet_status[i].packetLen = packet_len;
				sip_packet_status[i].chatroomptr = sipPacket->chatroomptr;
				if( outbuf_ptr && dec_flag )
					free(outbuf_ptr);
				outbuf_ptr = NULL;
				return i;
			}
			if( packet_len > sipPacket->msglen )
			{
				sip_packet_status[i].tmpBuf = (char *)malloc(packet_len);
				if( NULL == sip_packet_status[i].tmpBuf )
				{
					if( outbuf_ptr && dec_flag )
						free(outbuf_ptr);
					outbuf_ptr = NULL;
					clearSIP_Packet(i);
					return -5;
				}
				memcpy( sip_packet_status[i].tmpBuf+sip_packet_status[i].icopy, 
				sipPacket->msg, 
				sipPacket->msglen );
				sip_packet_status[i].icopy += sipPacket->msglen;
				sip_packet_status[i].packetLen = packet_len;
				sip_packet_status[i].chatroomptr = sipPacket->chatroomptr;
				if( outbuf_ptr && dec_flag )
					free(outbuf_ptr);
				outbuf_ptr = NULL;
				return -100;					
			}
		}
	}
	else
	{
		if( outbuf_ptr && dec_flag )
			free(outbuf_ptr);
		outbuf_ptr = NULL;
		return -6;
	}
	return 0;
}

/******************************************************************************/
/*
 *  RC4 decrypt MD5 checksum
 */
//======================================================
//** 函数名称: rc4DecCheckMD5
//** 功能描述: 加密RC4和校验MD5
//** 输　入: buf bufLen outBuf
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
static int rc4DecCheckMD5( const void *buf, const int bufLen, char **outBuf )
{
	
	char variableKey[17];
	memcpy(variableKey, pSystemInfo->DoorSysInfo.variableKey, 16);
	variableKey[16] = '\0';

    //rc4
	unsigned char *decbuf = NULL;
	decbuf = (unsigned char *)malloc(bufLen);
	if (NULL == decbuf)
	{
		return -1;
	}
	WRTRC4Decrypt( variableKey, (unsigned char *)buf, bufLen, decbuf);

	//md5
	const bool check_flag = true;
	if (checkMD5( decbuf, bufLen, check_flag ))
	{
		if(decbuf)
			free(decbuf);
		decbuf = NULL;
		return -1;
	}
	*outBuf = (char *)decbuf;
	return 0;
}

/******************************************************************************/
/*
 *  SIP server data processing function
 */
//======================================================
//** 函数名称: dealSIP_Packet
//** 功能描述: 处理SIP数据包
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
void dealSIP_Packet(const void *arg)
{
	if (NULL == arg)
	    return;
	
	const REMOTE_PACKET *sipPacket = (const REMOTE_PACKET *)arg;
    DEBUG_WARNING("remotename : %s\n", sipPacket->remotename);

	int ret_isExist = isExist(sipPacket->chatroomptr, sipPacket);
	int ret_add = 0;
	switch (ret_isExist)
	{
		case -1:
		{
			DEBUG_MESSAGE("new sip connecting!\n");
			ret_add = addSIP_Packet(sipPacket);
			break;
		}
		case -2:
		{
			//After the bag is confiscated, continue to collect
			DEBUG_MESSAGE("\tbig packet!continue recv until recv_len == packet_len!\n");
			return;
		}
		case -3:
		{
			DEBUG_MESSAGE("sipPacket->chatroomptr == 0 or -1 or sipPacket == NULL!\n");
			if(sipPacket->msg)
				free(sipPacket->msg);
			if(sipPacket)
				free((void *)sipPacket);
			return;
		}
		case -4:
		{
			DEBUG_MESSAGE("recv sip data > *(((char *)packet)+6)!\n");
			if(sipPacket->msg)
				free(sipPacket->msg);
			if(sipPacket)
				free((void *)sipPacket);				
			return;
		}
		default:
		{
			//Receive a full package and continue to distribute it to other threads
			if((ret_isExist >= 0) && (ret_isExist <= 19))
			{
				char *outBuf = NULL;
				//Decryption, MD5 checksum
				int ret = rc4DecCheckMD5( sip_packet_status[ret_isExist].tmpBuf, sip_packet_status[ret_isExist].packetLen, &outBuf);					
				if (!ret)
				{
					SOCKET_PACKET3 tpakcet;
					memset(&tpakcet,0,sizeof(tpakcet));
					memcpy( tpakcet.from.remotename, sipPacket->remotename, 256 );
					if(sipPacket->msg)
						free(sipPacket->msg);
					if(sipPacket)
						free((void *)sipPacket);	
					//send here
					struct sockaddr_in sock_msg;
					memset( &sock_msg, 0, sizeof(sock_msg) );
					if(outBuf != NULL)
					{
						tpakcet.from.isSIP = GATEWAY_SIP_SEND;
						tpakcet.from.chat_room_ptr = sip_packet_status[ret_isExist].chatroomptr;
						tpakcet.from.sock_msg = sock_msg;
						tpakcet.validlen = sip_packet_status[ret_isExist].packetLen;
						tpakcet.buf = (unsigned char *)outBuf;
						DEBUG_MESSAGE("recv internet sip packet!rc4 MD5 ok!\n");
						
						WRT_MsgQueue_s msg;
						int dataLen = sizeof(SOCKET_PACKET3);
						memset(&msg,0,sizeof(WRT_MsgQueue_s));
						msg.myType = MSG_NODE_RECV_MSG;
						memcpy(msg.myText, &tpakcet, dataLen);
						WRTCmdMsgQueue_g->msgSnd(&msg, dataLen);
					}
					clearSIP_Packet(ret_isExist);
				}
				else
				{
					if(sipPacket->msg)
						free(sipPacket->msg);
					if(sipPacket)
						free((void *)sipPacket);	

					clearSIP_Packet(ret_isExist);
					DEBUG_MESSAGE("recv sip packet rc4DecCheckMD5 failed!\n");
				}
			}

			return;
		}
	}

	switch(ret_add)
	{
		case -1:
			DEBUG_MESSAGE("sip packet fulled!\n");
			if(sipPacket->msg)
				free(sipPacket->msg);
			if(sipPacket)
				free((void *)sipPacket);				
			return;
		case -2:
			DEBUG_MESSAGE("sip recv data < 10B!\n");
			if(sipPacket->msg)
				free(sipPacket->msg);
			if(sipPacket)
				free((void *)sipPacket);			
			return;
		case -3:
			DEBUG_MESSAGE("sip packet status not empty!!\n");
			return;
		case -4:
			DEBUG_MESSAGE("recv sip packet,but not *****wrti***** head\n");
			if(sipPacket->msg)
				free(sipPacket->msg);
			if(sipPacket)
				free((void *)sipPacket);					
			return;
		case -5:
			DEBUG_MESSAGE("recv sip data,but malloc failed!\n");
			if(sipPacket->msg)
				free(sipPacket->msg);
			if(sipPacket)
				free((void *)sipPacket);	
			return;
		case -6:
			DEBUG_MESSAGE("recv sip data,but rc4dec failed!\n");
			if(sipPacket->msg)
				free(sipPacket->msg);
			if(sipPacket)
				free((void *)sipPacket);					
			return;
		case -100:
			//After the confiscation, continue to collect
			DEBUG_MESSAGE("recv data until packet_len == *(((char *)packet)+6)!\n");
			return;
		default:
		{
			//Receive a full package and continue to distribute it to other threads
			if((ret_add >= 0) && (ret_add <= 19))
			{
				char *outBuf = NULL;
				//Decryption, MD5 checksum
				int ret = rc4DecCheckMD5( sip_packet_status[ret_add].tmpBuf, sip_packet_status[ret_add].packetLen, &outBuf);
				if( !ret )
				{
					SOCKET_PACKET3 tpakcet;
					memset(&tpakcet,0,sizeof(tpakcet));
					memcpy( tpakcet.from.remotename, sipPacket->remotename, 256 );
					if(sipPacket->msg)
						free(sipPacket->msg);
					if(sipPacket)
						free((void *)sipPacket);	
					//send here
					struct sockaddr_in sock_msg;
					memset( &sock_msg, 0, sizeof(sock_msg) );
					if(outBuf != NULL)
					{
						tpakcet.from.isSIP = GATEWAY_SIP_SEND;
						tpakcet.from.chat_room_ptr = sip_packet_status[ret_add].chatroomptr;
						tpakcet.from.sock_msg = sock_msg;
						tpakcet.validlen = sip_packet_status[ret_add].packetLen;
						tpakcet.buf = (unsigned char *)outBuf;
						
						WRT_MsgQueue_s msg;
						int dataLen = sizeof(SOCKET_PACKET3);
						memset(&msg,0,sizeof(WRT_MsgQueue_s));
						msg.myType = MSG_NODE_RECV_MSG;
						memcpy(msg.myText, &tpakcet, dataLen);
						WRTCmdMsgQueue_g->msgSnd(&msg, dataLen);
					}						
					clearSIP_Packet(ret_add);
				}
				else
				{
					if(sipPacket->msg)
						free(sipPacket->msg);
					if(sipPacket)
						free((void *)sipPacket);	

					clearSIP_Packet(ret_add);
					DEBUG_ERROR("recv sip packet rc4DecCheckMD5 failed!\n");
				}
			}
			return;			
		}	

	}
}

/******************************************************************************/
/*
 *  initMsgHandler
 */
//======================================================
//** 函数名称: initMsgHandler
//** 功能描述: 初始化消息处理
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int initMsgHandler()
{
/*
 *	Initialize sip pack
 */
	InitSIP_Packet();	
/*
 *	Initialize white list
 */
	int ret = initWhiteList();
	CHECK_RET(ret, "initWhiteList error");

	return 0;
}

/******************************************************************************/
/*
 *  wrt_msgHandler_thread
 */
//======================================================
//** 函数名称: wrt_msgHandler_thread
//** 功能描述: 网络消息处理线程
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
void *wrt_msgHandler_thread(void *arg)
{
	int ret = 0;
	WRT_MsgQueue_s rcv_msg;
	SOCKET_PACKET3 mymsgText;
	SOCKET_PACKET3 *msgText = &mymsgText;

	DEBUG_MESSAGE("wrt_msgQueueHandler thread start!\n");
	while (true)
	{
		memset(&rcv_msg, 0, sizeof(WRT_MsgQueue_s));
		ret = WRTCmdMsgQueue_g->msgRcv(&rcv_msg, MAX_TIMEOUT);
		if (0 == ret)
		{		
			memcpy(msgText, rcv_msg.myText, sizeof(SOCKET_PACKET3));

			switch (rcv_msg.myType) 
			{
				case MSG_NODE_SIP_MSG:{/* SIP data prc */		
					unsigned long event[4] = {0,0,0,0};
					REMOTE_PACKET *sipPacket = NULL;					
					memcpy( event, rcv_msg.myText, sizeof(event) );
					sipPacket = (REMOTE_PACKET *)(event[1]);
					dealSIP_Packet(sipPacket);				
					break;
				}
				case MSG_NODE_RECV_MSG:{/* Recv data prc */
					if ((msgText != NULL) && (msgText->buf != NULL) && (msgText->validlen > 0))
					{
						usleep(100 * 1000);
						ProcessCmd(&(msgText->from), msgText->buf, msgText->validlen);
						break;
					}
					break;
				}
				default:{
					DEBUG_WARNING("have no this case!!!\n");
					break;
				}
			}		
			wrt_free_memory(msgText->buf);
		} else {
			DEBUG_ERROR("msgRcv error!!!\n");
		}
	}
}


