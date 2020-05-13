/*
 * wrt_devHandler.cpp -- Gateway Master Program Processing
 *
 * Copyright (c) Wrt Intelligent Technology Co Ltd. 2017. All Rights Reserved.
 *
 * See the Project file for usage and redistribution requirements
 *
 *	$Id: wrt_devHandler.cpp 	2017/06/20   Siny $
 */
 
/******************************** Description *********************************/
 
/*
 *   This file is mainly for device side data processing, 
 *   through the WRT protocol access to the corresponding command processing function.
 *   Gateway and device end communication bridge, callback device data to the APP side, 
 *   to achieve data control, status synchronization and other functions
 */
 
/********************************* Includes ***********************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <time.h>
#include "sock.h"
#include "wrt_MsgQueue.h"
#include "wrt_devHandler.h"
#include "wrt_msgHandler.h"
#include "wrt_serial.h"
#include "sqlite3.h"
#include "threadPool.h"
#include "queue.h"
#include "json.h"
#include "wrt_cloud.h"
#include "wrt_log.h"
#include "wrt_cfg.h"
#include "wrt_crypt.h"
#include "wrt_network.h"
#include "wrt_ntp.h"

/********************************* Defines ************************************/

netPacket g_packet;							/* Device data callback package */
sqlite3 *db;								/* The database handle */
int serial_fd;								/* Serial port handle */
queue_t *g_queue_linkage; 					/* Linked data queue */
queue_t *g_queue_ctrlAck; 					/* Control response queue */
static ptr_vector_t g_netCmd;				/* Save the network instruction data that is issued */
static dstring_t g_group;					/* Save group status data */
static dstring_t g_devs;					/* Save search device data */
static dstring_t g_faillist;				/* Save failed instruction data */
static dstring_t g_statelist;				/* Save sensor state data */
unsigned short g_percent;					/* Percentage value */
unsigned char  g_linkagenum;				/* Store the current linkage table number */
unsigned char g_AkeyDefense;    			/* Store the current linkage table number */
unsigned char g_seqnum;						/* Store the current seq number */
unsigned char g_scene[2]; 					/* Store the current scene table number */
unsigned int g_mcuId;						/* Store the gateway id */
int g_mcuVer[3];							/* Store the gateway version */
int g_isSearching; 							/* Store the searching state */
int g_isConfiging;							/* Store the Configuration state */
int g_whiteCount;							/* Store the white list number */
SENSOR_TIMEOUT sensorTimeout[BUFF_SIZE];    /* Store the sensor timeout time */


/* Define mutex variables */
static pthread_mutex_t g_sceneMutex;
static pthread_mutex_t g_mcuIdMutex;
static pthread_mutex_t g_isSearchingMutex;
static pthread_mutex_t g_seqnumMutex;
static pthread_mutex_t g_AkeyDefenseMutex;
static pthread_mutex_t g_sqlMutex;
static pthread_mutex_t g_ctrlMutex;


extern CWRTMsgQueue *WRTSockMsgQueue_g;
extern T_SYSTEMINFO* pSystemInfo;
extern GateWayConfig gateWayCfg;
extern CWRTLogManage *WRT_log_g;
extern WHITE_LIST g_whiteList[WHITE_LIST_NUM_MAX];

/********************************* code ******************************************/
/*
 *	Initializing defense status
 */
int init_defense_state()
{
	g_AkeyDefense = (unsigned char)pSystemInfo->DoorSysInfo.reserved[0];
	return 0;
}

/******************************************************************************/
/*
 *	Initializing database Load
 */
int sqlite3_load_init()
{
	int ret = -1;	
	char file_name[BUFF_SIZE] = {0};
	
	memset(file_name, 0, sizeof(file_name));
	sprintf(file_name, "%s", SQLITE_PATH_C);

	ret = sqlite3_open(file_name, &db);
	CHECK_RET(ret, "sqlite3_open error");

	return 0;
}

/******************************************************************************/
/*
 *   Initializing database
 */
int sqlite3_table_init(const char buf[][256], int size)
{
	int ret;
	int index;

	for (index = 0; index < size; index++)
	{
		ret = ecb_sqlite3_exec(db, buf[index], NULL, NULL, NULL);
		if (SQLITE_OK != ret)
		{
			DEBUG_ERROR("ecb_sqlite3_exec error index:%d", index);
			return -1;
		}
	}

	return 0;
}

/******************************************************************************/
/*
 *   Initializing queue
 */
//======================================================
//** 函数名称: init_global_queue
//** 功能描述: 初始化全局队列
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int init_global_queue()
{
	int ret;
/*
 *   Queues are used to transmit serial data
 */	
	ret = q_init(&g_queue_linkage);
	CHECK_RET(ret, "g_queue_linkage error");
	ret = q_init(&g_queue_ctrlAck);
	CHECK_RET(ret, "g_queue_ctrlAck error");	
	ret = dstr_init(&g_faillist, 100);
	CHECK_RET(ret, "g_faillist error");
	ret = dstr_init(&g_group, 100);
	CHECK_RET(ret, "g_group error");
	ret = dstr_init(&g_devs, 100);
	CHECK_RET(ret, "g_devs error");
	ret = dstr_init(&g_statelist, 100);
	CHECK_RET(ret, "g_devs error");	
	ret = ptr_vector_init(&g_netCmd, 100);
	CHECK_RET(ret, "g_netCmd error");

	return 0;
}

/******************************************************************************/
/*
 *   Get the gateway id 
 */
//======================================================
//** 函数名称: get_gateway_id
//** 功能描述: 获取id
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int get_gateway_id()
{
	int sock;
	struct ifreq ifreq; 

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		DEBUG_ERROR("socket creat error\n");
		return -1; 
	}
	strcpy(ifreq.ifr_name, "eth0"); 
	if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0) 
	{ 
		DEBUG_ERROR("ioctl SIOCGIFHWADDR error\n");
		close(sock);
		return -1; 
	}	
	g_mcuId = crc32((unsigned char *)ifreq.ifr_hwaddr.sa_data, 6);
	sprintf(gateWayCfg.gateWayID, "%02d%08x", GW_ID_TYPE, uartGetMcuId());
	sprintf(pSystemInfo->DoorSysInfo.gateWayDeviceID,"%s", gateWayCfg.gateWayID);
	DEBUG_MESSAGE("WRT id = %s\n", gateWayCfg.gateWayID);
	
	close(sock);
	return 0;
}

/******************************************************************************/
/*
 *   Get the gateway id and version
 */
//======================================================
//** 函数名称: get_id_version
//** 功能描述: 获取id和版本
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int get_id_version()
{
	int ret;
	
	ret = get_version_file();
	CHECK_RET(ret, "get_device_info error");
	
	ret = get_gateway_version((const char *)pSystemInfo->BootInfo.Version, g_mcuVer);
	CHECK_RET(ret, "get_gateway_version error");

	ret = get_gateway_id();
	CHECK_RET(ret, "get_gateway_id error");

	return 0;
}

/******************************************************************************/
/*
 *   Initialize device module information
 */
//======================================================
//** 函数名称: init_dev_handler
//** 功能描述: 初始化设备功能
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int init_dev_handler()
{
	int ret;
	int num;
	char sqlText[20][256] = {{0},{0}};
/*
 *	Initializing mutex locks
 */

	pthread_mutex_init(&g_sceneMutex, NULL);
	pthread_mutex_init(&g_mcuIdMutex, NULL);
	pthread_mutex_init(&g_isSearchingMutex, NULL);
	pthread_mutex_init(&g_seqnumMutex, NULL);
	pthread_mutex_init(&g_AkeyDefenseMutex, NULL);
	pthread_mutex_init(&g_sqlMutex, NULL);
	pthread_mutex_init(&g_ctrlMutex, NULL);
/*
 *	Initializing queue
 */
 
	ret = init_global_queue();
	CHECK_RET(ret, "init_global_queue error");

/*
 *	Initializing database
 */
	num = 0;
	sprintf(sqlText[num++], "%s", "create table ecbState(groupAddr INTEGER UNIQUE, way INTEGER, arg INTEGER)");
	sprintf(sqlText[num++], "%s", "create table ecbScene(scene INTERGER, groupAddr INTERGER, way INTEGER, arg INTERGER)");
	sprintf(sqlText[num++], "%s", "create table devInfo(source INTERGER, dest INTERGER, serialNum INTERGER, id INTERGER UNIQUE, type INTERGER, version INTERGER, ident INTERGER, report INTERGER)");
	sprintf(sqlText[num++], "%s", "create table rfSensor(id INTEGER, type INTEGER, dataH INTEGER, dataL INTEGER, groupAddr INTEGER, way INTEGER, arg INTEGER)");
	sprintf(sqlText[num++], "%s", "create table numAndMac(serialNum INTERGER, mac INTERGER)");
	sprintf(sqlText[num++], "%s", "create table sensorState(id INTEGER UNIQUE, type INTEGER, para1 INTEGER, para2 INTEGER, para3 INTEGER, para4 INTEGER, para5 INTEGER)");
	sprintf(sqlText[num++], "%s", "create table linkageRule(actionNum INTEGER, id INTEGER, type INTEGER, linkpara INTEGER, linkcond INTEGER, linkswitch INTEGER, alarmswitch INTEGER)");
	sprintf(sqlText[num++], "%s", "create table performAction(actionNum INTEGER, ord INTEGER, groupAddr INTEGER, way INTEGER, arg INTEGER, sleep INTEGER)");
	sprintf(sqlText[num++], "%s", "create table actionName(actionNum INTEGER UNIQUE, nm TEXT)");
	sprintf(sqlText[num++], "%s", "create table pushAlarm(nm TEXT, id INTEGER, type INTEGER, arg INTEGER, pushflag INTEGER)");	
	sprintf(sqlText[num++], "%s", "create table timerList(actionnum INTEGER UNIQUE, timeryear INTEGER, timertime INTEGER, ctrlway INTEGER, para1 INTEGER, para2 INTEGER, linkswitch INTEGER, defswitch INTEGER)");
	ret = sqlite3_load_init();
	CHECK_RET(ret, "sqlite3_load_init error");
	ret = sqlite3_table_init(sqlText, num);
	CHECK_RET(ret, "sqlite3_table_init error");

/*
 *	Initialize serial port
 */
	serial_fd = uartopen(2);
	uartset(serial_fd, 115200, 8, 'N', 1);

/*
 *	Default non search
 */
	uartSetIsSearching(0);
	uartSetIsConfiging(0);

/*
 *	Access to defense state
 */	
	init_defense_state();	

/*
 *	Initializing app data callback
 */
	InitUartDataToAppCallBack();

	return 0;
}

/******************************************************************************/
/*
 *	 Initializing app data callback
 */
void InitUartDataToAppCallBack()
{
	memset(&g_packet, 0, sizeof(g_packet));
	g_packet.cmd = NET_TRANSMIT_DATA_ACK;
	g_packet.processResult = uartDataToAppCallBack;
}

/******************************************************************************/
/*
 *	 Set a key sign of defence
 */
int setAkeyDefense(int data)
{
	pthread_mutex_lock(&g_AkeyDefenseMutex);
	g_AkeyDefense = data;
	pSystemInfo->DoorSysInfo.reserved[0]= (unsigned long)g_AkeyDefense;
	write_system_info();
	pthread_mutex_unlock(&g_AkeyDefenseMutex);
	return 0;
}

/******************************************************************************/
/*
 *	Get a key sign of defence
 */
int getAkeyDefense()
{
	return g_AkeyDefense;
}

/******************************************************************************/
/*
 *	Sets the search status flag
 */
int uartSetIsSearching(int data)
{
	pthread_mutex_lock(&g_isSearchingMutex);
	g_isSearching = data;
	pthread_mutex_unlock(&g_isSearchingMutex);
	return 0;
}

/******************************************************************************/
/*
 *	Gets the search status flag
 */
int uartGetIsSearching()
{
	return g_isSearching;
}

/******************************************************************************/
/*
 *	Sets the config status flag
 */
int uartSetIsConfiging(int data)
{
	pthread_mutex_lock(&g_isSearchingMutex);
	g_isConfiging = data;
	pthread_mutex_unlock(&g_isSearchingMutex);
	return 0;
}

/******************************************************************************/
/*
 *	Gets the config status flag
 */
int uartGetIsConfiging()
{
	return g_isConfiging;
}

/******************************************************************************/
/*
 *	Sets the current scene number
 */
int uartSetVarScene(unsigned char Hdata, unsigned char Ldata)
{
	pthread_mutex_lock(&g_sceneMutex);
	g_scene[0] = Hdata;
	g_scene[1] = Ldata;
	pthread_mutex_unlock(&g_sceneMutex);

	return 0;
}

/******************************************************************************/
/*
 *	 Gets the current scene number
 */
unsigned char *uartGetVarScene()
{
	return g_scene;
}

/******************************************************************************/
/*
 *	 Set gateway ID
 */
int uartSetMcuId(unsigned int id)
{
	pthread_mutex_lock(&g_mcuIdMutex);
	g_mcuId = id;
	pthread_mutex_unlock(&g_mcuIdMutex);
	return 0;
}

/******************************************************************************/
/*
 *	 Get gateway ID
 */
unsigned int uartGetMcuId()
{
	return g_mcuId;
}

/******************************************************************************/
/*
 *	 Set white list number
 */
int setWhiteListCount(int count)
{
	g_whiteCount = count;
	return 0;
}

/******************************************************************************/
/*
 *	 Get white list number
 */
int getWhileListCount()
{
	return g_whiteCount;
}

//======================================================
//** 函数名称: uartGetSeqnumUp
//** 功能描述: 串口获取序号
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
unsigned char uartGetSeqnumUp()
{
	pthread_mutex_lock(&g_seqnumMutex);
	g_seqnum++;
	if (0 == g_seqnum)
	{
		g_seqnum = 1;
	}
	pthread_mutex_unlock(&g_seqnumMutex);

	return g_seqnum;
}


/******************************************************************************/
/*
 *	 Sqllite3 database exec
 */

//======================================================
//** 函数名称: ecb_sqlite3_exec
//** 功能描述: 数据库执行
//** 输　入: db sql aa bb cc
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int ecb_sqlite3_exec(sqlite3 *db, const char *sql, void *aa, void *bb, void *cc)
{
	if (NULL == db || NULL == sql)
	{
		return -1;
	}
	
	pthread_mutex_lock(&g_sqlMutex);
	sqlite3_exec(db, sql, NULL, NULL, NULL);
	pthread_mutex_unlock(&g_sqlMutex);

	return 0;
}

/******************************************************************************/
/*
 *	Sqlite3 get data
 */
//======================================================
//** 函数名称: ecb_sqlite3_get_table
//** 功能描述: 数据库获取表
//** 输　入: db zSql pazResult pnRow pnColumn pzErrmsg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int ecb_sqlite3_get_table( sqlite3 *db, const char *zSql, char ***pazResult, int *pnRow, int *pnColumn, char **pzErrmsg)
{
	if (NULL == db || NULL == zSql || NULL == pazResult || NULL == pnRow || NULL == pnColumn)
	{
		return -1;
	}

	pthread_mutex_lock(&g_sqlMutex);
	sqlite3_get_table(db, zSql, pazResult, pnRow, pnColumn, NULL);
	pthread_mutex_unlock(&g_sqlMutex);

	return 0;
}

/******************************************************************************/
/*
 *	Check database data
 */
//======================================================
//** 函数名称: checkSqlData
//** 功能描述: 检查sql数据
//** 输　入: sqlData pos size
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int checkSqlData(char **sqlData, int pos, int size)
{
	int index;
	
	for (index = 0; index < size; index++)
	{
		if (NULL == sqlData[index + pos])
		{
			return -1;
		}
	}
	
	return 0;
}

/******************************************************************************/
/*
 *	clear queue data
 */
//======================================================
//** 函数名称: chearQueueData
//** 功能描述: 清空队列数据
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int chearQueueData()
{
	data_t data = {0};
	
	while (q_size(g_queue_ctrlAck))
	{
		data = q_pop_head(g_queue_ctrlAck, 0, 200);
		wrt_free_memory(((uartPacket*)(data.ptr))->buf);
		wrt_free_memory(data.ptr);
	}

	return 0;
}

//======================================================
//** 函数名称: WRTGateWayRate
//** 功能描述: 测试网关发射功率
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int WRTGateWayRate()
{
	unsigned char uartSndData[SER_MAX_LENGTH];
	memset(uartSndData, 0, sizeof(uartSndData));
	uartSndData[SER_DATA_HEAD0] = 0x5A;
	uartSndData[SER_DATA_HEAD1] = 0xA5;
	uartSndData[SER_DATA_LENGTH]= 6+9;
	uartSndData[SER_DATA_SRCH]  = 0xFF;
	uartSndData[SER_DATA_SRCL]  = 0xFF;
	uartSndData[SER_DATA_DESTH] = 0xFF;
	uartSndData[SER_DATA_DESTL] = 0xFF;
	uartSndData[SER_DATA_CMDH]  = 0x01;
	uartSndData[SER_DATA_CMDL]  = 0x03;
	uartSndData[SER_DATA_PARA0] = *(unsigned char *)(((char *)(&g_mcuId))+3);//device id
	uartSndData[SER_DATA_PARA1] = *(unsigned char *)(((char *)(&g_mcuId))+2);
	uartSndData[SER_DATA_PARA2] = *(unsigned char *)(((char *)(&g_mcuId))+1);
	uartSndData[SER_DATA_PARA3] = *(unsigned char *)(((char *)(&g_mcuId))+0);
	uartSndData[SER_DATA_PARA4] = 0x30; //device type
	uartSndData[SER_DATA_PARA5] = 0x0F;
	uartSndData[SER_DATA_PARA6] = g_mcuVer[1];//device version
	uartSndData[SER_DATA_PARA7] = g_mcuVer[2];
	uartCheck(&uartSndData[SER_DATA_LENGTH]);
	SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH]+3);

	return 0;
}



/*****************************************************************************/
/*
 *	Network side receive data write log
 */
//======================================================
//** 函数名称: uartRecvMesToLog
//** 功能描述: 网络接收消息到日志
//** 输　入: packet buf len cmd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int netRecvMesToLog(netPacket *packet, const unsigned char *buf, const int len, unsigned short cmd)
{
	int index;
	netLogData nld;
	char *net_ip = NULL;

	if (cmd == NET_PUSH_CONFIG_DATA ||
		cmd == NET_PUSH_FAIL_CONFIG_DATA)
		return 0;
	
	memset(&nld, 0, sizeof(netLogData));
	
	WRT_log_g->getTimeYmdms(nld.time);
	memcpy(nld.cmdType, "NET", sizeof("NET"));
	
	if (GATEWAY_SIP_SEND == packet->userData.isSIP)
	{
		memcpy(nld.cmdWay, "SIP", sizeof("SIP"));
		memcpy(nld.srcName, packet->userData.internet.data, sizeof(nld.srcName) - 1);
	}
	else
	{
		memcpy(nld.cmdWay, "LOCAL", sizeof("LOCAL"));
		net_ip = inet_ntoa(packet->userData.local.sock_msg.sin_addr);
		memcpy(nld.srcName, net_ip, strlen(net_ip));	
		
	}
	
	sprintf(nld.dstName, "%02d%08x", GW_ID_TYPE, uartGetMcuId());

	nld.cmd = cmd;
	
	for (index = 0; index < sizeof(netCmdArr)/sizeof(CMDLIST); index++)
	{
		if (nld.cmd == netCmdArr[index].cmd)
		{
			memcpy(nld.cmdCN, netCmdArr[index].ch, 40);
			break;
		}
	}
	
	WRT_log_g->ecbLogDec2Hex(buf, nld.buf, len);
	
	WRT_log_g->logWrite("%s@@%s@@%s@@%s@@%s@@%04x@@%s@@%s@@</br>\n",//log logging
					nld.time,
					nld.cmdType,
					nld.cmdWay,
					nld.srcName,
					nld.dstName,
					nld.cmd,
					nld.cmdCN,
					nld.buf);
	
	return 0;
}

/*****************************************************************************/
/*
 *	uart side receive data write log
 */
//======================================================
//** 函数名称: uartRecvMesToLog
//** 功能描述: 串口接收消息到日志
//** 输　入: buf len cmd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int uartRecvMesToLog(const unsigned char *buf, const int len, unsigned short cmd)
{
	int ret;
	int index;
	uartLogData sld;
	
	memset(&sld, 0, sizeof(uartLogData));
	WRT_log_g->getTimeYmdms(sld.time);
	memcpy(sld.cmdName, "ECB", sizeof("ECB"));
	sld.srcId = 0xFFFFFFFF;
	sld.dstId = uartGetMcuId();
	sld.cmd = cmd;
	for (index = 0; index < sizeof(uartCmdArr)/sizeof(CMDLIST); index++)
	{
		if (cmd == uartCmdArr[index].cmd)
		{
			memcpy(sld.cmdCN, uartCmdArr[index].ch, 40);
			break;
		}
	}
	
	WRT_log_g->ecbLogDec2Hex(buf, sld.buf, len);
	WRT_log_g->logWrite("%s@@%s@@%08x@@%08x@@%04x@@%s@@%s@@</br>\n",//log logging
							sld.time,
							sld.cmdName,
							sld.srcId,	
							sld.dstId, 
							sld.cmd, 
							sld.cmdCN,
							sld.buf);

	return 0;
}

/*****************************************************************************/
/*
 *	check_sensor_timeout
 */
//======================================================
//** 函数名称: check_sensor_timeout
//** 功能描述: 检查传感器状态超时线程
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
void *check_sensor_timeout(void * arg)
{
	int TCP_TIMEOUT;
	time_t now;
	time_t to_limit;
	SQL_INFO sqlInfo;
	unsigned int index;
	SENSOR_TIMEOUT sensorZero;

	while (true)
	{
		TCP_TIMEOUT = 120;
		time(&now);
		to_limit = now - TCP_TIMEOUT;
		memset(&sensorZero, 0, sizeof(sensorZero));
		for (index = 0; index < (sizeof(sensorTimeout)/sizeof(sensorTimeout[0])); index++)
		{
			if (memcmp(&sensorTimeout[index].id, &sensorZero, sizeof(sensorZero)) == 0)
				continue;
			if (sensorTimeout[index].timeout < to_limit)
			{
				DEBUG_WARNING("sensor timeout remove! id: %08x type: %02x \n",
					sensorTimeout[index].id, sensorTimeout[index].type);			
				memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
				sprintf(sqlInfo.sql, "update sensorState set para1 = %d, para2 = %d, para3 = %d, para4 = %d, para5 = %d where id = %d", 
						0, 0, 0, 0, 0, sensorTimeout[index].id);
				ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
				memset(&sensorTimeout[index].id, 0, sizeof(sensorZero));
				continue;
			}
		}
		sleep(5);
	}
	
	return NULL;
}


/*****************************************************************************/
/*
 *	update_sensor_timeout
 */
//======================================================
//** 函数名称: update_sensor_timeout
//** 功能描述: 更新传感器状态超时
//** 输　入: sensorInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int update_sensor_timeout(SENSOR_INFO *sensorInfo)
{	
	time_t now;
	unsigned int index;

	time(&now);
	
	if (ALARM_SENSOR_TYPE == sensorInfo->type || 
		MULT_ENV_SENSOR_TYPE == sensorInfo->type ||
		PM_SENSOR_TYPE == sensorInfo->type ||
		MULT_ENV_HEART_SENSOR_TYPE == sensorInfo->type ||
		BODY_LIGHT_SENSOR_TYPE == sensorInfo->type)
	{
		for (index = 0; index < sizeof(sensorTimeout)/sizeof(sensorTimeout[0]); index++)
		{
			if (sensorInfo->id == sensorTimeout[index].id && sensorInfo->type == sensorTimeout[index].type)
			{
				sensorTimeout[index].id = sensorInfo->id;
				sensorTimeout[index].type = sensorInfo->type;
				sensorTimeout[index].timeout = now;
				return 0;
			}
		}
		if (index == sizeof(sensorTimeout)/sizeof(sensorTimeout[0]))
			DEBUG_ERROR("add new sensor timeout id: %08x type: %02x\n", sensorInfo->id, sensorInfo->type);
		
		for (index = 0; index < sizeof(sensorTimeout)/sizeof(sensorTimeout[0]); index++)
		{
			if (0 == sensorTimeout[index].id && 0 == sensorTimeout[index].type)
			{
				sensorTimeout[index].id = sensorInfo->id;
				sensorTimeout[index].type = sensorInfo->type;
				sensorTimeout[index].timeout = now;
				return 0;
			}
		}
		if (index == sizeof(sensorTimeout)/sizeof(sensorTimeout[0]))
			DEBUG_ERROR("max sensor timeout arrray! not update!!");
	}

	return 0;
}

/******************************************************************************/
/*
 *	net_sensor_status_report
 */
//======================================================
//** 函数名称: net_sensor_status_report
//** 功能描述: 传感器状态上报
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int net_sensor_status_report(netPacket *packet)
{	
	int ret;	
	int index;
	SQL_INFO sqlInfo;
	SENSOR_INFO sensorInfo;
	unsigned char dataBuf[BUFF_SIZE];
	
	ret = ecb_sqlite3_get_table(db, "select * from sensorState", &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret,"ecb_sqlite3_get_table error");
		
	for (index = 7; index < (sqlInfo.row + 1) * sqlInfo.col; index += 7)
	{
		if(checkSqlData(sqlInfo.result, index, 7))
		{
			DEBUG_ERROR("null result");
			continue;
		}			
		sensorInfo.id    = atoi(sqlInfo.result[index + 0]);
		sensorInfo.type  = atoi(sqlInfo.result[index + 1]);
		sensorInfo.para1 = atoi(sqlInfo.result[index + 2]);
		sensorInfo.para2 = atoi(sqlInfo.result[index + 3]);
		sensorInfo.para3 = atoi(sqlInfo.result[index + 4]);
		sensorInfo.para4 = atoi(sqlInfo.result[index + 5]);
		sensorInfo.para5 = atoi(sqlInfo.result[index + 6]);
		memset(dataBuf, 0, sizeof(dataBuf));
		dataBuf[0]  = UART_DATA_HEAD1;
		dataBuf[1]  = UART_DATA_HEAD2;
		dataBuf[2]  = ((unsigned char*)&sensorInfo.id)[3];
		dataBuf[3]  = ((unsigned char*)&sensorInfo.id)[2];
		dataBuf[4]  = ((unsigned char*)&sensorInfo.id)[1];
		dataBuf[5]  = ((unsigned char*)&sensorInfo.id)[0];
		dataBuf[6]  = ((unsigned char*)&sensorInfo.type)[0];
		memcpy(&dataBuf[7], &sensorInfo.para1, 20);
		dstr_append(&g_statelist, (char *)dataBuf, 27);
	}

	ret = compose_packet_send_app(NET_SENSOR_STATUS_REPORT_ACK);
	CHECK_RET(ret, "NET_SENSOR_STATUS_REPORT_ACK error");

	return 0;
}


/******************************************************************************/
/*
 *	update_sensor_status_data
 */
//======================================================
//** 函数名称: update_sensor_status_data
//** 功能描述: 更新传感器状态数据
//** 输　入: pdata sensorInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int update_sensor_status_data(uartPacket *pdata, SENSOR_INFO *sensorInfo)
{
	int ret;
	int paraLen;
	SQL_INFO sqlInfo;
	unsigned char dataBuf[BUFF_SIZE];
	memset(sensorInfo, 0, sizeof(SENSOR_INFO)); 
	sensorInfo->id = getDataNaBytes(pdata->buf, 10, 4);
	sensorInfo->type = getDataNaBytes(pdata->buf, 14, 1);
	paraLen = pdata->bufLen - 16;
	memcpy((char *)&sensorInfo->para1, (char *)(pdata->buf + 15), paraLen); 
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "update sensorState set para1 = %d, para2 = %d, para3 = %d, para4 = %d, para5 = %d where id = %d", 
			sensorInfo->para1, sensorInfo->para2, sensorInfo->para3, sensorInfo->para4, sensorInfo->para5, sensorInfo->id);
	ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
	CHECK_RET(ret, "update sensorState data error");

	update_sensor_timeout(sensorInfo);

	net_sensor_status_report(NULL);

	return 0;
}


/******************************************************************************/
/*
 *	query_sensor_linkage_data
 */
//======================================================
//** 函数名称: compara_sensor_linkage_data
//** 功能描述: 比较传感器联动数据
//** 输　入: sensorInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int query_sensor_linkage_data(SENSOR_INFO *sensorInfo)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	unsigned char actionNum;
	unsigned int linkSwitch;
	unsigned int alarmSwitch;
	unsigned char *linkNumber;

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select actionNum, linkswitch, alarmswitch from linkageRule where id = %d and type = %d", sensorInfo->id, sensorInfo->type);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_get_table error");

	for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
	{
		if(checkSqlData(sqlInfo.result, index, 3))
		{
			DEBUG_ERROR("null result");
			continue;
		}	
	
		actionNum	= atoi(sqlInfo.result[index + 0]);
		linkSwitch	= atoi(sqlInfo.result[index + 1]);
		alarmSwitch = atoi(sqlInfo.result[index + 2]);	

		if (compara_sensor_linkage_data(actionNum))
		{
			DEBUG_MESSAGE("compara_sensor_linkage_data fail actionnum = %d\n", actionNum);
			continue;
		}

		if (compara_sensor_status_data(sensorInfo, actionNum))
		{
			DEBUG_MESSAGE("compara_sensor_status_data same actionnum = %d\n", actionNum);
			continue;
		}

		if (compara_timer_linkage_data(actionNum))
			continue;
	
		DEBUG_MESSAGE("compara linkage data ok actionnum = %d\n", actionNum);

		linkNumber = (unsigned char *)malloc(sizeof(unsigned char));
		*linkNumber = actionNum;

		if ((ALARM_FULL_TIME == alarmSwitch) || 
			(AKEY_DEFENSE_ON == getAkeyDefense() && ALARM_FUNC_ON == alarmSwitch) || 
			(ALARM_SENSOR_TYPE == sensorInfo->type))
		{
			if (linkSwitch) 
				pool_add_worker(performActionPrc, linkNumber);
			else
				pool_add_worker(performActionPrc3, linkNumber);
		}
		else
		{	
			if (linkSwitch)
				pool_add_worker(performActionPrc2, linkNumber);
			else
				wrt_free_memory(linkNumber);
		}
		sleep(1);
	}

	return 0;
}


/******************************************************************************/
/*
 *	compara_sensor_linkage_data
 */
//======================================================
//** 函数名称: compara_sensor_linkage_data
//** 功能描述: 比较传感器联动数据
//** 输　入: actionNum
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int compara_sensor_linkage_data(unsigned char actionNum)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	SENSOR_INFO sensorInfo;
	unsigned int linkCondition;
	
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select id, type, linkpara, linkcond from linkageRule where actionNum = %d", actionNum);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "select linkageRule data error");

	for (index = 4; index < (sqlInfo.row + 1) * sqlInfo.col; index += 4)
	{
		if (checkSqlData(sqlInfo.result, index, 4))
		{
			DEBUG_ERROR("null result");
			continue;
		}
		
		sensorInfo.id = atoi(sqlInfo.result[index + 0]);
		sensorInfo.type = atoi(sqlInfo.result[index + 1]);
		sensorInfo.para1 = atoi(sqlInfo.result[index + 2]);
		linkCondition = atoi(sqlInfo.result[index + 3]);
		
		if (query_sensor_sqlite_data(linkCondition, &sensorInfo))
		{
			DEBUG_ERROR("query_sensor_sqlite_data fail actionnum = %d\n", actionNum);
			return -1;
		}								
	}	

	return 0;
}

//======================================================
//** 函数名称: compara_sensor_status_data
//** 功能描述: 比较传感器状态数据
//** 输　入: sensorInfo actionNum
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int compara_sensor_status_data(SENSOR_INFO *sensorInfo, unsigned char actionNum)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	ECB_INFO stateInfo;
	
	if (MULT_ENV_SENSOR_TYPE == sensorInfo->type ||
		PM_SENSOR_TYPE == sensorInfo->type ||
		MULT_ENV_HEART_SENSOR_TYPE == sensorInfo->type ||
		BODY_LIGHT_SENSOR_TYPE == sensorInfo->type)
	{	
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "select groupAddr, way, arg from performAction where actionnum = %d", actionNum);
		ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
		CHECK_RET(ret, "ecb_sqlite3_get_table error");
		for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
		{
			if (checkSqlData(sqlInfo.result, index, 3))
			{
				DEBUG_ERROR("null result");
				continue;
			}	
			ecbInfo.group = atoi(sqlInfo.result[index + 0]);
			ecbInfo.way   = atoi(sqlInfo.result[index + 1]);
			ecbInfo.para  = atoi(sqlInfo.result[index + 2]);
			if (ecbInfo.way == DEV_SCENE_CTRL)
				return 0;
			if (compare_sqlite_status_data(&ecbInfo) > 0)
				return 0;
		}
		return 1;
	}

	return 0;
}

//======================================================
//** 函数名称: compare_sqlite_status_data
//** 功能描述: 比较数据库状态数据
//** 输　入: info
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int compare_sqlite_status_data(ECB_INFO *info)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select way, arg from ecbState where groupAddr = %d", info->group);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_get_table error");
	for (index = 2; index < (sqlInfo.row + 1) * sqlInfo.col; index += 2)
	{
		if (checkSqlData(sqlInfo.result, index, 2))
		{
			DEBUG_ERROR("null result");
			continue;
		}
		ecbInfo.way  = atoi(sqlInfo.result[index + 0]);
		ecbInfo.para = atoi(sqlInfo.result[index + 1]);
		if (!(ecbInfo.way == info->way && ecbInfo.para == info->para))
			return 1;
	}

	return 0;
}

/******************************************************************************/
/*
 *	compara_timer_linkage_data
 */
//======================================================
//** 函数名称: compara_timer_linkage_data
//** 功能描述: 比较联动中定时任务数据
//** 输　入: actionNum
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int compara_timer_linkage_data(unsigned char actionNum)
{
	int count;
	SQL_INFO sqlInfo = {0};
	sqlite3_stmt *statment = NULL;

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select count(*) from timerList where actionNum = %d", actionNum);
	if (sqlite3_prepare(db, sqlInfo.sql, -1, &statment, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statment) == SQLITE_ROW)
			count = sqlite3_column_int(statment, 0);
	}
	sqlite3_finalize(statment);
	
	if (count > 0)
		return 1;

	return 0;
}


/******************************************************************************/
/*
 *	query_sensor_sqlite_data
 */
//======================================================
//** 函数名称: query_sensor_sqlite_data
//** 功能描述: 查询传感器数据库数据
//** 输　入: linkCond  linkInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int query_sensor_sqlite_data(unsigned int linkCond, SENSOR_INFO *linkInfo)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	SENSOR_INFO sensorInfo;
	unsigned char linkParaCond;
	unsigned char linkParaLen;
	unsigned char linkParaPos;
	unsigned int sensorPara;
	unsigned int isSatisfyCond;

	linkParaCond = (linkCond >> 12) & 0xF;
	linkParaLen  = (linkCond >> 8) & 0xF;
	linkParaPos  = (linkCond) & 0xFF;
	
	isSatisfyCond = -1;
	
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select para1, para2, para3, para4, para5 from sensorState where id = %d and type = %d", linkInfo->id, linkInfo->type);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_get_table error");
	
	for (index = 5; index < (sqlInfo.row + 1) * sqlInfo.col; index += 5)
	{
		if (checkSqlData(sqlInfo.result, index, 5))
		{
			DEBUG_ERROR("null result");
			continue;
		}
		
		sensorInfo.para1 = atoi(sqlInfo.result[index + 0]);
		sensorInfo.para2 = atoi(sqlInfo.result[index + 1]);
		sensorInfo.para3 = atoi(sqlInfo.result[index + 2]);
		sensorInfo.para4 = atoi(sqlInfo.result[index + 3]);
		sensorInfo.para5 = atoi(sqlInfo.result[index + 4]);

		if (linkParaCond < LINK_COND_BIT)
			sensorPara = getDataNaBytes((unsigned char *)&sensorInfo.para1, linkParaPos, linkParaLen);
		else
			sensorPara = sensorInfo.para1 & linkInfo->para1;
		
		switch (linkParaCond)
		{
			case LINK_COND_EQUAL:	
				if (sensorPara == linkInfo->para1)
					isSatisfyCond = 0;
				break;
				
			case LINK_COND_LESS:
				if (sensorPara < linkInfo->para1)
					isSatisfyCond = 0;
				break;
				
			case LINK_COND_MORE:
				if (sensorPara > linkInfo->para1)
					isSatisfyCond = 0;
				break;
				
			case LINK_COND_LESSEQUAL:
				if (sensorPara <= linkInfo->para1)
					isSatisfyCond = 0;
				break;
				
			case LINK_COND_MOREEQUAL:
				if (sensorPara >= linkInfo->para1)
					isSatisfyCond = 0;
				break;
				
			case LINK_COND_BIT:
				if (sensorPara == linkInfo->para1)
					isSatisfyCond = 0;
				break;
				
			default:
				DEBUG_ERROR("linkParaCond:%d error", linkParaCond);
				break;
		}
	}

	return isSatisfyCond;
}


/******************************************************************************/
/*
 *	Linkage processing thread, processing sensor data
 */
//======================================================
//** 函数名称: dealLinkageFunc
//** 功能描述: 联动线程
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
void *dealLinkageFunc(void *arg)
{
	uartPacket *pdata;
	SENSOR_INFO sensorInfo;
	
	pool_init(10);
	
	while (true)
	{ 
		data_t data = q_pop_head(g_queue_linkage, 2, 0);
		if (data.ptr != NULL)
		{
			pdata = (uartPacket *)data.ptr;
			update_sensor_status_data(pdata, &sensorInfo);
			query_sensor_linkage_data(&sensorInfo);
			wrt_free_memory(pdata->buf);
			wrt_free_memory(pdata);
		}
	}
	
	pool_destroy();

}

/******************************************************************************/
/*
 *	select_linkage_action_run
 */
//======================================================
//** 函数名称: select_linkage_action_run
//** 功能描述: 挑选联动动作执行
//** 输　入: actionNum number
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int select_linkage_action_run(unsigned char actionNum, int number)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	unsigned short delayTime;
	unsigned char uartSndData[SER_MAX_LENGTH];

	sprintf(sqlInfo.sql, "select groupAddr, way, arg, sleep from performAction where actionNum = %d and ord = %d", actionNum, number + 1);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "select data from performAction error\n");

	for (index = 4; index < (sqlInfo.row + 1) * sqlInfo.col; index += 4)
	{
		if (checkSqlData(sqlInfo.result, index, 4) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}

		usleep(500*1000);
		memset(&ecbInfo, 0, sizeof(ECB_INFO));
		ecbInfo.group = atoi(sqlInfo.result[index + 0]);
		ecbInfo.way   = atoi(sqlInfo.result[index + 1]);
		ecbInfo.para  = atoi(sqlInfo.result[index + 2]);
		delayTime	  = atoi(sqlInfo.result[index + 3]);
		sleep(delayTime);

		if (!division_ctrlway_func(ecbInfo.way, ecbInfo.para))
			continue;

		fill_data_msg(uartSndData, ECB_CTRL_DEVICE, &ecbInfo);
		SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
	}	

	return 0;
}


/******************************************************************************/
/*
 *	Device control way processing
 */
//======================================================
//** 函数名称: performActionPrc3
//** 功能描述: 联动执行3
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int division_ctrlway_func(unsigned char way, unsigned short para)
{
	int ret;
	
	if (DEFENSE_STATUS == way)
	{
		if (1 == para)
			setAkeyDefense(AKEY_DEFENSE_ON);
		else 
			setAkeyDefense(AKEY_DEFENSE_OFF);	

		ret = compose_packet_send_app(NET_SET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "NET_SET_DEFENSE_FLAG_ACK");

		ret = compose_packet_send_app(EXT_GET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "EXT_GET_DEFENSE_FLAG_ACK");		
		return 0;
	}

	return -1;
}

/******************************************************************************/
/*
 *	Linkage processing, with push alarm
 */
//======================================================
//** 函数名称: performActionPrc1
//** 功能描述: 联动执行1
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
void *performActionPrc(void *arg)
{
	int index;
	int count;
	SQL_INFO sqlInfo = {0};
	unsigned char actionNum;
	sqlite3_stmt *statment = NULL;
	
	actionNum = *(unsigned char *)arg;
	wrt_free_memory(arg);

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select count(*) from performAction where actionNum = %d", actionNum);
	if (sqlite3_prepare(db, sqlInfo.sql, -1, &statment, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statment) == SQLITE_ROW)
			count = sqlite3_column_int(statment, 0);
	}
	sqlite3_finalize(statment);
	
	pthread_mutex_lock(&g_ctrlMutex);
	for (index = 0; index < count; index++)
		select_linkage_action_run(actionNum, index);
	pthread_mutex_unlock(&g_ctrlMutex);
	
	push_alarm_message(actionNum);

	return NULL;
}

/******************************************************************************/
/*
 *	Linkage processing, with push alarm
 */
//======================================================
//** 函数名称: performActionPrc2
//** 功能描述: 联动执行2
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
void *performActionPrc2(void *arg)
{
	int ret;
	int index;
	int count;
	SQL_INFO sqlInfo = {0};
	unsigned char actionNum;
	sqlite3_stmt *statment = NULL;
	
	actionNum = *(unsigned char *)arg;
	wrt_free_memory(arg);

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select count(*) from performAction where actionNum = %d", actionNum);
	if (sqlite3_prepare(db, sqlInfo.sql, -1, &statment, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statment) == SQLITE_ROW)
			count = sqlite3_column_int(statment, 0);
	}
	sqlite3_finalize(statment);
	
	pthread_mutex_lock(&g_ctrlMutex);
	for (index = 0; index < count; index++)
		select_linkage_action_run(actionNum, index);
	pthread_mutex_unlock(&g_ctrlMutex);

	return NULL;
}

/******************************************************************************/
/*
 *	Linkage processing, with push alarm
 */
//======================================================
//** 函数名称: performActionPrc3
//** 功能描述: 联动执行3
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================   
void *performActionPrc3(void *arg)
{
	unsigned char actionNum;
	
	actionNum = *(unsigned char *)arg;
	wrt_free_memory(arg);

	push_alarm_message(actionNum);

	return NULL;
}

/******************************************************************************/
/*
 *	push_alarm_message
 */
//======================================================
//** 函数名称: push_alarm_message
//** 功能描述: APP上报告警
//** 输　入: actionNum
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int push_alarm_message(unsigned char actionNum)
{
	int index;
	time_t tm;
	SQL_INFO sqlInfo;
	GW_POST_INFO mes;
	char *tmp = NULL;
	sqlite3_stmt *stat = NULL;	
	char t[BUFF_SIZE] = "abcdefghijklmnopqrstuvwxyz1234567890";
	
	memset(&mes, 0, sizeof(GW_POST_INFO));
	srand((unsigned int)time(NULL) + (unsigned int)uartGetSeqnumUp());
	index = rand();
	for(index = 0; index < sizeof(mes.uuid) - 1; index++)
		mes.uuid[index] = t[rand()%strlen(t)];
	
	sprintf(mes.id, "%02d%08x", GW_ID_TYPE, g_mcuId);	
	mes.eventType = 1;
	
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select nm from actionName where actionNum = %d", actionNum);
	if(sqlite3_prepare(db, sqlInfo.sql, -1, &stat, NULL) == SQLITE_OK)
	{
		while(sqlite3_step(stat) == SQLITE_ROW)
		{
			tmp = (char *)sqlite3_column_text(stat, 0);
			memcpy(mes.eventName, tmp, sizeof(mes.eventName));
		}
	}
	sqlite3_finalize(stat);

	time(&tm);
	sprintf(mes.time, "%010d%03d", tm, 1);
	memcpy(mes.time + 10, "000", 4);

	json_post_message(&mes);

	return 0;
}

/******************************************************************************/
/*
 *	ext push_alarm_message
 */
//======================================================
//** 函数名称: ext_push_alarm_message
//** 功能描述: 分机上报告警
//** 输　入: eventName
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int ext_push_alarm_message(char *eventName)
{
	int ret;
	int index;
	time_t tm;
	GW_POST_INFO mes;
	char t[BUFF_SIZE] = "abcdefghijklmnopqrstuvwxyz1234567890";
	
	memset(&mes, 0, sizeof(GW_POST_INFO));
	srand((unsigned int)time(NULL) + (unsigned int)uartGetSeqnumUp());
	index = rand();
	for(index = 0; index < sizeof(mes.uuid) - 1; index++)
		mes.uuid[index] = t[rand()%strlen(t)];
	
	sprintf(mes.id, "%02d%08x", GW_ID_TYPE, g_mcuId);	
	mes.eventType = 1;
	
	memcpy(mes.eventName, eventName, 40);
	time(&tm);
	sprintf(mes.time, "%010d%03d", tm, 1);
	memcpy(mes.time + 10, "000", 4);

	json_post_message(&mes);

	return 0;
}

/******************************************************************************/
/*
 *	dealTimerFunc
 */
//======================================================
//** 函数名称: dealTimerFunc
//** 功能描述: 定时线程
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================   
void *dealTimerFunc(void *arg)
{
	time_t timeTmp;
	struct tm *curTime;
	int hour = 0;
	int min = 0;
	int count = 3;
	
	while (true)
	{
		time(&timeTmp);
		curTime = localtime(&timeTmp);
		if (!((curTime->tm_hour == hour) && (curTime->tm_min == min)))
		{
			hour = curTime->tm_hour;
			min  = curTime->tm_min;
			getTimeInfoFromTimer(curTime);
		}
		//每天00:00:30设备时间同步(不属于联动定时部分，用于液晶面板等需要时间显示设备)
		if ((curTime->tm_hour == 0) && (curTime->tm_min == 0) && (count-- > 0))
				sendDevtimeTimer();
		if ((curTime->tm_hour == 0) && (curTime->tm_min == 1))
			count = 3;

		sleep(1);
	}

	return NULL;
}

/******************************************************************************/
/*
 *	getTimeInfoFromTimer
 */
//======================================================
//** 函数名称: getTimeInfoFromTimer
//** 功能描述: 获取定时任务信息
//** 输　入: taskTime
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================   
int getTimeInfoFromTimer(struct tm *taskTime)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	TIMER_INFO timerInfo;
	unsigned char timebuf[8];

	taskTime->tm_year += 1900; 
	taskTime->tm_mon += 1;

	//获取定时任务时间 -- 当前时间
	timebuf[0] = ((unsigned char *)&(taskTime->tm_year))[1];
	timebuf[1] = ((unsigned char *)&(taskTime->tm_year))[0];
	timebuf[2] = ((unsigned char *)&(taskTime->tm_mon))[0];
	timebuf[3] = ((unsigned char *)&(taskTime->tm_mday))[0];
	timebuf[4] = ((unsigned char *)&(taskTime->tm_hour))[0];
	timebuf[5] = ((unsigned char *)&(taskTime->tm_min))[0];
	timebuf[6] = ((unsigned char *)&(taskTime->tm_sec))[0];
	timebuf[7] = ((unsigned char *)&(taskTime->tm_wday))[0];

	//获取定时任务时间 -- 联动时间
	ret = ecb_sqlite3_get_table(db, "select * from timerList", &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret,"ecb_sqlite3_get_table error");
	
	for (index = 8; index < (sqlInfo.row + 1) * sqlInfo.col; index += 8)
	{
		if (checkSqlData(sqlInfo.result, index, 8))
		{
			printf("null result");
			continue;
		}

		memset(&timerInfo, 0, sizeof(timerInfo));
		timerInfo.actionNum = atoi(sqlInfo.result[index + 0]);
		timerInfo.timerYear = atoi(sqlInfo.result[index + 1]);
		timerInfo.timerTime = atoi(sqlInfo.result[index + 2]);
		timerInfo.ctrlWay = atoi(sqlInfo.result[index + 3]);
		timerInfo.para1 = atoi(sqlInfo.result[index + 4]);
		timerInfo.para2 = atoi(sqlInfo.result[index + 5]);
		timerInfo.linkSwitch = atoi(sqlInfo.result[index + 6]);
		timerInfo.defSwitch = atoi(sqlInfo.result[index + 7]);
		
		timerInfo.timebuf[0] = ((unsigned char *)&(timerInfo.timerYear))[3];
		timerInfo.timebuf[1] = ((unsigned char *)&(timerInfo.timerYear))[2];
		timerInfo.timebuf[2] = ((unsigned char *)&(timerInfo.timerYear))[1];
		timerInfo.timebuf[3] = ((unsigned char *)&(timerInfo.timerYear))[0];
		timerInfo.timebuf[4] = ((unsigned char *)&(timerInfo.timerTime))[3];
		timerInfo.timebuf[5] = ((unsigned char *)&(timerInfo.timerTime))[2];
		timerInfo.timebuf[6] = ((unsigned char *)&(timerInfo.timerTime))[1];

		printf("linknumber %d timerTime: %04d-%02d-%02d %02d:%02d:%02d\n", 
			timerInfo.actionNum, timerInfo.timebuf[0] << 8 | timerInfo.timebuf[1], timerInfo.timebuf[2], timerInfo.timebuf[3], 
			timerInfo.timebuf[4], timerInfo.timebuf[5], timerInfo.timebuf[6]);
		printf("linknumber %d curTime: %04d-%02d-%02d %02d:%02d:%02d\n", 
			timerInfo.actionNum, timebuf[0] << 8 | timebuf[1], timebuf[2], timebuf[3], timebuf[4], timebuf[5], timebuf[6]);
		
		if (timerInfoFromTimerActionPrc(&timerInfo, timebuf))
			continue;
		//printf("timerTime: actionNum = %d ok!\n", timerInfo.actionNum);
	}

	return 0;
}

/******************************************************************************/
/*
 *	timerInfoFromTimerActionPrc
 */
//======================================================
//** 函数名称: timerInfoFromTimerActionPrc
//** 功能描述: 定时信息根据定时动作处理
//** 输　入: timerInfo timebuf
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================   
int timerInfoFromTimerActionPrc(TIMER_INFO *timerInfo, unsigned char *timebuf)
{
	int index;

	/* 日期比对 (年,月,日,时,分)--- 判断当前日期是否满足定时任务 */
	for (index = 0; index < 6; index++)
	{
		if (index < 4)
		{
			if (*(timerInfo->timebuf + index) &&
				(*(timerInfo->timebuf + index) != *(timebuf + index)))
			{
				printf("timerTime difference index:%d\n", index);
				return -1;
			}
		}
		else
		{
			if (*(timerInfo->timebuf + index) != *(timebuf + index))
			{
				printf("timerTime difference index:%d\n", index);
				return -1;
			}
		}
	}

	/* 日期年比对 --- 判断年是否相同 */
	if ((*(timerInfo->timebuf) << 8) | *(timerInfo->timebuf + 1))
	{
		if (!((*(timerInfo->timebuf) == *timebuf) && (*(timerInfo->timebuf + 1) == *(timebuf + 1))))
		{
			printf("timerTime year difference\n");
			return -1;
		}
	}
	
	/* 定时联动开关关闭，布/撤防开关不满足 --- 删除单次定时任务 */
	if ((!timerInfo->linkSwitch) /*|| ((timerInfo->defSwitch != 2) && (timerInfo->defSwitch == ecbGetDefenseSwitch()))*/)
	{
		if (timerInfo->ctrlWay == SINGLE_TIMER)
		{
			if (0 == deleteTimerFormAction(timerInfo->actionNum))
				printf("deleteTimerFormAction ok\n");
		}
		printf("timer no pass linkSwitch:%d defswitch:%d\n", timerInfo->linkSwitch, timerInfo->defSwitch);
		return -1;
	}

	/* 单次定时任务 -- 定时任务执行一次后删除该条定时任务 */
	if (timerInfo->ctrlWay == SINGLE_TIMER)
	{
		if (0 == (timerInfo->para1 & 0x7F))//bit 0 -- bit 6 标识星期天 -- 星期六
		{
			/* 日期定时任务 --- 星期参数为0时表示日期定时任务 */
			if (0 == timerTaskLinkActionPrc(timerInfo->actionNum))
				printf("timerTaskLinkActionPrc ok\n");
			if (0 == deleteTimerFormAction(timerInfo->actionNum))
				printf("deleteTimerFormAction ok\n");
		}
		else
		{
			/* 星期定时任务，星期比对 --- 判断当前星期是否满足定时任务 */
			if ((timerInfo->para1 >> timebuf[7]) & 0x01)
			{
				if (0 == timerTaskLinkActionPrc(timerInfo->actionNum))
					printf("timerTaskLinkActionPrc ok\n");
				if (0 == deleteTimerFormAction(timerInfo->actionNum))
					printf("deleteTimerFormAction ok\n");
			}
		}
	}
	/* 重复定时任务 -- 定时任务执行一次后不删除该条定时任务 */
	else if (timerInfo->ctrlWay == REPEAT_TIMER)
	{
		if (0 == (timerInfo->para1 & 0x7F))//bit 0 -- bit 6 标识星期天 -- 星期六
		{
			/* 日期定时任务 --- 星期参数为0时表示日期定时任务 */
			if (0 == timerTaskLinkActionPrc(timerInfo->actionNum))
				printf("timerTaskLinkActionPrc ok\n");
		}
		else
		{
			/* 星期定时任务，星期比对 --- 判断当前星期是否满足定时任务 */
			if ((timerInfo->para1 >> timebuf[7]) & 0x01)
			{
				if (0 == timerTaskLinkActionPrc(timerInfo->actionNum))
					printf("timerTaskLinkActionPrc ok\n");
			}
		}
	}
	
	return 0;
}

/******************************************************************************/
/*
 *	timerTaskLinkActionPrc
 */
//======================================================
//** 函数名称: timerTaskLinkActionPrc
//** 功能描述: 定时任务联动处理
//** 输　入: actionNum
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================   
int timerTaskLinkActionPrc(unsigned char actionNum)
{
	int ret;
	int index;
	int linkSwitch;
	int alarmSwitch;
	SQL_INFO sqlInfo;
	unsigned char *linkNumber;

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select linkswitch, defswitch from timerList where actionNum =  %d", actionNum);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_get_table error");

	for (index = 2; index < (sqlInfo.row + 1) * sqlInfo.col; index += 2)
	{
		if(checkSqlData(sqlInfo.result, index, 2))
		{
			DEBUG_ERROR("null result");
			continue;
		}	
	
		linkSwitch  = atoi(sqlInfo.result[index + 0]);
		alarmSwitch = atoi(sqlInfo.result[index + 1]);	
	}

	if (compara_sensor_linkage_data(actionNum))
	{
		DEBUG_MESSAGE("compara_sensor_linkage_data fail actionnum = %d\n", actionNum);
		return -1;
	}
	DEBUG_MESSAGE("timerTask linkage data ok actionnum = %d\n", actionNum);
	
	linkNumber = (unsigned char *)malloc(sizeof(unsigned char));
	*linkNumber = actionNum;
	
	if ((ALARM_FULL_TIME == alarmSwitch) || (AKEY_DEFENSE_ON == getAkeyDefense() && ALARM_FUNC_ON == alarmSwitch) )
	{
		if (linkSwitch)	
			pool_add_worker(performActionPrc, linkNumber);
		else
			pool_add_worker(performActionPrc3, linkNumber);
	}
	else
	{	
		if (linkSwitch)
			pool_add_worker(performActionPrc2, linkNumber);
		else
			wrt_free_memory(linkNumber);
	}	
	
	return 0;
}

/******************************************************************************/
/*
 *	deleteTimerFormAction
 */
//======================================================
//** 函数名称: deleteTimerFormAction
//** 功能描述: 删除定时动作
//** 输　入: number
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================   
int deleteTimerFormAction(unsigned char number)
{
	int ret;
	SQL_INFO sqlInfo;
	/* 删除指定联动编号定时任务 */
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "delete from timerList where actionnum = %d", number);
	ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");
	return 0;
}

/******************************************************************************/
/*
 *	Assembly data package
 */
//======================================================
//** 函数名称: composeSendPacket
//** 功能描述: 封装数据包 
//** 输　入: response sendbuf send_len rc4_md5_flag
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int composeSendPacket(const netResponse *response, char **sendbuf, int *send_len, bool rc4_md5_flag)
{
	netResponse net_response;
	unsigned char md5[32];
	memset( md5, 0, sizeof(md5) );

	memcpy( &net_response, response, sizeof(netResponse) );
	char *packet = NULL;
	packet = (char *)malloc(40+(net_response.bufLen)+16);
	if( NULL == packet )
	{
		*sendbuf = NULL;
		return -1;
	}
	memset( packet, 0, 40+(net_response.bufLen)+16 );
	strncpy( packet, "WRTI", 4 );

	*(unsigned short *)(packet+4) = htons(net_response.cmd);

	int packetLen = 0;
	int temp_len = 40+(net_response.bufLen)+16;

	char *p_len = (char *)&temp_len;

	packet[6] = p_len[0];
	packet[7] = p_len[1];
	packet[8] = p_len[2];
	packet[9] = p_len[3];

	memcpy( &packetLen, &packet[6], sizeof(int) );

	int index = 0;
	for (index = 0; index < net_response.bufLen; index++)
	{
		packet[40+index] = net_response.buf[index];
	}

	if (rc4_md5_flag)
	{
		if (GenerateMD5( packet, packetLen-16, md5 ) < 0)
		{
			free(packet);
			packet = NULL;
			*sendbuf = NULL;
			return -2;
		}
		for (index=0; index<16; index++)
		{
			packet[40+index+net_response.bufLen] = md5[index];
		}

		memcpy(&packetLen, packet+6, sizeof(packetLen));

		int temp_len = 40+(net_response.bufLen)+16;

		char *p_len = (char *)&temp_len;

		packet[6] = p_len[0];
		packet[7] = p_len[1];
		packet[8] = p_len[2];
		packet[9] = p_len[3];

		char variableKey[17];
		memcpy( variableKey, pSystemInfo->DoorSysInfo.variableKey, 16 );
		variableKey[16] = '\0';

		char *out_packet = NULL;
		out_packet = (char *)malloc( 40+(net_response.bufLen)+16 );
		if( NULL == out_packet )
		{
			free(packet);
			packet = NULL;
			*sendbuf = NULL;
			return -1;
		}

		printf("\nsend app data:\n");
		for (int i=0; i < 40+(net_response.bufLen)+16; i++)
			printf("%02x ", (unsigned char)packet[i]);
		printf("\n\n");

		if (WRTRC4Encrypt(variableKey, (unsigned char *)packet, packetLen, (unsigned char *)out_packet) < 0)
		{
			free(packet);
			free(out_packet);
			packet = NULL;
			out_packet = NULL;		
			*sendbuf = NULL;
			return -3;
		}			
		free(packet);
		*send_len = packetLen;
		*sendbuf = out_packet;
		return 0;
	}
	*send_len = packetLen;
	*sendbuf = packet;
	return 0;
}

/******************************************************************************/
/*
 *	Serial data callback sent to app
 */
//======================================================
//** 函数名称: uartDataToAppCallBack
//** 功能描述: 串口数据回调app函数
//** 输　入: response userData
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int uartDataToAppCallBack(netResponse *response, void *userData)
{
	if (NULL == response || NULL == userData)
	{
		return -1;
	}

	static int firstFlag = 0;
	char *sendbuf = NULL;
	netResponse net_response;

	USER_DATA user_data;
	memcpy( &user_data, userData, sizeof(user_data) );	
	memcpy( &net_response, response, sizeof(netResponse) );
	int send_len = 0;
	int ret = composeSendPacket(&net_response, &sendbuf, &send_len, true);
	if (!ret)
	{ 
/*
 *		Send all connected IP
 */
		SOCKET_PACKET3 packet;
		struct sockaddr_in sock_msg;
		memset(&sock_msg, 0, sizeof(sock_msg));
		unsigned short msgcmd =  net_response.cmd;

		if (!firstFlag)
		{
			packet.from.isSIP = NET_TRANSMIT_DATA_ACK;
			packet.from.chat_room_ptr = -1;
			firstFlag = 1;
		}
		else
		{
			packet.from.chat_room_ptr = user_data.internet.chatroomptr;
			memcpy( packet.from.remotename, user_data.internet.data, sizeof(packet.from.remotename) );
			packet.from.isSIP = user_data.isSIP;
		}
		packet.from.sock_msg = sock_msg;
		packet.validlen = send_len;
		packet.buf = (unsigned char *)sendbuf;

		WRT_MsgQueue_s msg;
		memset(&msg,0,sizeof(WRT_MsgQueue_s));
		msg.myType = MSG_NODE_TCP_SEND_ALL;
		memcpy(msg.myText, &packet, sizeof(SOCKET_PACKET3));
		if(NULL != WRTSockMsgQueue_g)
			WRTSockMsgQueue_g->msgSnd(&msg, sizeof(SOCKET_PACKET3));
	}
	else
	{
		DEBUG_MESSAGE("error ret:%d\n", ret);
	}
	
	return 0;
}

/******************************************************************************/
/*
 *	The gateway pushes messages to the server using JSON
 */
//======================================================
//** 函数名称: json_post_message
//** 功能描述: json推送消息
//** 输　入: post_params
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int json_post_message(GW_POST_INFO *post_params)
{	
	int ret = 0;
	int index = 0;
	char mobile[64] = {0};
	const char *find = NULL;

	if (0 == getWhileListCount())
	{
		DEBUG_WARNING("json post WhileList count is null\n");
		return 0;
	}
	
	json_object* payload = json_object_new_object();
	json_object* params_array = json_object_new_array();
	json_object* params = json_object_new_object();
	json_object* params_sub_array = json_object_new_array();
	json_object* params_sub = json_object_new_object();

	json_object_object_add(params,"uuid",json_object_new_string(post_params->uuid));
	json_object_object_add(params,"getwayID",json_object_new_string(post_params->id));
	json_object_object_add(params,"eventType",json_object_new_int(post_params->eventType));
	json_object_object_add(params,"eventName",json_object_new_string(post_params->eventName));
	json_object_object_add(params,"eventTime",json_object_new_string(post_params->time));
	DEBUG_MESSAGE("mobile count = %d\n", getWhileListCount());
	for (index = 0; index < getWhileListCount(); index++)
	{
		memset(mobile, 0, sizeof(mobile));
		find = strstr((char *)g_whiteList[index].mobile, "@");
		if (find)
		{
			sprintf(mobile, (char *)g_whiteList[index].mobile, strlen((char *)g_whiteList[index].mobile));
			DEBUG_MESSAGE("json email = %s\n", mobile);
		}
		else
		{
			dec2Hex(g_whiteList[index].mobile, mobile, 11);
			DEBUG_MESSAGE("json mobile = %s\n", mobile);
		}
		
		json_object_array_add( params_sub_array, json_object_new_string(mobile));
	}
	json_object_object_add(params, "phoneNumbers", params_sub_array);
	json_object_array_add(params_array, params);
	json_object_object_add(payload, "event", params_array);
	const char* p = json_object_to_json_string(payload);
	DEBUG_MESSAGE("%s\n", p);
	dstring_t result;
	dstr_init(&result, 1024);
	ret = json_post("http://smart.wrtrd.net:8080/homeServer/event/sendEvents", p, strlen(p), &result);

	json_object_put(payload);
	
	dstr_destroy(&result);
	DEBUG_MESSAGE("json post message ok!!\n");
	
	return 0;
}

/******************************************************************************/
/*
 *   Send data to Uart
 */
//======================================================
//** 函数名称: SendDataToUart
//** 功能描述: 发送数据到串口
//** 输　入: str length
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================   
int SendDataToUart(unsigned char *str, unsigned char length)
{
	int i;
	unsigned char chksum = 0;
	
	printf("\n%04x : ", (str[7] << 8 | str[8]));
	for(i = 0; i < length; i++)
		printf("%02x ", (unsigned char)*(str + i));
	printf("\n");
	
	int write_len = uartwrite(serial_fd, str, length);	
	//printf("write uart len: %d\n", write_len);	

	return write_len;
}

/******************************************************************************/
/*
 *   Serial data comparison filtering, the same data is not processed
 */
//======================================================
//** 函数名称: uart_compare_data_packet
//** 功能描述: 串口比较数据包
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int uart_compare_data_packet(uartPacket *packet)
{
	int i;
	long time;	
	unsigned short cmd;
	static uartPacket tmpPacket;
	static unsigned char tmpBuf[BUFF_SIZE] = {0};
	
	cmd = packet->cmd;
			
	if (tmpPacket.cmd == cmd && tmpPacket.bufLen == packet->bufLen)
	{
		for (i = 0; i < packet->bufLen; i++)
		{
			if (tmpPacket.buf[i] == packet->buf[i])
			{
				continue;
			}
			else
			{
				tmpPacket.cmd = cmd;
				tmpPacket.bufLen = packet->bufLen;
				memset(tmpBuf, 0, sizeof(tmpBuf));
				memcpy(tmpBuf, packet->buf, packet->bufLen);
				tmpPacket.buf = tmpBuf;
				return 0;
			}
		}
		
		DEBUG_WARNING("recv data packet same, no deal");
		return -1;	
	}
	else
	{
		tmpPacket.cmd = cmd;
		tmpPacket.bufLen = packet->bufLen;
		memset(tmpBuf, 0, sizeof(tmpBuf));
		memcpy(tmpBuf, packet->buf, packet->bufLen);
		tmpPacket.buf = tmpBuf;
		return 0;
	}
	
	return 0;
}

/******************************************************************************/
/*
 *   Encapsulated message data frame, In accordance with the WRT protocol format
 */
//======================================================
//** 函数名称: fill_data_msg
//** 功能描述: 封装下发的数据帧格式
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
void fill_data_msg(unsigned char *uartSndData, unsigned short cmd, ECB_INFO *ecbInfo)
{
/*
 *   format: 5A + A5 + len(1B) + src(2B) + dest(2B) + cmd(2B) + para(nB) + check(1B)
 */
	memset(uartSndData, 0, SER_MAX_LENGTH);
	uartSndData[SER_DATA_HEAD0] = 0x5A;
	uartSndData[SER_DATA_HEAD1] = 0xA5;
	uartSndData[SER_DATA_SRCH]  = 0xFF;
	uartSndData[SER_DATA_SRCL]  = 0xFF;

	switch (cmd)
	{
		case ECB_CTRL_DEVICE:
			
		{
			uartSndData[SER_DATA_DESTH] = ((unsigned char*)&ecbInfo->group)[1];
			uartSndData[SER_DATA_DESTL] = ((unsigned char*)&ecbInfo->group)[0];
			uartSndData[SER_DATA_CMDH]	= 0x02;
			uartSndData[SER_DATA_CMDL]	= 0x01; 
			uartSndData[SER_DATA_LENGTH]= 6+4;	
			uartSndData[SER_DATA_PARA0] = ((unsigned char*)&ecbInfo->way)[0];
			uartSndData[SER_DATA_PARA1] = ((unsigned char*)&ecbInfo->para)[1];
			uartSndData[SER_DATA_PARA2] = ((unsigned char*)&ecbInfo->para)[0];
			break;
		}

		case ECB_SEARCH_DEVICE:
		{
			uartSndData[SER_DATA_DESTH] = 0xFF;
			uartSndData[SER_DATA_DESTL] = 0xFF;
			uartSndData[SER_DATA_CMDH]	= 0x01;
			uartSndData[SER_DATA_CMDL]	= 0x01; 
			uartSndData[SER_DATA_LENGTH]= 6+2;
			uartSndData[SER_DATA_PARA0] = ecbInfo->way;
			break;
		}		
		
		case ECB_DEV_STATUS_GROUP_QUERY:
		{
			uartSndData[SER_DATA_DESTH] = 0xFF;
			uartSndData[SER_DATA_DESTL] = 0xFF;
			uartSndData[SER_DATA_CMDH]	= 0x03;
			uartSndData[SER_DATA_CMDL]	= 0x80;	
			uartSndData[SER_DATA_LENGTH]= 6+4;
			uartSndData[SER_DATA_PARA0]	= 0x01;	
			uartSndData[SER_DATA_PARA1]	= ((unsigned char*)&ecbInfo->group)[1];	
			uartSndData[SER_DATA_PARA2]	= ((unsigned char*)&ecbInfo->group)[0];
			break;
		}
		
		case ECB_AIR_COND_STATUS_QUERY:
		{
			uartSndData[SER_DATA_DESTH] = 0xFF;
			uartSndData[SER_DATA_DESTL] = 0xFF;
			uartSndData[SER_DATA_CMDH]	= 0x03;
			uartSndData[SER_DATA_CMDL]	= 0x82;	
			uartSndData[SER_DATA_LENGTH]= 6+4;	
			uartSndData[SER_DATA_PARA0] = 0x01;
			uartSndData[SER_DATA_PARA1] = ((unsigned char*)&ecbInfo->group)[1];
			uartSndData[SER_DATA_PARA2]	= ((unsigned char*)&ecbInfo->group)[0];
			break;
		}
		
		case ECB_DEV_DATA_CLEAR:
		{
			uartSndData[SER_DATA_DESTH] = 0xFF;
			uartSndData[SER_DATA_DESTL] = 0xFF;
			uartSndData[SER_DATA_CMDH]	= 0x0A;
			uartSndData[SER_DATA_CMDL]	= 0x01;	
			uartSndData[SER_DATA_LENGTH]= 6+1;
			break;
		}
				
		default:
			DEBUG_ERROR("sendDataMsg cmd not recognized!\n");
			break;
	}
	
	uartCheck(&uartSndData[SER_DATA_LENGTH]);
}

/******************************************************************************/
/*
 *  Empty device configuration information
 */
//======================================================
//** 函数名称: dev_config_data_clear
//** 功能描述: 设备配置数据清除
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int dev_config_data_clear()
{
	int times = 3;
	data_t queue_data;
	unsigned short recvCmd;
	
	unsigned char uartSndData[SER_MAX_LENGTH];
	fill_data_msg(uartSndData, ECB_DEV_DATA_CLEAR, NULL);

	chearQueueData();

	while (times--)
	{	
		SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
		usleep(1000 * 1000);
		queue_data = q_pop_head(g_queue_ctrlAck, 0, 150);
		if (NULL != queue_data.ptr)
		{	
			recvCmd = ((uartPacket*)(queue_data.ptr))->cmd;
			wrt_free_memory(((uartPacket*)(queue_data.ptr))->buf);
			wrt_free_memory((uartPacket*)(queue_data.ptr));	
			if (ECB_DEV_DATA_CLEAR_ACK == recvCmd)/* Receive data ack */		
				return 0;
		}
	}	

	return -1;
}

//======================================================
//** 函数名称: keep_data_to_list
//** 功能描述: 添加数据到下发队列中
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
void keep_data_to_list(netPacket *packet)
{
	int pos = 0;
	unsigned char *data = NULL;

	if (packet->buf == NULL)
		return;
		
	while (true)
	{
		if (packet->buf[pos] == UART_DATA_HEAD1 && packet->buf[pos + 1] == UART_DATA_HEAD2)
		{
			data = (unsigned char *)malloc(packet->buf[pos+2] + 3);
			if (NULL == data)
				continue;
			memset(data, 0, sizeof(data));
			memcpy(data, &(packet->buf[pos]), packet->buf[pos+2] + 3);	
			ptr_vector_add(&g_netCmd, data);
		}
		pos++;	
		if (pos >= packet->bufLen)
			break;
	}
}

//======================================================
//** 函数名称: send_config_data
//** 功能描述: 发送配置数据
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int send_config_data()
{
	int ret;
	int index;
	int times;
	int size;
	SQL_INFO sqlInfo;
	DEV_INFO devInfo;
	DEV_INFO devInfoAck;
	data_t data = {0};
	unsigned short cmd;
	unsigned char *data_ptr = NULL;
	
	size = ptr_vector_size(&g_netCmd);
	dstr_append(&g_faillist, "\x00", 1);
		
	sleep(3);
	chearQueueData();
	
	for (index = 0; index < size; index++)
	{ 
/* 
 *       Calculates the percentage of down configuration
 */		
		g_percent = (unsigned short)((index * 100) / size);

		compose_packet_send_app(NET_CAL_PERCENT_DATA_ACK);

		memset(&devInfo, 0 ,sizeof(DEV_INFO));
		data_ptr = (unsigned char *)ptr_vector_get(&g_netCmd, index);
		if (NULL == data_ptr)/* Make sure the data is not empty */
			continue;
		
		devInfo.cmd = getDataNaBytes(data_ptr, 7, 2);
		/* cmd 0105 No numbering, special handling */
		if (ECB_DEV_CONFIG_DATA == devInfo.cmd)
		{
			devInfo.addr = getDataNaBytes(data_ptr, 5, 2);
			devInfo.devId = getDataNaBytes(data_ptr, 9, 4);
			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			sprintf(sqlInfo.sql, "update devInfo set dest = %d where id = %d", devInfo.addr, devInfo.devId);		
			ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
			CHECK_RET(ret, "update devInfo set dest error");
		}
		else
		{
			devInfo.addr = getDataNaBytes(data_ptr, 5, 2);
			devInfo.chan = getDataNaBytes(data_ptr, 9, 1);
		}
		
		times = 3;				
		while (times)
		{
			usleep(1000*200);
			SendDataToUart(data_ptr, data_ptr[SER_DATA_LENGTH] + 3);
			usleep(1000*800);
			data = q_pop_head(g_queue_ctrlAck, 0, 200);	
			
			if (NULL == data.ptr)
			{
				if (--times <= 0)
				{
					dstr_append(&g_faillist, (char *)data_ptr, data_ptr[2] + 3);
					break;
				}			
			}
			else 
			{
				devInfoAck.addr = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 3, 2);
				devInfoAck.chan = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 9, 1);
				devInfoAck.cmd = ((uartPacket*)(data.ptr))->cmd;
				wrt_free_memory(((uartPacket*)(data.ptr))->buf);
				wrt_free_memory(data.ptr);	
				
				if (ECB_DEV_CONFIG_DATA_ACK == devInfoAck.cmd)
				{
					if (devInfo.addr == devInfoAck.addr)
						break;
					times--;
					continue;
				}
				else
				{	
					if (devInfo.addr == devInfoAck.addr && devInfo.chan == devInfoAck.chan)			
						break;
					times--;
					continue;
				}			
			}
		}
	}

	for (index = 0; index < size; index++)
	{
		data_ptr = (unsigned char *)ptr_vector_get(&g_netCmd, index);
		wrt_free_memory(data_ptr);
	}
	return 0;
}

//======================================================
//** 函数名称: compose_packet_send_app
//** 功能描述: 封装数据发送至APP
//** 输　入: cmd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int compose_packet_send_app(unsigned short cmd)
{
	int ret;
	netResponse spn = {0};
	
	switch (cmd)
	{
		case NET_PUSH_CONFIG_DATA_ACK:
		case NET_PUSH_FAIL_CONFIG_DATA_ACK:	
		case NET_FAIL_CONFIG_DATA_QUERY_ACK:
		{		
			spn.flag = 0;	
			spn.cmd = cmd;
			spn.bufLen = dstr_get_data_length(&g_faillist);
			if (spn.bufLen > 0)
				spn.buf = (unsigned char *)malloc(dstr_get_data_length(&g_faillist) * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				if (spn.bufLen == 1)
					*spn.buf = 1;
				else
					dstr_get_data(&g_faillist, (char *)(spn.buf));
			}
			dstr_destroy(&g_faillist);
			dstr_init(&g_faillist, 100);	
			init_faillist_file(spn.buf, spn.bufLen);/* Refresh failed list file */
			break;
		}

		case NET_GROUP_STATUS_QUERY_ACK:
		case NET_GROUP_STATUS_QUERY_SINGLE_ACK:
		case NET_AIR_COND_STATUS_QUERY_ACK:	
		{
			spn.flag = 0;	
			spn.cmd = cmd;
			spn.bufLen = dstr_get_data_length(&g_group);
			if (spn.bufLen > 0)
				spn.buf = (unsigned char *)malloc(dstr_get_data_length(&g_group) * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				dstr_get_data(&g_group, (char *)(spn.buf));			
			}
			dstr_destroy(&g_group);
			dstr_init(&g_group, 100);	
			break;
		}

		case NET_SCENE_STATUS_QUERY_ACK:
		{
			spn.flag = 0;	
			spn.cmd = cmd;
			spn.bufLen = 2;
			spn.buf = (unsigned char *)malloc(2 * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				memcpy(spn.buf, g_scene, 2);	
			}
			break;
		}

		case NET_PUSH_LINKAGE_LIST_ACK:
		case NET_RESPLACE_GW_DATA_ACK:	
		{
			spn.flag = 0;	
			spn.cmd = cmd;
			spn.bufLen = 1;
			spn.buf = (unsigned char *)malloc(1 * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				spn.buf[0] = g_linkagenum;
			}
			break;
		}		

		case NET_GROUP_STATUS_SYNC_ACK:
		case NET_SCENE_STATUS_SYNC_ACK:
		case NET_SET_SENSOR_DATA_ACK:
		case NET_SET_LINKAGE_SWTICH_ACK:
		{
			spn.flag = 0;	
			spn.cmd = cmd;
			spn.bufLen = 1;
			spn.buf = (unsigned char *)malloc(1 * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				memcpy(spn.buf, "\x01", 1);	
			}
			break;
		}
		
		case NET_SET_DEFENSE_FLAG_ACK:
		{
			spn.flag = 0;	
			spn.cmd = cmd;
			spn.bufLen = 2;
			spn.buf = (unsigned char *)malloc(2 * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				spn.buf[0] = getAkeyDefense();
				spn.buf[1] = 1;
			}
			break;			
		}
		
		case EXT_GET_DEFENSE_FLAG_ACK:
		{
			spn.flag = 0;
			spn.cmd = cmd;	
			spn.bufLen = 4;
			spn.buf = (unsigned char *)malloc(4 * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				spn.buf[0] = 0;
				spn.buf[1] = 0;
				spn.buf[2] = 0;
				spn.buf[3] = (getAkeyDefense() == 0) ? 1 : 0;
			}
		}

		case NET_SEARCH_DEVICE_ACK:
		{
			spn.flag = 0;	
			spn.cmd = cmd;
			spn.bufLen = dstr_get_data_length(&g_devs);
			if (spn.bufLen > 0)
				spn.buf = (unsigned char *)malloc(dstr_get_data_length(&g_devs) * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				dstr_get_data(&g_devs, (char *)(spn.buf));			
			}
			dstr_destroy(&g_devs);
			dstr_init(&g_devs, 100);	
			break;	
		}
		
		case NET_CAL_PERCENT_DATA_ACK:
		{
			spn.flag = 0;	
			spn.cmd = cmd;
			spn.bufLen = 2;
			spn.buf = (unsigned char *)malloc(2 * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				spn.buf[0] = ((unsigned char *)&g_percent)[1];
				spn.buf[1] = ((unsigned char *)&g_percent)[0];				
			}
			break;
		}
		
		case NET_SENSOR_STATUS_REPORT_ACK:
		{
			spn.flag = 0;	
			spn.cmd = cmd;
			spn.bufLen = dstr_get_data_length(&g_statelist);
			if (spn.bufLen > 0)
				spn.buf = (unsigned char *)malloc(dstr_get_data_length(&g_statelist) * sizeof(unsigned char));
			if (NULL != spn.buf)
			{
				dstr_get_data(&g_statelist, (char *)(spn.buf));			
			}
			dstr_destroy(&g_statelist);
			dstr_init(&g_statelist, 100);
			break;
		}

		default:
			DEBUG_ERROR("cmd not recognized cmd:%04x", cmd);
			return -1;
	}

	if (NULL != g_packet.processResult)
	{
		ret = g_packet.processResult(&spn, &(g_packet.userData));
	}
	
	wrt_free_memory(spn.buf);
	usleep(100*1000);
	return ret;
}

//======================================================
//** 函数名称: init_faillist_file
//** 功能描述: 初始化失败列表文件
//** 输　入: str num
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int init_faillist_file(unsigned char *str, int num)
{
	FILE * fail_fd;
	
	if ((fail_fd = fopen(FAILLIST_FILE_PATH, "w+")) == NULL)
	{
		DEBUG_ERROR("open faillist file error\n");
		return -1;

	}
	
	if (fwrite(str, num, 1, fail_fd) < 0)
	{
		DEBUG_ERROR("write faillist file error\n");
		return -1;
	}
	
	fclose(fail_fd);
	
	return 0;
}

//======================================================
//** 函数名称: ecb_group_ctrl_map
//** 功能描述: 组地址控制映射
//** 输　入: way para
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int ecb_group_ctrl_map(unsigned char *way, unsigned short *para)
{
	if (NULL == way || NULL == para)
	{
		return -1;
	}
	unsigned short tmp;
	
	switch (*way)
	{
		case CUR_CTRL_ON:
			*way = DEV_STATUS_ON;
			*para = 0x00;
			break;
		case CUR_CTRL_OFF:
			*way = DEV_STATUS_OFF;
			*para = 0x00;
			break;
		default:
			break;
	}
	return 0;
}

//======================================================
//** 函数名称: dev_status_group_query
//** 功能描述: 设备状态组地址查询
//** 输　入: group
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int dev_status_group_query(unsigned short group)
{
	int ret;
	int times = 3;
	SQL_INFO sqlInfo;
	ECB_INFO netData;
	ECB_INFO ecbInfo;
	data_t queue_data;
	uartPacket* data_ptr = NULL;	
	unsigned short recvCmd;
	unsigned char uartSndData[SER_MAX_LENGTH];

	chearQueueData();

	DEBUG_MESSAGE("dev_status_group_query start!!\n");
	memset(&netData, 0, sizeof(netData));
	netData.group = group;
	fill_data_msg(uartSndData, ECB_DEV_STATUS_GROUP_QUERY, &netData);
	
	while (times--)
	{	
		SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
		usleep(500*1000);
		queue_data = q_pop_head(g_queue_ctrlAck, 0, 200);
		if (NULL != queue_data.ptr)
		{
			data_ptr = (uartPacket*)queue_data.ptr;
			recvCmd = data_ptr->cmd;
			ecbInfo.group = getDataNaBytes(data_ptr->buf, 10, 2);
			ecbInfo.way   = getDataNaBytes(data_ptr->buf, 12, 1);
			ecbInfo.para  = getDataNaBytes(data_ptr->buf, 13, 2);
			if (ECB_DEV_STATUS_GROUP_QUERY_ACK == recvCmd && group == ecbInfo.group)
			{
				memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
				sprintf(sqlInfo.sql, "update ecbState set way = %d, arg = %d where groupAddr = %d", ecbInfo.way, ecbInfo.para, ecbInfo.group); 
				ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
				CHECK_RET(ret, "ecb_sqlite3_exec error");
			}
			wrt_free_memory(data_ptr->buf);
			wrt_free_memory(data_ptr);
			if (ECB_DEV_STATUS_GROUP_QUERY_ACK == recvCmd)			
				return 0;
		}
	}

	return -1;
}

//======================================================
//** 函数名称: new_search_for_dev
//** 功能描述: 重新搜索设备
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int new_search_for_dev()
{
	int ret;
	int times;
	data_t data = {0};
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	unsigned char uartSndData[SER_MAX_LENGTH];

	unsigned short source = 0xffff;		 //src
	unsigned short dest;				 //dest
	unsigned char serialNum;			 //number
	unsigned int id;					 //dev id
	unsigned short type;				 //dev type
	unsigned short version; 			 //dev version
	
	ret = ecb_sqlite3_exec(db, "delete from devInfo", NULL, NULL, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");	

	ret = dev_config_data_clear();
	//CHECK_RET(ret, "dev_config_data_clear error");
	
	srand((int)time(NULL));
	serialNum = rand();
	times = 10;

	sleep(3);
	chearQueueData();
	
	while(times--)
	{
		ecbInfo.way = serialNum;
		fill_data_msg(uartSndData, ECB_SEARCH_DEVICE, &ecbInfo);
		SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
		sleep(1);
		data = q_pop_head(g_queue_ctrlAck, 0, 200);		
		if (NULL != data.ptr)
		{
			if (ECB_SEARCH_DEVICE_ACK == (((uartPacket*)(data.ptr))->cmd))
			{							
				dest = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 3, 2);
				id = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 10, 4);
				type = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 14, 2);
				version = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 16, 2);
				
				memset(uartSndData, 0, sizeof(uartSndData));
				uartSndData[SER_DATA_HEAD0] = 0x5A;
				uartSndData[SER_DATA_HEAD1] = 0xA5;
				uartSndData[SER_DATA_SRCH]	= 0xFF;
				uartSndData[SER_DATA_SRCL]	= 0xFF;
				uartSndData[SER_DATA_DESTH] = ((unsigned char *)(&dest))[1];
				uartSndData[SER_DATA_DESTL] = ((unsigned char *)(&dest))[0];
				uartSndData[SER_DATA_CMDH]	= 0x01;
				uartSndData[SER_DATA_CMDL]	= 0xA0; 
				uartSndData[SER_DATA_LENGTH]= 6+8;
				uartSndData[SER_DATA_PARA0] = serialNum;
				uartSndData[SER_DATA_PARA1] = ((unsigned char *)(&id))[3];
				uartSndData[SER_DATA_PARA2] = ((unsigned char *)(&id))[2];
				uartSndData[SER_DATA_PARA3] = ((unsigned char *)(&id))[1];
				uartSndData[SER_DATA_PARA4] = ((unsigned char *)(&id))[0];
				uartSndData[SER_DATA_PARA5] = ((unsigned char *)(&type))[1];
				uartSndData[SER_DATA_PARA6] = ((unsigned char *)(&type))[0];
				uartCheck(&uartSndData[SER_DATA_LENGTH]);
				usleep(100*1000);
				SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
				memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
				sprintf(sqlInfo.sql, "insert into devInfo(source, dest, serialNum, id, type, version, ident, report) \
							  values(%d, %d, %d, %d, %d, %d, %d, %d)", source, dest, serialNum, id, type, version, 0, 0);		   
				ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
				CHECK_RET(ret, "ecb_sqlite3_exec error");				
				times = 10;/* recv data , Reset flag bit 10 */
			}
			else if (ECB_DEV_AUTO_REPORT == (((uartPacket*)(data.ptr))->cmd))
			{	
				dest = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 3, 2);
				id = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 9, 4);
				type = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 13, 2);
				version = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 15, 2);
				
				memset(uartSndData, 0, sizeof(uartSndData));
				uartSndData[SER_DATA_HEAD0] = 0x5A;
				uartSndData[SER_DATA_HEAD1] = 0xA5;
				uartSndData[SER_DATA_SRCH]	= 0xFF;
				uartSndData[SER_DATA_SRCL]	= 0xFF;
				uartSndData[SER_DATA_DESTH] = ((unsigned char *)(&dest))[1];
				uartSndData[SER_DATA_DESTL] = ((unsigned char *)(&dest))[0];
				uartSndData[SER_DATA_CMDH]	= 0x01;
				uartSndData[SER_DATA_CMDL]	= 0x04; 
				uartSndData[SER_DATA_LENGTH]= 6+9;
				uartSndData[SER_DATA_PARA0] = ((unsigned char *)(&id))[3];
				uartSndData[SER_DATA_PARA1] = ((unsigned char *)(&id))[2];
				uartSndData[SER_DATA_PARA2] = ((unsigned char *)(&id))[1];
				uartSndData[SER_DATA_PARA3] = ((unsigned char *)(&id))[0];
				uartSndData[SER_DATA_PARA4] = ((unsigned char *)(&type))[1];
				uartSndData[SER_DATA_PARA5] = ((unsigned char *)(&type))[0];
				uartSndData[SER_DATA_PARA6] = ((unsigned char *)(&version))[1];
				uartSndData[SER_DATA_PARA7] = ((unsigned char *)(&version))[0];
				uartCheck(&uartSndData[SER_DATA_LENGTH]);
				usleep(100*1000);
				SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
				
				memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
				sprintf(sqlInfo.sql, "insert into devInfo(source, dest, serialNum, id, type, version, ident, report) \
							  values(%d, %d, %d, %d, %d, %d, %d, %d)", source, dest, serialNum, id, type, version, 0, 0);		   
				ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
				CHECK_RET(ret, "ecb_sqlite3_exec error");
				times = 10;
			}
			wrt_free_memory(((uartPacket*)(data.ptr))->buf);
			wrt_free_memory(data.ptr);
		}
	}
	return 0;
}

//======================================================
//** 函数名称: cont_search_for_dev
//** 功能描述: 继续搜索设备
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int cont_search_for_dev()
{
	int ret;
	int times;
	netResponse spn;
	SQL_INFO sqlInfo;
	unsigned char uartSndData[SER_MAX_LENGTH];

	unsigned short source = 0xffff;		 //src
	unsigned short dest;				 //dest
	unsigned char serialNum;			 //number
	unsigned int id;					 //dev id 
	unsigned short type;				 //dev type
	unsigned short version; 			 //dev version
/*
 *  Notify app to report equipment
 */
 /*
	spn.flag = 0;
	spn.cmd = NET_SEARCH_DEVICE_ACK;	
	spn.bufLen = 1;
	spn.buf = (unsigned char *)malloc(1 * sizeof(unsigned char));
	if (NULL != spn.buf)
	{
		memcpy(spn.buf, "\x02", 1);	
	}	
	if (NULL != g_packet.processResult)
	{
		ret = g_packet.processResult(&spn, &(g_packet.userData));
	}
	wrt_free_memory(spn.buf);*/

	chearQueueData();
/*
 *   search device
 */
	times = 30;
	uartSetIsSearching(1);
	while(times-- && uartGetIsSearching())
	{
		usleep(500*1000);
		data_t data = q_pop_head(g_queue_ctrlAck, 1, 500);	
		if (NULL != data.ptr)
		{
			if (ECB_DEV_AUTO_REPORT == (((uartPacket*)(data.ptr))->cmd))
			{
				dest = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 3, 2);
				id = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 9, 4);
				type = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 13, 2);
				version = getDataNaBytes(((uartPacket*)(data.ptr))->buf, 15, 2);
				
				memset(uartSndData, 0, sizeof(uartSndData));
				uartSndData[SER_DATA_HEAD0] = 0x5A;
				uartSndData[SER_DATA_HEAD1] = 0xA5;
				uartSndData[SER_DATA_SRCH]	= 0xFF;
				uartSndData[SER_DATA_SRCL]	= 0xFF;
				uartSndData[SER_DATA_DESTH] = ((unsigned char *)(&dest))[1];
				uartSndData[SER_DATA_DESTL] = ((unsigned char *)(&dest))[0];
				uartSndData[SER_DATA_CMDH]	= 0x01;
				uartSndData[SER_DATA_CMDL]	= 0x04; 
				uartSndData[SER_DATA_LENGTH]= 6+9;
				uartSndData[SER_DATA_PARA0] = ((unsigned char *)(&id))[3];
				uartSndData[SER_DATA_PARA1] = ((unsigned char *)(&id))[2];
				uartSndData[SER_DATA_PARA2] = ((unsigned char *)(&id))[1];
				uartSndData[SER_DATA_PARA3] = ((unsigned char *)(&id))[0];
				uartSndData[SER_DATA_PARA4] = ((unsigned char *)(&type))[1];
				uartSndData[SER_DATA_PARA5] = ((unsigned char *)(&type))[0];
				uartSndData[SER_DATA_PARA6] = ((unsigned char *)(&version))[1];
				uartSndData[SER_DATA_PARA7] = ((unsigned char *)(&version))[0];
				uartCheck(&uartSndData[SER_DATA_LENGTH]);
				usleep(100*1000);
				SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);			

				memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
				sprintf(sqlInfo.sql, "insert into devInfo(source, dest, serialNum, id, type, version, ident, report) \
							  values(%d, %d, %d, %d, %d, %d, %d, %d)", source, dest, serialNum, id, type, version, 0, 0);		   
				ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
				CHECK_RET(ret, "ecb_sqlite3_exec error");
				
				memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
				sprintf(sqlInfo.sql, "update devInfo set ident = 0, report = 0 where id = %d", id); 		 
				ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
				CHECK_RET(ret, "ecb_sqlite3_exec error");	
				times = 60;/* recv data , Reset flag bit 60 */
			}
			else
			{
			}
			wrt_free_memory(((uartPacket*)(data.ptr))->buf);
			wrt_free_memory(data.ptr);
		}
	}
	return 0;
}

//======================================================
//** 函数名称: search_dev_new
//** 功能描述: 重新搜索设备
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
void *search_dev_new(void *arg)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	unsigned char count[2];
	unsigned char dataList[256];
	unsigned short source = 0xffff; 	 //src
	unsigned short dest;				 //dest
	unsigned char serialNum;			 //dev number
	unsigned int id;					 //dev id
	unsigned short type;				 //dev type
	unsigned short version; 			 //dev version

	
	if (new_search_for_dev() < 0)
	{
		uartSetIsSearching(0);
		DEBUG_ERROR("cont_search_for_dev error");
		return NULL;
	} 
	uartSetIsSearching(0);

	ret = ecb_sqlite3_get_table(db, "select source, dest, id, type, version from devInfo where report = 0", 
									&sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	//CHECK_RET(ret, "ecb_sqlite3_get_table error");

/*
 *	 data format: flag + count + data
 */
	dstr_append(&g_devs, "\x01", 1);
	count[0] = (unsigned short)sqlInfo.row & 0xff;
	count[1] = ((unsigned short)sqlInfo.row >> 8) & 0xff;
	dstr_append(&g_devs, (char *)count, 2); 
	for (index = 5; index < (sqlInfo.row + 1) * sqlInfo.col; index += 5)
	{
		if (checkSqlData(sqlInfo.result, index, 5) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		
		source = atoi(sqlInfo.result[index + 0]);
		dest = atoi(sqlInfo.result[index + 1]);
		id	= atoi(sqlInfo.result[index + 2]);
		type = atoi(sqlInfo.result[index + 3]);
		version = atoi(sqlInfo.result[index + 4]);		
		memset(dataList, 0, sizeof(dataList));
		dataList[0] = ((unsigned char*)&source)[1];
		dataList[1] = ((unsigned char*)&source)[0];
		dataList[2] = ((unsigned char*)&dest)[1];
		dataList[3] = ((unsigned char*)&dest)[0];
		dataList[4] = ((unsigned char*)&id)[3];
		dataList[5] = ((unsigned char*)&id)[2];
		dataList[6] = ((unsigned char*)&id)[1];
		dataList[7] = ((unsigned char*)&id)[0];
		dataList[8] = ((unsigned char*)&type)[1];
		dataList[9] = ((unsigned char*)&type)[0];
		dataList[10] = ((unsigned char*)&version)[1];
		dataList[11] = ((unsigned char*)&version)[0];
		dstr_append(&g_devs, (char *)dataList, 12);

		/* Set the identity to 1 already reported */
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "update devInfo set report = %d where id = %d", 1, id);
		ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
		//CHECK_RET(ret, "ecb_sqlite3_exec error");

	}

	ret = compose_packet_send_app(NET_SEARCH_DEVICE_ACK);
	//CHECK_RET(ret, "NET_SEARCH_DEVICE_ACK error");

	return NULL;
}

//======================================================
//** 函数名称: search_dev_cont
//** 功能描述: 继续搜索设备
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
void *search_dev_cont(void *arg)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	unsigned char count[2];
	unsigned char dataList[256];
	unsigned short source = 0xffff;		 //src
	unsigned short dest;				 //dest
	unsigned char serialNum;			 //dev number
	unsigned int id;					 //dev id
	unsigned short type;				 //dev type
	unsigned short version; 			 //dev version

	
	if (cont_search_for_dev() < 0)
	{
		uartSetIsSearching(0);
		DEBUG_ERROR("cont_search_for_dev error");
		return NULL;
	} 
	uartSetIsSearching(0);

	ret = ecb_sqlite3_get_table(db, "select source, dest, id, type, version from devInfo where report = 0", 
									&sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	//CHECK_RET(ret, "ecb_sqlite3_get_table error");

/*
 *	 data format: flag + count + data
 */
	dstr_append(&g_devs, "\x01", 1);
	count[0] = (unsigned short)sqlInfo.row & 0xff;
	count[1] = ((unsigned short)sqlInfo.row >> 8) & 0xff;
	dstr_append(&g_devs, (char *)count, 2); 
	for (index = 5; index < (sqlInfo.row + 1) * sqlInfo.col; index += 5)
	{
		if (checkSqlData(sqlInfo.result, index, 5) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		
		source = atoi(sqlInfo.result[index + 0]);
		dest = atoi(sqlInfo.result[index + 1]);
		id	= atoi(sqlInfo.result[index + 2]);
		type = atoi(sqlInfo.result[index + 3]);
		version = atoi(sqlInfo.result[index + 4]);		
		memset(dataList, 0, sizeof(dataList));
		dataList[0] = ((unsigned char*)&source)[1];
		dataList[1] = ((unsigned char*)&source)[0];
		dataList[2] = ((unsigned char*)&dest)[1];
		dataList[3] = ((unsigned char*)&dest)[0];
		dataList[4] = ((unsigned char*)&id)[3];
		dataList[5] = ((unsigned char*)&id)[2];
		dataList[6] = ((unsigned char*)&id)[1];
		dataList[7] = ((unsigned char*)&id)[0];
		dataList[8] = ((unsigned char*)&type)[1];
		dataList[9] = ((unsigned char*)&type)[0];
		dataList[10] = ((unsigned char*)&version)[1];
		dataList[11] = ((unsigned char*)&version)[0];
		dstr_append(&g_devs, (char *)dataList, 12);

		/* Set the identity to 1 already reported */
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "update devInfo set report = %d where id = %d", 1, id);
		ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
		//CHECK_RET(ret, "ecb_sqlite3_exec error");

	}

	ret = compose_packet_send_app(NET_SEARCH_DEVICE_ACK);
	//CHECK_RET(ret, "NET_SEARCH_DEVICE_ACK error");

	return NULL;
}

//======================================================
//** 函数名称: net_scene_ctrl
//** 功能描述: 情景控制
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_scene_ctrl(netPacket *packet)
{
	ECB_INFO ecbInfo;
	unsigned char uartSndData[SER_MAX_LENGTH];
	ecbInfo.group= 0xffff;
	ecbInfo.way  = 0x19;
	ecbInfo.para = getDataNaBytes(packet->buf, 0, 1);	
	fill_data_msg(uartSndData, ECB_CTRL_DEVICE, &ecbInfo);
	SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
	uartSetVarScene(0x00, (unsigned char)ecbInfo.para);
	return 0;
}

//======================================================
//** 函数名称: net_group_ctrl
//** 功能描述: 组地址控制
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_group_ctrl(netPacket *packet)
{
	ECB_INFO ecbInfo;
	unsigned char uartSndData[SER_MAX_LENGTH];	
	ecbInfo.group = getDataNaBytes(packet->buf, 0, 2);
	ecbInfo.way   = getDataNaBytes(packet->buf, 2, 1);
	ecbInfo.para  = getDataNaBytes(packet->buf, 3, 2);
	fill_data_msg(uartSndData, ECB_CTRL_DEVICE, &ecbInfo);
	SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
	return 0;
}

//======================================================
//** 函数名称: net_group_query
//** 功能描述: 组地址状态查询
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_group_query(netPacket *packet)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	unsigned char dataList[BUFF_SIZE];

	ret = ecb_sqlite3_get_table(db, "select * from ecbState", &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_get_table error");

	for (index = 3; index < (sqlInfo.row+1)*sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}	
		ecbInfo.group = atoi(sqlInfo.result[index+0]);
		ecbInfo.way   = atoi(sqlInfo.result[index+1]);
		ecbInfo.para  = atoi(sqlInfo.result[index+2]);
		ret = ecb_group_ctrl_map(&ecbInfo.way, &ecbInfo.para);
		CHECK_RET(ret, "ecb_group_ctrl_map error");
		
		memset(dataList, 0, sizeof(dataList));
		dataList[0] = ((unsigned char*)&ecbInfo.group)[1];
		dataList[1] = ((unsigned char*)&ecbInfo.group)[0];
		dataList[2] = ((unsigned char*)&ecbInfo.way)[0];
		dataList[3] = ((unsigned char*)&ecbInfo.para)[1];
		dataList[4] = ((unsigned char*)&ecbInfo.para)[0];	
		dstr_append(&g_group, (char *)dataList, 5);	
	}

	ret = compose_packet_send_app(NET_GROUP_STATUS_QUERY_ACK);
	CHECK_RET(ret, "NET_GROUP_STATUS_QUERY_ACK");

	return 0;
}

//======================================================
//** 函数名称: net_scene_query
//** 功能描述: 情景状态查询
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_scene_query(netPacket *packet)
{
	int ret = compose_packet_send_app(NET_SCENE_STATUS_QUERY_ACK);
	CHECK_RET(ret, "NET_SCENE_STATUS_QUERY_ACK");
	return 0;
}

//======================================================
//** 函数名称: net_scene_sync
//** 功能描述: 状态表同步
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_group_sync(netPacket *packet)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	unsigned short group;

	ret = ecb_sqlite3_exec(db, "delete from ecbState", NULL, NULL, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");

	for (index = 0; index < packet->bufLen; index += 2)
	{
		group = getDataNaBytes(packet->buf, index, 2);
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "insert into ecbState(groupAddr, way, arg) values(%d, %d, %d)", group, 3, 0);      
		ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
		//CHECK_RET(ret, "ecb_sqlite3_exec error");
	}

	//ret = dev_status_group_query(group);
	//CHECK_RET(ret, "dev_status_group_query error");

	ret = compose_packet_send_app(NET_GROUP_STATUS_SYNC_ACK);
	CHECK_RET(ret, "NET_GROUP_STATUS_SYNC_ACK error");

	return 0;
}

//======================================================
//** 函数名称: net_scene_sync
//** 功能描述: 情景表同步
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_scene_sync(netPacket *packet)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	unsigned char scene;

	ret = ecb_sqlite3_exec(db, "delete from ecbScene", NULL, NULL, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");

	for (index = 0; index < packet->bufLen; index += 6)
	{
		scene = getDataNaBytes(packet->buf, index, 1);
		ecbInfo.group = getDataNaBytes(packet->buf, index + 1, 2);
		ecbInfo.way = getDataNaBytes(packet->buf, index + 3, 1);
		ecbInfo.para = getDataNaBytes(packet->buf, index + 4, 2);
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "insert into ecbScene(scene, groupAddr, way, arg) values(%d, %d, %d, %d)", scene, ecbInfo.group, ecbInfo.way, ecbInfo.para);      
		ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");
	}
	
	ret = compose_packet_send_app(NET_SCENE_STATUS_SYNC_ACK);
	CHECK_RET(ret, "NET_SCENE_STATUS_SYNC_ACK error");
	
	return 0;
}

//======================================================
//** 函数名称: net_group_query_single
//** 功能描述: 组地址单个查询
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_group_query_single(netPacket *packet)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	unsigned short group;
	unsigned char dataList[256];
	unsigned char uartSndData[SER_MAX_LENGTH];

	group = getDataNaBytes(packet->buf, 0, 2);

	ret = dev_status_group_query(group);
	CHECK_RET(ret, "dev_status_group_query error");

	ret = ecb_sqlite3_get_table(db, "select * from ecbState", &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_get_table error");

	for (index = 3; index < (sqlInfo.row+1)*sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		ecbInfo.group = atoi(sqlInfo.result[index+0]);
		ecbInfo.way   = atoi(sqlInfo.result[index+1]);
		ecbInfo.para  = atoi(sqlInfo.result[index+2]);
		if (ecbInfo.group == group)
		{
			memset(dataList, 0, sizeof(dataList));
			dataList[0] = ((unsigned char*)&ecbInfo.group)[1];
			dataList[1] = ((unsigned char*)&ecbInfo.group)[0];
			dataList[2] = ((unsigned char*)&ecbInfo.way)[0];
			dataList[3] = ((unsigned char*)&ecbInfo.para)[1];
			dataList[4] = ((unsigned char*)&ecbInfo.para)[0];	
			dstr_append(&g_group, (char *)dataList, 5);	
			break;
		}
	}
	
	ret = compose_packet_send_app(NET_GROUP_STATUS_QUERY_SINGLE_ACK);
	CHECK_RET(ret, "NET_GROUP_STATUS_QUERY_SINGLE_ACK error");

	return 0;
}

//======================================================
//** 函数名称: net_push_config
//** 功能描述: 下发配置数据
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_push_config(netPacket *packet)
{
	int ret;
	int size;
	unsigned char status = 0x03;

	uartSetIsConfiging(1);

	ret = init_faillist_file(&status, 1);
	CHECK_RET(ret, "init_faillist_file error");

	if (packet->cmd == NET_PUSH_CONFIG_DATA)
		dev_config_data_clear();
	//CHECK_RET(ret, "dev_config_data_clear error");

	keep_data_to_list(packet);
	
	send_config_data();
	uartSetIsConfiging(0);

	if (packet->cmd == NET_PUSH_CONFIG_DATA)
	{
		ret = compose_packet_send_app(NET_PUSH_CONFIG_DATA_ACK);
		CHECK_RET(ret, "NET_PUSH_CONFIG_DATA_ACK error");
	}
	else
	{
		ret = compose_packet_send_app(NET_PUSH_FAIL_CONFIG_DATA_ACK);
		CHECK_RET(ret, "NET_PUSH_CONFIG_DATA_ACK error");
	}
	
	ptr_vector_destroy(&g_netCmd);
	ptr_vector_init(&g_netCmd, 100);

	return 0;
}

//======================================================
//** 函数名称: net_search_dev
//** 功能描述: app搜索设备
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_search_dev(netPacket *packet)
{	
	int ret;
	int index;
	SQL_INFO sqlInfo;
	unsigned char count[2];
	unsigned char dataList[256];
	unsigned short source = 0xffff;		 //src
	unsigned short dest;				 //dest
	unsigned char serialNum;			 //dev number
	unsigned int id;					 //dev id
	unsigned short type;				 //dev type
	unsigned short version; 			 //dev version

	uartSetIsSearching(1);/* Enter search status */

	if (0x00 == packet->buf[0])/* new search */
	{
		pool_add_worker(search_dev_new, NULL);
		//if (new_search_for_dev() < 0)
		//{
		//	uartSetIsSearching(0);
		//	DEBUG_ERROR("cont_search_for_dev error");
		//	return -1;
		//}	
	}
	else if (0x01 == packet->buf[0])/* Continue search */
	{
		pool_add_worker(search_dev_cont, NULL);
		//if (cont_search_for_dev() < 0)
		//{
		//	uartSetIsSearching(0);
		//	DEBUG_ERROR("cont_search_for_dev error");
		//	return -1;
		//}	
	}
	else
		return -1;
#if 0
	uartSetIsSearching(0);

	ret = ecb_sqlite3_get_table(db, "select source, dest, id, type, version from devInfo where report = 0", 
									&sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_get_table error");

/*
 *   data format: flag + count + data
 */
	dstr_append(&g_devs, "\x01", 1);
	count[0] = (unsigned short)sqlInfo.row & 0xff;
	count[1] = ((unsigned short)sqlInfo.row >> 8) & 0xff;
	dstr_append(&g_devs, (char *)count, 2);	
	for (index = 5; index < (sqlInfo.row + 1) * sqlInfo.col; index += 5)
	{
		if (checkSqlData(sqlInfo.result, index, 5) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		
		source = atoi(sqlInfo.result[index + 0]);
		dest = atoi(sqlInfo.result[index + 1]);
		id  = atoi(sqlInfo.result[index + 2]);
		type = atoi(sqlInfo.result[index + 3]);
		version = atoi(sqlInfo.result[index + 4]);		
		memset(dataList, 0, sizeof(dataList));
		dataList[0] = ((unsigned char*)&source)[1];
		dataList[1] = ((unsigned char*)&source)[0];
		dataList[2] = ((unsigned char*)&dest)[1];
		dataList[3] = ((unsigned char*)&dest)[0];
		dataList[4] = ((unsigned char*)&id)[3];
		dataList[5] = ((unsigned char*)&id)[2];
		dataList[6] = ((unsigned char*)&id)[1];
		dataList[7] = ((unsigned char*)&id)[0];
		dataList[8] = ((unsigned char*)&type)[1];
		dataList[9] = ((unsigned char*)&type)[0];
		dataList[10] = ((unsigned char*)&version)[1];
		dataList[11] = ((unsigned char*)&version)[0];
		dstr_append(&g_devs, (char *)dataList, 12);

		/* Set the identity to 1 already reported */
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "update devInfo set report = %d where id = %d", 1, id);
		ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");

	}

	ret = compose_packet_send_app(NET_SEARCH_DEVICE_ACK);
	CHECK_RET(ret, "NET_SEARCH_DEVICE_ACK error");
#endif	
	return ret;
}

//======================================================
//** 函数名称: net_set_sensor
//** 功能描述: 下发传感器列表
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_set_sensor(netPacket *packet)
{
	int ret;
	int pos;
	int index;
	int other;
	int paraLen;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	SENSOR_INFO sensorInfo;

	ret = ecb_sqlite3_exec(db, "delete from rfSensor", NULL, NULL, NULL);
	CHECK_RET(ret, "delete from rfSensor error");

	index = 0;
	pos = 1;
	other = 12;
	while (packet->bufLen)
	{
		if ((packet->buf[pos] == UART_DATA_HEAD1 && packet->buf[pos + 1] == UART_DATA_HEAD2) || (pos == packet->bufLen - 1))
		{
			memset(&sensorInfo, 0, sizeof(sensorInfo));	
			sensorInfo.id = getDataNaBytes(packet->buf, index + 2, 4);
			sensorInfo.type = getDataNaBytes(packet->buf, index + 6, 1);

			if (pos < packet->bufLen - 1)
				paraLen = pos - index - other;
			else if (pos == packet->bufLen - 1)
				paraLen = packet->bufLen - index - other;

			switch (paraLen)
			{
				case 1:
					sensorInfo.dataH = getDataNaBytes(packet->buf, index + 7, 1);
					break;
				case 2:
					sensorInfo.dataH = getDataNaBytes(packet->buf, index + 7, 2);
					break;
				case 4:
					sensorInfo.dataH = getDataNaBytes(packet->buf, index + 7, 4);
					break;
				case 8:
					sensorInfo.dataH = getDataNaBytes(packet->buf, index + 7, 4);
					sensorInfo.dataL = getDataNaBytes(packet->buf, index + 11, 4);
					break;
				default:
					break;						
			}			
			ecbInfo.group = getDataNaBytes(packet->buf, index + 7 + paraLen, 2);
			ecbInfo.way = getDataNaBytes(packet->buf, index + 9 + paraLen, 1);
			ecbInfo.para = getDataNaBytes(packet->buf, index + 10 + paraLen, 2);

			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			sprintf(sqlInfo.sql, "insert into rfSensor(id, type, dataH, dataL, groupAddr, way, arg) values(%d, %d, %d, %d, %d, %d, %d)", 
								sensorInfo.id, sensorInfo.type, sensorInfo.dataH, sensorInfo.dataL, ecbInfo.group, ecbInfo.way, ecbInfo.para);
			ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
			CHECK_RET(ret, "ecb_sqlite3_exec error");
			
			index = pos;
		}
		
		if (++pos > packet->bufLen - 1)
			break;
	}
	ret = compose_packet_send_app(NET_SET_SENSOR_DATA_ACK);
	CHECK_RET(ret, "NET_SET_SENSOR_DATA_ACK error");

	return 0;
}

//======================================================
//** 函数名称: net_transmit_data
//** 功能描述: 透传数据
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_transmit_data(netPacket *packet)
{
	unsigned char uartSndData[SER_MAX_LENGTH];	
	memset(uartSndData, 0, sizeof(uartSndData));
	memcpy(uartSndData, "\x5A\xA5", 2);
	memcpy(uartSndData+2, packet->buf, packet->bufLen);
	SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
	return 0;
}

//======================================================
//** 函数名称: net_air_data_query
//** 功能描述: 空调数据查询
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_air_data_query(netPacket *packet)
{
	int ret;
	int times;
	data_t data;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	uartPacket* data_ptr;	
	unsigned char dataList[256];
	unsigned char uartSndData[SER_MAX_LENGTH];
	
	ecbInfo.group = getDataNaBytes(packet->buf, 0, 2);
	fill_data_msg(uartSndData, ECB_AIR_COND_STATUS_QUERY, &ecbInfo);

	chearQueueData();
	times = 3;
	while (times--)
	{	
		SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
		usleep(500*1000);
		data = q_pop_head(g_queue_ctrlAck, 0, 200);
		if (NULL != data.ptr)
		{
			data_ptr = (uartPacket *)data.ptr;
			if (ECB_AIR_COND_STATUS_QUERY_ACK == data_ptr->cmd &&
				ecbInfo.group == getDataNaBytes(data_ptr->buf, 10, 2))
			{	
				memset(dataList, 0, sizeof(dataList));
				memcpy(dataList, data_ptr->buf+10, data_ptr->bufLen-11);
				
				ecbInfo.way = getDataNaBytes(data_ptr->buf, 12, 1);
				
				dstr_append(&g_group, (char *)dataList, data_ptr->bufLen-11); 
				ret = compose_packet_send_app(NET_AIR_COND_STATUS_QUERY_ACK);
				CHECK_RET(ret, "NET_AIR_COND_STATUS_QUERY_ACK error");
			
				wrt_free_memory(data_ptr->buf);
				wrt_free_memory(data_ptr);
				
				#if 0
				ecbInfo.way = (dataList[3] >> 4) & 0x0F;
				if (1 == ecbInfo.way)
					ecbInfo.way = DEV_STATUS_ON;
				else if (2 == ecbInfo.way)
					ecbInfo.way = DEV_STATUS_OFF;
				else
					ecbInfo.way = DEV_STATUS_OFF;
				ecbInfo.para = 0;
				#endif
				
				if (0x01 == ((ecbInfo.way >> 4) & 0x0f))
					ecbInfo.way = DEV_STATUS_ON;
				else if (0x02 == ((ecbInfo.way >> 4) & 0x0f))
					ecbInfo.way = DEV_STATUS_OFF;
				else
					ecbInfo.way = DEV_STATUS_OFF;
				
				if (ecbInfo.way == DEV_STATUS_ON || ecbInfo.way == DEV_STATUS_OFF)
				{
					memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
					sprintf(sqlInfo.sql, "update ecbState set way = %d, arg = %d where groupAddr = %d", ecbInfo.way, ecbInfo.para, ecbInfo.group);      
					ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
					CHECK_RET(ret, "ecb_sqlite3_exec error");
				}

				break;
			}
			else
			{
				wrt_free_memory(data_ptr->buf);
				wrt_free_memory(data_ptr);
			}
		}
	}	

	return -1;	
}

//======================================================
//** 函数名称: net_replace_gw_data
//** 功能描述: 替换网关数据
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_replace_gw_data(netPacket *packet)
{	
	int ret;
	int index;
	SQL_INFO sqlInfo;
	unsigned short source;
	unsigned short dest;
	unsigned int id;
	unsigned short type;
	unsigned short version;	
	unsigned char scene;
	unsigned short group;
	unsigned char way;
	unsigned short para;
	unsigned char list_type;
	
	int pos;
	int other;
	int paraLen;
	ECB_INFO ecbInfo;
	SENSOR_INFO sensorInfo;

	list_type = getDataNaBytes(packet->buf, 0, 1);

	if (1 == list_type)
	{
		ret = ecb_sqlite3_exec(db, "delete from devInfo", NULL, NULL, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");		
		for(index = 1; index < packet->bufLen - 1; index += 12)
		{
			source = getDataNaBytes(packet->buf, index, 2);
			dest = getDataNaBytes(packet->buf, index + 2, 2);
			id = getDataNaBytes(packet->buf, index + 4, 4);
			type = getDataNaBytes(packet->buf, index + 8, 2);
			version = getDataNaBytes(packet->buf, index + 10, 2);
			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			sprintf(sqlInfo.sql, "insert into devInfo(source, dest, serialNum, id, type, version, ident, report) \
				          values(%d, %d, %d, %d, %d, %d, %d, %d)", source, dest, 0, id, type, version, 0, 1);
			ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
			CHECK_RET(ret, "ecb_sqlite3_exec error");
		}
	}
	else if (2 == list_type)
	{
		ret = ecb_sqlite3_exec(db, "delete from ecbScene", NULL, NULL, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");	
		for (index = 1; index < packet->bufLen; index += 6)
		{
			scene = getDataNaBytes(packet->buf, index, 1);
			group = getDataNaBytes(packet->buf, index + 1, 2);
			way   = getDataNaBytes(packet->buf, index + 3, 1);
			para  = getDataNaBytes(packet->buf, index + 4, 2);

			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			sprintf(sqlInfo.sql, "insert into ecbScene(scene, groupAddr, way, arg) values(%d, %d, %d, %d)", scene, group, way, para);      
			ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
			CHECK_RET(ret, "ecb_sqlite3_exec error");

		}
	}
	else if (3 == list_type)
	{
		ret = ecb_sqlite3_exec(db, "delete from rfSensor", NULL, NULL, NULL);
		CHECK_RET(ret, "delete from rfSensor error");

		index = 0;
		pos = 1;
		other = 12;
		while (packet->bufLen)
		{
			if ((packet->buf[pos] == UART_DATA_HEAD1 && packet->buf[pos + 1] == UART_DATA_HEAD2) || (pos == packet->bufLen - 1))
			{
				memset(&sensorInfo, 0, sizeof(sensorInfo));	
				sensorInfo.id = getDataNaBytes(packet->buf, index + 2, 4);
				sensorInfo.type = getDataNaBytes(packet->buf, index + 6, 1);

				if (pos < packet->bufLen - 1)
					paraLen = pos - index - other;
				else if (pos == packet->bufLen - 1)
					paraLen = packet->bufLen - index - other;

				switch (paraLen)
				{
					case 1:
						sensorInfo.dataH = getDataNaBytes(packet->buf, index + 7, 1);
						break;
					case 2:
						sensorInfo.dataH = getDataNaBytes(packet->buf, index + 7, 2);
						break;
					case 4:
						sensorInfo.dataH = getDataNaBytes(packet->buf, index + 7, 4);
						break;
					case 8:
						sensorInfo.dataH = getDataNaBytes(packet->buf, index + 7, 4);
						sensorInfo.dataL = getDataNaBytes(packet->buf, index + 11, 4);
						break;
					default:
						break;						
				}			
				ecbInfo.group = getDataNaBytes(packet->buf, index + 7 + paraLen, 2);
				ecbInfo.way = getDataNaBytes(packet->buf, index + 9 + paraLen, 1);
				ecbInfo.para = getDataNaBytes(packet->buf, index + 10 + paraLen, 2);

				memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
				sprintf(sqlInfo.sql, "insert into rfSensor(id, type, dataH, dataL, groupAddr, way, arg) values(%d, %d, %d, %d, %d, %d, %d)", 
									sensorInfo.id, sensorInfo.type, sensorInfo.dataH, sensorInfo.dataL, ecbInfo.group, ecbInfo.way, ecbInfo.para);
				ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
				CHECK_RET(ret, "ecb_sqlite3_exec error");
				
				index = pos;
			}
			
			if (++pos > packet->bufLen - 1)
				break;
		}		
	}	
	else
	{
		ret = ecb_sqlite3_exec(db, "delete from ecbState", NULL, NULL, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");
		for (index = 1; index < packet->bufLen; index += 2)
		{
			group = getDataNaBytes(packet->buf, index, 2);
			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			sprintf(sqlInfo.sql, "insert into ecbState(groupAddr, way, arg) values(%d, %d, %d)", group, 3, 0);      
			ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
			//CHECK_RET(ret, "ecb_sqlite3_exec error");
			//dev_status_group_query(group);
		}
		//ret = dev_status_group_query(group);
		//CHECK_RET(ret, "dev_status_group_query error");
	}

	g_linkagenum = list_type;
	ret = compose_packet_send_app(NET_RESPLACE_GW_DATA_ACK);
	CHECK_RET(ret, "NET_RESPLACE_GW_DATA_ACK error");
	return 0;
}

//======================================================
//** 函数名称: net_push_fail_config
//** 功能描述: 下发失败配置数据
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_push_fail_config(netPacket *packet)
{
/*
 *  To deal with the same configuration
 */
	net_push_config(packet);
}

//======================================================
//** 函数名称: net_fail_config_query
//** 功能描述: 查询失败配置列表
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_fail_config_query(netPacket *packet)
{		
	int ret;
	int size;
	struct stat statbuf;
	unsigned char para;
	unsigned char *data_buf = NULL;
	char file_name[256] = {0};	
	FILE * fail_fd;
	netResponse spn;

	para = getDataNaBytes(packet->buf, 0, 1);
	if (1 == para)
	{
		memset(file_name, 0, sizeof(file_name));
		sprintf(file_name, "%s", FAILLIST_FILE_PATH);

		stat(file_name, &statbuf);
		size = statbuf.st_size;
		if (size <= 0)
			return 0;
		
		data_buf = (unsigned char *)malloc(size);
		CHECK_PTR(data_buf, "data_buf malloc error\n");

		fail_fd = fopen(file_name, "a+");
		CHECK_PTR(fail_fd, "fail_fd fopen error\n");
		
		if (size > 0)
			ret = fread(data_buf, size, 1, fail_fd);
		
		if (fail_fd != NULL)
			fclose(fail_fd);
		
		dstr_append(&g_faillist, (char *)data_buf, size);

		ret = compose_packet_send_app(NET_FAIL_CONFIG_DATA_QUERY_ACK);
		CHECK_RET(ret, "NET_FAIL_CONFIG_DATA_QUERY_ACK error");
		wrt_free_memory(data_buf);
		
	}
	else if (0 == para)
	{
		memset(file_name, 0, sizeof(file_name));
		sprintf(file_name, "%s", FAILLIST_FILE_PATH);
		fail_fd = fopen(FAILLIST_FILE_PATH, "w+");
		CHECK_PTR(fail_fd, "fail_fd fopen error\n");
		fclose(fail_fd);
	}
	else
	{
	}
	
	return 0;
}

//======================================================
//** 函数名称: net_push_linkage_list
//** 功能描述: 下发联动列表
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_push_linkage_list(netPacket *packet)
{	
	int ret;
	int pos;
	int index;
	int other;
	int paraLen;
	SQL_INFO sqlInfo = {0};
	ECB_INFO ecbInfo = {0};
	SENSOR_INFO sensorInfo = {0};	
	
	unsigned char flag;
	unsigned char actionNum;
	unsigned int  linkCond;
	unsigned char linkSwitch;
	unsigned char alarmSwitch;
	unsigned short waitTime;
	unsigned char ord;
	unsigned char arg;
	unsigned char listType; 
	char nm[40] = {0};
	
	
	listType = getDataNaBytes(packet->buf, 0, 1);

	if (1 == listType)//Synchronous sensor status table
	{
		ret = ecb_sqlite3_exec(db, "delete from sensorState", NULL, NULL, NULL);
		CHECK_RET(ret, "delete from sensorState error");

		index = 1;
		pos = 2;
		other = 7;
		while (packet->bufLen - 1)
		{	
			if ((packet->buf[pos] == UART_DATA_HEAD1 && packet->buf[pos + 1] == UART_DATA_HEAD2) || (pos == packet->bufLen - 1))
			{	
				memset(&sensorInfo, 0, sizeof(sensorInfo));	
				sensorInfo.id = getDataNaBytes(packet->buf, index + 2, 4);
				sensorInfo.type = getDataNaBytes(packet->buf, index + 6, 1);	
				
				if (pos < packet->bufLen - 1)
					paraLen = pos - index - other;
				else if (pos == packet->bufLen - 1)
					paraLen = packet->bufLen - index - other;

				switch(paraLen)
				{
					case 1:
						sensorInfo.para1= getDataNaBytes(packet->buf, index + 7, 1);
						break;
					case 2:
						sensorInfo.para1 = getDataNaBytes(packet->buf, index + 7, 2);
						break;
					case 4:
						sensorInfo.para1 = getDataNaBytes(packet->buf, index + 7, 4);
						break;
					case 8:
						sensorInfo.para1 = getDataNaBytes(packet->buf, index + 7, 4);
						sensorInfo.para2 = getDataNaBytes(packet->buf, index + 11, 4);
						break;
					default:
						break;
				}
				memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
				sprintf(sqlInfo.sql, "insert into sensorState(id, type, para1, para2, para3, para4, para5) values(%d, %d, %d, %d, %d, %d, %d)", 
								sensorInfo.id, sensorInfo.type, sensorInfo.para1, sensorInfo.para2, sensorInfo.para3, sensorInfo.para4, sensorInfo.para5); 	 
				ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
				CHECK_RET(ret, "ecb_sqlite3_exec error");
				index = pos;
			}		
			if (++pos > packet->bufLen - 1)
				break;	
		}

	}
	else if (2 == listType)//Synchronous linkage rule table
	{
		ret = ecb_sqlite3_exec(db, "delete from linkageRule", NULL, NULL, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");
		
		for (index = 1; index < packet->bufLen; index += 18)
		{
			actionNum = getDataNaBytes(packet->buf, index + 2, 1);
			sensorInfo.id = getDataNaBytes(packet->buf, index + 3, 4);
			sensorInfo.type = getDataNaBytes(packet->buf, index + 7, 1);
			sensorInfo.para1 = getDataNaBytes(packet->buf, index + 8, 4);
			linkCond = getDataNaBytes(packet->buf, index + 12, 4);
			linkSwitch = getDataNaBytes(packet->buf, index + 16, 1);
			alarmSwitch = getDataNaBytes(packet->buf, index + 17, 1);
			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			sprintf(sqlInfo.sql, "insert into linkageRule(actionNum, id, type, linkpara, linkcond, linkswitch, alarmswitch) values(%d, %d, %d, %d, %d, %d, %d)", 
							actionNum, sensorInfo.id, sensorInfo.type, sensorInfo.para1, linkCond, linkSwitch, alarmSwitch); 	 
			ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
			CHECK_RET(ret, "ecb_sqlite3_exec error");		
		}
	}
	else if (3 == listType)//Synchronous execution action table
	{
		ret = ecb_sqlite3_exec(db, "delete from performAction", NULL, NULL, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");

		for (index = 1; index < packet->bufLen; index += 11)
		{
			actionNum = getDataNaBytes(packet->buf, index + 2, 1);
			ord  = getDataNaBytes(packet->buf, index + 3, 1);
			ecbInfo.group= getDataNaBytes(packet->buf, index + 4, 2);
			ecbInfo.way  = getDataNaBytes(packet->buf, index + 6, 1);
			ecbInfo.para = getDataNaBytes(packet->buf, index + 7, 2);
			waitTime = getDataNaBytes(packet->buf, index + 9, 2);
			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			sprintf(sqlInfo.sql, "insert into performAction(actionNum, ord, groupAddr, way, arg, sleep) values(%d, %d, %d, %d, %d, %d)", 
							actionNum, ord, ecbInfo.group, ecbInfo.way, ecbInfo.para, waitTime);	   
			ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
			CHECK_RET(ret, "ecb_sqlite3_exec error");
		}
	}
	else if(4 == listType)//Synchronous Chinese name table
	{
		ret = ecb_sqlite3_exec(db, "delete from actionName", NULL, NULL, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");
		
		for (index = 3; index < packet->bufLen; index += 43)
		{
			actionNum = getDataNaBytes(packet->buf, index, 1);
			memset(nm, 0, sizeof(nm));
			memcpy(nm, packet->buf + index + 1, 40);
			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			sprintf(sqlInfo.sql, "insert into actionName(actionNum, nm) values(%d, '%s')", actionNum, nm); 
			ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
			CHECK_RET(ret, "ecb_sqlite3_exec error");
		}
	}
	else if(5 == listType)//Synchronous push list
	{
		if (packet->bufLen > 150)
		{
			ret = ecb_sqlite3_exec(db, "delete from pushAlarm", NULL, NULL, NULL);
			CHECK_RET(ret, "ecb_sqlite3_exec error");
			
			for (index = 1; index < packet->bufLen; index += 47)
			{
				memset(nm, 0, sizeof(nm));
				memcpy(nm, packet->buf + index, 40);
				sensorInfo.id  = getDataNaBytes(packet->buf, index + 40, 4);
				sensorInfo.type= getDataNaBytes(packet->buf, index + 44, 1);
				arg = getDataNaBytes(packet->buf, index + 45, 1);
				flag= getDataNaBytes(packet->buf, index + 46, 1);
				
				memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
				sprintf(sqlInfo.sql, "insert into pushAlarm(nm, id, type, arg, pushflag) values('%s', %d, %d, %d, %d)", 
								nm, sensorInfo.id, sensorInfo.type, arg, flag); 
				ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
				CHECK_RET(ret, "ecb_sqlite3_exec error");
			}
		}
		else
		{	
			memset(nm, 0, 40);
			memcpy(nm, packet->buf + 1, 40);
			sensorInfo.id  = getDataNaBytes(packet->buf, 1 + 40, 1);
			sensorInfo.type= getDataNaBytes(packet->buf, 1 + 44, 1);
			arg = getDataNaBytes(packet->buf, 1 + 45, 1);
			flag= getDataNaBytes(packet->buf, 1 + 46, 1);
			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			
			sprintf(sqlInfo.sql, "update pushAlarm set nm = '%s', pushflag = %d where id = %d and type = %d and arg = %d", 
				nm, flag, sensorInfo.id, sensorInfo.type, arg); 
			ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
			CHECK_RET(ret, "ecb_sqlite3_exec error");	
		}
	}
	else
	{
	}
	
	g_linkagenum = listType;
	ret = compose_packet_send_app(NET_PUSH_LINKAGE_LIST_ACK);
	CHECK_RET(ret, "NET_PUSH_LINKAGE_LIST_ACK error");		
	return ret;
}

//======================================================
//** 函数名称: net_set_linkage_switch
//** 功能描述: 网络设置联动开关
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_set_linkage_switch(netPacket *packet)
{
	int ret;
	int index;
	char sql[256];
	unsigned char actionNum;
	unsigned char linkageSwitch;
	
	for (index = 0; index < packet->bufLen; index += 2)
	{
		actionNum = getDataNaBytes(packet->buf, index, 1);
		linkageSwitch = getDataNaBytes(packet->buf, index + 1, 1);

		memset(sql, 0, sizeof(sql));
		sprintf(sql, "update linkageRule set linkswitch = %d where actionNum = %d", linkageSwitch, actionNum);
		ret = ecb_sqlite3_exec(db, sql, NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		{
			DEBUG_ERROR("ecb_sqlite3_exec error\n");
			return -1;
		}
	}
	if (compose_packet_send_app(NET_SET_LINKAGE_SWTICH_ACK))
	{
		DEBUG_ERROR("NET_SET_LINKAGE_SWTICH_ACK error\n");
		return -1;
	}

	return 0;
}

//======================================================
//** 函数名称: net_set_defense_flag
//** 功能描述: 网络设置布撤防标志
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_set_defense_flag(netPacket *packet)
{
	int ret;
	char sql[256];
	unsigned char defFlag;
	netResponse spn;

	defFlag = getDataNaBytes(packet->buf, 0, 1);
	if ((AKEY_DEFENSE_ON == defFlag) || (AKEY_DEFENSE_OFF == defFlag))
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "update defState set state = %d", defFlag);          
		ret = ecb_sqlite3_exec(db, sql, NULL, NULL, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");
		//set defense
		setAkeyDefense(defFlag);
		//app alarm
		ret = compose_packet_send_app(NET_SET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "NET_SET_DEFENSE_FLAG_ACK error");
		//ext alarm
		ret = compose_packet_send_app(EXT_GET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "EXT_GET_DEFENSE_FLAG_ACK error");
	}
	else if (AKEY_DEFENSE_QUERY == defFlag)
	{
		spn.flag = 0;
		spn.cmd = NET_SET_DEFENSE_FLAG_ACK;	
		spn.bufLen = 2;
		spn.buf = (unsigned char *)malloc(2 * sizeof(unsigned char));
		CHECK_PTR(spn.buf, "spn buf malloc error");
		spn.buf[0] = AKEY_DEFENSE_QUERY;
		spn.buf[1] = (unsigned char)getAkeyDefense();	
		if (NULL != g_packet.processResult)
		{
			ret = g_packet.processResult(&spn, &(g_packet.userData));
			if (ret < 0)
				DEBUG_ERROR("defense packet->processResult error\n");
		}
		wrt_free_memory(spn.buf);
	}

	return 0;
}

//======================================================
//** 函数名称: net_set_timer_list
//** 功能描述: 下发定时列表
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_set_timer_list(netPacket *packet)
{
	int ret;
	int index;
	TIMER_INFO timerInfo;
	SQL_INFO sqlInfo;
	
	ret = ecb_sqlite3_exec(db, "delete from timerList", NULL, NULL, NULL);
	CHECK_RET(ret, "delete from timerList fail");

	for (index = 0; index < packet->bufLen; index += 16)
	{
		timerInfo.actionNum = getDataNaBytes(packet->buf + index, 0, 1);
		timerInfo.timerYear = getDataNaBytes(packet->buf + index, 1, 4);
		timerInfo.timerTime = getDataNaBytes(packet->buf + index, 5, 4);
		timerInfo.ctrlWay = getDataNaBytes(packet->buf + index, 9, 1);
		timerInfo.para1 = getDataNaBytes(packet->buf + index, 10, 2);
		timerInfo.para2 = getDataNaBytes(packet->buf + index, 12, 2);
		timerInfo.linkSwitch = getDataNaBytes(packet->buf + index, 14, 1);
		timerInfo.defSwitch = getDataNaBytes(packet->buf + index, 15, 1);
		
		printf("actionnum: %d\n", timerInfo.actionNum);
		printf("timerTime: %04d-%02d-%02d %02d:%02d:%02d %d %02x %02x %d %d\n", timerInfo.timerYear >> 16 & 0xffff, 
			timerInfo.timerYear >> 8 & 0xff, timerInfo.timerYear & 0xff, timerInfo.timerTime >> 24 & 0xff, timerInfo.timerTime >> 16 & 0xff, timerInfo.timerTime >> 8 & 0xff,
			timerInfo.ctrlWay, timerInfo.para1, timerInfo.para2, timerInfo.linkSwitch, timerInfo.defSwitch);

		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "insert into timerList(actionnum, timeryear, timertime, ctrlway, para1, para2, linkswitch, defswitch) values(%d, %d, %d, %d, %d, %d, %d, %d)",
							 timerInfo.actionNum, timerInfo.timerYear, timerInfo.timerTime, timerInfo.ctrlWay, timerInfo.para1, timerInfo.para2, timerInfo.linkSwitch, timerInfo.defSwitch); 
		ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
		CHECK_RET(ret, "delete from timerList fail");
	}

	return 0;
}

//======================================================
//** 函数名称: net_extension_report_alarm
//** 功能描述: 网络上报告警-分机
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_extension_report_alarm(netPacket *packet)
{
	int defenseArea;
	char eventName[BUFF_SIZE];
	char defenseName[BUFF_SIZE];

	// 4B(int)num + 32B(char)name
	defenseArea = getDataNaBytes(packet->buf, 0, 4);

	memset(eventName, 0, sizeof(eventName));
	memset(defenseName, 0, sizeof(defenseName));
	memcpy(defenseName, packet->buf + 4, 32);
	sprintf(eventName, "%d %s", defenseArea, defenseName);
	
	if (AKEY_DEFENSE_ON == getAkeyDefense())
		ext_push_alarm_message(eventName);
	
	return 0;
}

//======================================================
//** 函数名称: net_extension_defense_flag
//** 功能描述: 网络布撤防-分机
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_extension_defense_flag(netPacket *packet)
{
	int ret;
	int defFlag;
	SQL_INFO sqlInfo;

	defFlag = (getDataNaBytes(packet->buf, 0, 4) == 0) ? 1 : 0;
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "update defState set state = %d", defFlag);          
	ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
	CHECK_RET(ret, "update defState error");
	setAkeyDefense(defFlag);
	
	return 0;
}

//======================================================
//** 函数名称: net_set_time_sync
//** 功能描述: 网络设置时间同步
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int net_set_time_sync(netPacket *packet)
{	
	struct tm mytm;
	time_t set_time;
	struct timeval tv;

	mytm.tm_year = getDataNaBytes(packet->buf, 0, 2);
	mytm.tm_mon  = getDataNaBytes(packet->buf, 2, 1);
	mytm.tm_mday = getDataNaBytes(packet->buf, 3, 1);
	mytm.tm_wday = getDataNaBytes(packet->buf, 4, 1);
	mytm.tm_hour = getDataNaBytes(packet->buf, 5, 1);
	mytm.tm_min  = getDataNaBytes(packet->buf, 6, 1);
	mytm.tm_sec  = getDataNaBytes(packet->buf, 7, 1);	
	mytm.tm_year -= 1900;
	mytm.tm_mon -= 1;	
	set_time = mktime(&mytm);	
	tv.tv_sec = set_time;
	tv.tv_usec = 0;
	settimeofday(&tv, (struct timezone*)0);	
	setRtcTime();
	return 0;
}

//======================================================
//** 函数名称: compare_data_is_pass
//** 功能描述: 比较数据是否通过
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int compare_data_is_pass(uartPacket *packet)
{
	int ret;
	SQL_INFO sqlInfo = {0};
	SENSOR_INFO sensorInfo = {0}; 

	switch (packet->cmd)
	{
		case ECB_SENSOR_DATA_REPORT:
			sensorInfo.id = getDataNaBytes(packet->buf, 10, 4);
			sensorInfo.type = getDataNaBytes(packet->buf, 14, 1);
			memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
			sprintf(sqlInfo.sql, "select para1, para2 from sensorState where id = %d and type = %d", sensorInfo.id, sensorInfo.type);
			ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
			CHECK_RET(ret, "select data from sensorState error\n");
			if (!(sqlInfo.row && sqlInfo.col))
				return -1;
			DEBUG_ERROR("sensor data ok!!\n");
			break;
			
		case ECB_CTRL_DEVICE_ACK:
		case ECB_STATUS_DATA_REPORT:
		case ECB_SEARCH_DEVICE:	
		case ECB_SENSOR_TIME_SYNC:	
			return -1;
			
		default:
			break;
	}
	
	return 0;
}

//======================================================
//** 函数名称: data_send_to_queue
//** 功能描述: 数据发送到队列
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int data_send_to_queue(uartPacket *packet)
{
	data_t data = {0};
	uartPacket *uart_data = NULL;
	
	uart_data = (uartPacket *)malloc(sizeof(uartPacket));
	CHECK_PTR(uart_data, "uart_data malloc error");

	uart_data->cmd = packet->cmd;
	uart_data->bufLen = packet->bufLen;
	uart_data->buf = (unsigned char*)malloc(uart_data->bufLen * sizeof(unsigned char));
	if (NULL == uart_data->buf)
	{
		wrt_free_memory(uart_data);
		DEBUG_ERROR("uart_data->buf malloc error\n");
		return -1;
	}
	memset(uart_data->buf, 0, uart_data->bufLen);
	memcpy(uart_data->buf, packet->buf, uart_data->bufLen);	
	data.ptr = uart_data;
	if (packet->cmd == ECB_SENSOR_DATA_REPORT)
		q_push_tail(g_queue_linkage, &data);
	else
		q_push_tail(g_queue_ctrlAck, &data);
	
	return 0;
}

//======================================================
//** 函数名称: sensor_switch_prc
//** 功能描述: 开关处理
//** 输　入: sensorInfo ecbInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int sensor_switch_prc(SENSOR_INFO *sensorInfo, ECB_INFO *ecbInfo)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select groupAddr, way, arg from ecbState where groupAddr =(select groupAddr from rfSensor where id = %d and dataH = %d and dataL = %d)", sensorInfo->id, sensorInfo->dataH, sensorInfo->dataL);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");

	for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		ecbInfo->group = atoi(sqlInfo.result[index + 0]);
		ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
		ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
		ecbInfo->way = (ecbInfo->way == DEV_STATUS_ON) ? DEV_STATUS_OFF : DEV_STATUS_ON;
	}
	
	return 0;
}

//======================================================
//** 函数名称: sensor_curtain_prc
//** 功能描述: 窗帘处理
//** 输　入: sensorInfo ecbInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int sensor_curtain_prc(SENSOR_INFO *sensorInfo, ECB_INFO *ecbInfo)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select groupAddr, way, arg from ecbState where groupAddr =(select groupAddr from rfSensor where id = %d and dataH = %d and dataL = %d)", sensorInfo->id, sensorInfo->dataH, sensorInfo->dataL);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");

	for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		ecbInfo->group = atoi(sqlInfo.result[index + 0]);
		ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
		ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
		if (1 == sensorInfo->dataH)
			ecbInfo->way = CUR_CTRL_ON;
		else if (2 == sensorInfo->dataH)
			ecbInfo->way = CUR_CTRL_STOP;
		else if (3 == sensorInfo->dataH)
			ecbInfo->way = CUR_CTRL_OFF; 
	}

	return 0;
}

//======================================================
//** 函数名称: sensor_dimmer_prc
//** 功能描述: 调光处理
//** 输　入: sensorInfo ecbInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int sensor_dimmer_prc(SENSOR_INFO *sensorInfo, ECB_INFO *ecbInfo)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select groupAddr, way, arg from ecbState where groupAddr =(select groupAddr from rfSensor where id = %d and dataH = %d and dataL = %d)", sensorInfo->id, sensorInfo->dataH, sensorInfo->dataL);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");
	
	for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		ecbInfo->group = atoi(sqlInfo.result[index + 0]);
		ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
		ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
		if (1 == sensorInfo->dataH)
		{
			if (DEV_STATUS_OFF != ecbInfo->way && ecbInfo->para > 0 && ecbInfo->para <= 100)
				ecbInfo->way = DEV_STATUS_OFF;
			else if (DEV_STATUS_OFF == ecbInfo->way)
				ecbInfo->way = DEV_STATUS_ON;
		}
		else if (2 == sensorInfo->dataH)
		{
			ecbInfo->way = TURN_LIGHT_ADD;
			ecbInfo->para = 0;
		}
		else if (3 == sensorInfo->dataH)
		{
			ecbInfo->way = TURN_LIGHT_SUB;
			ecbInfo->para = 0;
		}
	}

	return 0;
}

//======================================================
//** 函数名称: sensor_scene_prc
//** 功能描述: 情景处理
//** 输　入: sensorInfo ecbInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int sensor_scene_prc(SENSOR_INFO *sensorInfo, ECB_INFO *ecbInfo)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select groupAddr, way, arg from rfSensor where id = %d and dataH = %d and dataL = %d", sensorInfo->id, sensorInfo->dataH, sensorInfo->dataL);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");

	for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		ecbInfo->group = atoi(sqlInfo.result[index + 0]);
		ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
		ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
		uartSetVarScene(((unsigned char *)&ecbInfo->para)[1], ((unsigned char *)&ecbInfo->para)[0]);
	}	

	return 0;
}

//======================================================
//** 函数名称: sensor_ctrl_prc
//** 功能描述: 遥控器处理
//** 输　入: sensorInfo ecbInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int sensor_ctrl_prc(SENSOR_INFO *sensorInfo, ECB_INFO *ecbInfo)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select groupAddr, way, arg from rfSensor where id = %d and dataH = %d and dataL = %d", sensorInfo->id, sensorInfo->dataH, sensorInfo->dataL);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");

	for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		ecbInfo->group = atoi(sqlInfo.result[index + 0]);
		ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
		ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
	}
	
	if (DEV_CTRL_SWITCH == ecbInfo->way)
	{
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "select groupAddr, way, arg from ecbState where groupAddr =(select groupAddr from rfSensor where id = %d and dataH = %d and dataL = %d)", sensorInfo->id, sensorInfo->dataH, sensorInfo->dataL);
		ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");
		for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
		{
			if (checkSqlData(sqlInfo.result, index, 3) < 0)
			{
				DEBUG_ERROR("checkSqlData error\n");
				continue;
			}
			ecbInfo->group = atoi(sqlInfo.result[index + 0]);
			ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
			ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
			ecbInfo->way = (ecbInfo->way == DEV_STATUS_ON) ? DEV_STATUS_OFF : DEV_STATUS_ON;
		}
	}
	else if (DEFENSE_STATUS == ecbInfo->way)
	{
		ecbInfo->group = 0;		
		if (1 == ecbInfo->para)
			setAkeyDefense(0);
		else
			setAkeyDefense(1);

		ret = compose_packet_send_app(NET_SET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "NET_SET_DEFENSE_FLAG_ACK error");

		ret = compose_packet_send_app(EXT_GET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "EXT_GET_DEFENSE_FLAG_ACK error");
	}

	return 0;
}

//======================================================
//** 函数名称: sensor_trigger_prc
//** 功能描述: 触发器处理
//** 输　入: sensorInfo ecbInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int sensor_trigger_prc(SENSOR_INFO *sensorInfo, ECB_INFO *ecbInfo)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select groupAddr, way, arg from rfSensor where id = %d and dataH = %d and dataL = %d", sensorInfo->id, sensorInfo->dataH, sensorInfo->dataL);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");
	
	for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		ecbInfo->group = atoi(sqlInfo.result[index + 0]);
		ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
		ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
	}
	
	if (DEV_CTRL_SWITCH == ecbInfo->way)
	{
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "select groupAddr, way, arg from ecbState where groupAddr =(select groupAddr from rfSensor where id = %d and dataH = %d and dataL = %d)", sensorInfo->id, sensorInfo->dataH, sensorInfo->dataL);
		ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");
		
		for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
		{
			if (checkSqlData(sqlInfo.result, index, 3) < 0)
			{
				DEBUG_ERROR("checkSqlData error\n");
				continue;
			}
			ecbInfo->group = atoi(sqlInfo.result[index + 0]);
			ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
			ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
			ecbInfo->way = (ecbInfo->way == DEV_STATUS_ON) ? DEV_STATUS_OFF : DEV_STATUS_ON;
		}
	}
	else if (DEFENSE_STATUS == ecbInfo->way)
	{
		ecbInfo->group = 0;	
		if (1 == ecbInfo->para)
			setAkeyDefense(0);
		else
			setAkeyDefense(1);
	
		ret = compose_packet_send_app(NET_SET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "NET_SET_DEFENSE_FLAG_ACK error");
		
		ret = compose_packet_send_app(EXT_GET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "EXT_GET_DEFENSE_FLAG_ACK error");
	}

	return 0;
}

//======================================================
//** 函数名称: sensor_env_prc
//** 功能描述: 环境传感器处理
//** 输　入: sensorInfo ecbInfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int sensor_env_prc(SENSOR_INFO *sensorInfo, ECB_INFO *ecbInfo)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select groupAddr, way, arg from rfSensor where id = %d", sensorInfo->id);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");

	for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		ecbInfo->group = atoi(sqlInfo.result[index + 0]);
		ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
		ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
	}
	
	if (DEV_CTRL_SWITCH == ecbInfo->way)
	{
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "select groupAddr, way, arg from ecbState where groupAddr =(select groupAddr from rfSensor where id = %d)", sensorInfo->id);
		ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");
		
		for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
		{
			if (checkSqlData(sqlInfo.result, index, 3) < 0)
			{
				DEBUG_ERROR("checkSqlData error\n");
				continue;
			}
			ecbInfo->group = atoi(sqlInfo.result[index + 0]);
			ecbInfo->way   = atoi(sqlInfo.result[index + 1]);
			ecbInfo->para  = atoi(sqlInfo.result[index + 2]);
			ecbInfo->way = (ecbInfo->way == DEV_STATUS_ON) ? DEV_STATUS_OFF : DEV_STATUS_ON;
		}
	}
	else if (DEFENSE_STATUS == ecbInfo->way)
	{
		ecbInfo->group = 0;	
		if (1 == ecbInfo->para)
			setAkeyDefense(0);
		else
			setAkeyDefense(1);
	
		ret = compose_packet_send_app(NET_SET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "NET_SET_DEFENSE_FLAG_ACK error");
		
		ret = compose_packet_send_app(EXT_GET_DEFENSE_FLAG_ACK);
		CHECK_RET(ret, "EXT_GET_DEFENSE_FLAG_ACK error");
	}
	else if (DEV_SCENE_CTRL == ecbInfo->way)
	{
	
	}
	else
	{
		memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
		sprintf(sqlInfo.sql, "select groupAddr, way, arg from ecbState where groupAddr =(select groupAddr from rfSensor where id = %d)", sensorInfo->id);
		ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
		CHECK_RET(ret, "ecb_sqlite3_exec error");
		
		for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
		{
			if (checkSqlData(sqlInfo.result, index, 3) < 0)
			{
				DEBUG_ERROR("checkSqlData error\n");
				continue;
			}
			//ecbInfo->group = atoi(sqlInfo.result[index + 0]);
			if (ecbInfo->way == atoi(sqlInfo.result[index + 1]) &&
				ecbInfo->para == atoi(sqlInfo.result[index + 2]))
				ecbInfo->group = 0;
		}
	}

	return 0;
}

//======================================================
//** 函数名称: ecb_sensor_ctrl
//** 功能描述: 传感器控制
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int ecb_sensor_ctrl(uartPacket *packet)
{
	int ret;
	int cnt;
	unsigned char num;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo = {0};
	SENSOR_INFO sensorInfo = {0};
	unsigned char uartSndData[BUFF_SIZE];
#if 1	
	memset(uartSndData, 0, sizeof(uartSndData));
	uartSndData[SER_DATA_HEAD0] = 0x5A;
	uartSndData[SER_DATA_HEAD1] = 0xA5;
	uartSndData[SER_DATA_SRCH]	= 0xFF;
	uartSndData[SER_DATA_SRCL]	= 0xFF;
	uartSndData[SER_DATA_DESTH] = packet->buf[3];
	uartSndData[SER_DATA_DESTL] = packet->buf[4];
	uartSndData[SER_DATA_CMDH]	= 0x03;
	uartSndData[SER_DATA_CMDL]	= 0x51;	
	uartSndData[SER_DATA_LENGTH]= packet->bufLen - 3;	
	memcpy(&uartSndData[SER_DATA_PARA0], &packet->buf[9], packet->bufLen - 10);
	uartCheck(&uartSndData[SER_DATA_LENGTH]);
	SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);	
#endif
	num = getDataNaBytes(packet->buf, 9, 1);
	sensorInfo.id = getDataNaBytes(packet->buf, 10, 4);
	sensorInfo.type = getDataNaBytes(packet->buf, 14, 1);
	cnt = packet->bufLen - 16;

	sensorInfo.dataH = 0;
	sensorInfo.dataL = 0;
	switch(cnt)
	{
		case 1:
			sensorInfo.dataH = getDataNaBytes(packet->buf, 15, 1);
			break;
		case 2:
			sensorInfo.dataH = getDataNaBytes(packet->buf, 15, 2);
			break;
		case 4:
			sensorInfo.dataH = getDataNaBytes(packet->buf, 15, 4);
			break;
		case 8:
			sensorInfo.dataH = getDataNaBytes(packet->buf, 15, 4);
			sensorInfo.dataL = getDataNaBytes(packet->buf, 19, 4);
			break;
		default:
			break;
	}

	if (sensorInfo.type >= 0x2C && sensorInfo.type <= 0x33)// 1 road,8 road switch
		ret = sensor_switch_prc(&sensorInfo, &ecbInfo);
	else if (0x2A == sensorInfo.type || 0x2B == sensorInfo.type)//curtains
		ret = sensor_curtain_prc(&sensorInfo, &ecbInfo);
	else if (0x28 == sensorInfo.type || 0x29 == sensorInfo.type)//Dimming
		ret = sensor_dimmer_prc(&sensorInfo, &ecbInfo);
	else if (sensorInfo.type >= 0x34 && sensorInfo.type <= 0x3B)//scene panel
		ret = sensor_scene_prc(&sensorInfo, &ecbInfo);
	else if (0x01 == sensorInfo.type || 0x52 == sensorInfo.type || 0x23 == sensorInfo.type)//8 key,32 key ctrl
		ret = sensor_ctrl_prc(&sensorInfo, &ecbInfo);
	else if (0x44 == sensorInfo.type)// 4 road trigger
		ret = sensor_trigger_prc(&sensorInfo, &ecbInfo);
	else if (0x53 == sensorInfo.type)// env
		ret = sensor_env_prc(&sensorInfo, &ecbInfo);	
	else 
		ret = sensor_ctrl_prc(&sensorInfo, &ecbInfo);
	CHECK_RET(ret, "ecb_sensor_ctrl error");

	if (ecbInfo.group)
	{
		usleep(500*1000);
		fill_data_msg(uartSndData, ECB_CTRL_DEVICE, &ecbInfo);
		SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);
	}

	return 0;
}

//======================================================
//** 函数名称: ecb_devtime_sync
//** 功能描述: 设备时间同步
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int ecb_devtime_sync(uartPacket *packet)
{
	time_t timeTmp;
	struct tm *curTime;
	unsigned char uartSndData[BUFF_SIZE];
	
	char number = getDataNaBytes(packet->buf, 9, 1);
	
	time(&timeTmp);
	curTime = localtime(&timeTmp);
	
	//DEBUG_ERROR("%d-%d-%d %d:%d:%d week:%d day:%d",
	//	curTime->tm_year, curTime->tm_mon, curTime->tm_mday, 
	//	curTime->tm_hour, curTime->tm_min, curTime->tm_sec,
	//	curTime->tm_wday, curTime->tm_yday);
	
	if (0 == curTime->tm_wday)
		curTime->tm_wday += 7;
	
	curTime->tm_year += 1900;
	curTime->tm_mon += 1;

	memset(uartSndData, 0, sizeof(uartSndData));
	uartSndData[SER_DATA_HEAD0] = 0x5A;
	uartSndData[SER_DATA_HEAD1] = 0xA5;
	uartSndData[SER_DATA_LENGTH] = 6+9;
	uartSndData[SER_DATA_SRCH] = 0xFF;
	uartSndData[SER_DATA_SRCL] = 0xFF;
	uartSndData[SER_DATA_DESTH] = packet->buf[3];
	uartSndData[SER_DATA_DESTL] = packet->buf[4];
	uartSndData[SER_DATA_CMDH] = 0x03;
	uartSndData[SER_DATA_CMDL] = 0x31;
	uartSndData[SER_DATA_PARA0] = number;
	uartSndData[SER_DATA_PARA1] = ((unsigned char *)&curTime->tm_year)[1];	
	uartSndData[SER_DATA_PARA2] = ((unsigned char *)&curTime->tm_year)[0];	
	uartSndData[SER_DATA_PARA3] = ((unsigned char *)&curTime->tm_mon)[0];
	uartSndData[SER_DATA_PARA4] = ((unsigned char *)&curTime->tm_mday)[0];
	uartSndData[SER_DATA_PARA5] = ((unsigned char *)&curTime->tm_wday)[0];	
	uartSndData[SER_DATA_PARA6] = ((unsigned char *)&curTime->tm_hour)[0];
	uartSndData[SER_DATA_PARA7] = ((unsigned char *)&curTime->tm_min)[0];
	uartSndData[SER_DATA_PARA8] = ((unsigned char *)&curTime->tm_sec)[0];
	
	uartCheck(&uartSndData[SER_DATA_LENGTH]);
	SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);	

	return 0;
}

//======================================================
//** 函数名称: sendDevtimeTimer
//** 功能描述: 设备时间同步
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int sendDevtimeTimer()
{
	time_t timeTmp;
	struct tm *curTime;
	unsigned char uartSndData[BUFF_SIZE];

	time(&timeTmp);
	curTime = localtime(&timeTmp);
	
	if (0 == curTime->tm_wday)
		curTime->tm_wday += 7;

	curTime->tm_year += 1900;
	curTime->tm_mon += 1;
	//default year time don't deal
	if (curTime->tm_year == 2015)
		return -1;

	memset(uartSndData, 0, sizeof(uartSndData));
	uartSndData[SER_DATA_HEAD0] = 0x5A;
	uartSndData[SER_DATA_HEAD1] = 0xA5;
	uartSndData[SER_DATA_LENGTH] = 6+9;
	uartSndData[SER_DATA_SRCH] = 0xFF;
	uartSndData[SER_DATA_SRCL] = 0xFF;
	uartSndData[SER_DATA_DESTH] = 0xFF;
	uartSndData[SER_DATA_DESTL] = 0xFF;
	uartSndData[SER_DATA_CMDH] = 0x03;
	uartSndData[SER_DATA_CMDL] = 0x31;
	uartSndData[SER_DATA_PARA0] = 0x01;
	uartSndData[SER_DATA_PARA1] = ((unsigned char *)&curTime->tm_year)[1];	
	uartSndData[SER_DATA_PARA2] = ((unsigned char *)&curTime->tm_year)[0];	
	uartSndData[SER_DATA_PARA3] = ((unsigned char *)&curTime->tm_mon)[0];
	uartSndData[SER_DATA_PARA4] = ((unsigned char *)&curTime->tm_mday)[0];
	uartSndData[SER_DATA_PARA5] = ((unsigned char *)&curTime->tm_wday)[0];	
	uartSndData[SER_DATA_PARA6] = ((unsigned char *)&curTime->tm_hour)[0];
	uartSndData[SER_DATA_PARA7] = ((unsigned char *)&curTime->tm_min)[0];
	uartSndData[SER_DATA_PARA8] = ((unsigned char *)&curTime->tm_sec)[0];
	
	uartCheck(&uartSndData[SER_DATA_LENGTH]);
	SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);	

	return 0;
}

//======================================================
//** 函数名称: update_group_status
//** 功能描述: 更新数据库状态
//** 输　入: info
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int update_group_status(ECB_INFO *info)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;

	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select way, arg from ecbState where groupAddr = %d", info->group);	
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_get_table error");

	for (index = 2; index < (sqlInfo.row + 1) * sqlInfo.col; index += 2)
	{
		if (checkSqlData(sqlInfo.result, index, 2) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}	
		ecbInfo.way   = atoi(sqlInfo.result[index + 0]);
		ecbInfo.para  = atoi(sqlInfo.result[index + 1]);	
		if (DEV_CTRL_SWITCH == info->way)
			info->way = (ecbInfo.way == DEV_STATUS_ON) ? DEV_STATUS_OFF : DEV_STATUS_ON;
		else if (CUR_CTRL_ON == info->way)
			info->way = DEV_STATUS_ON;
		else if (CUR_CTRL_OFF == info->way)
			info->way = DEV_STATUS_OFF;

		//info->para = 0;
	}

	return 0;	
}

//======================================================
//** 函数名称: scene_status_sync
//** 功能描述: 情景状态同步
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int scene_status_sync(uartPacket *packet)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	unsigned short scene;
	unsigned char data_buf[BUFF_SIZE];
	
	scene = getDataNaBytes(packet->buf, 12, 2);
	uartSetVarScene(0x00, scene);
	
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "update ecbState set way=(select way from ecbScene where scene = %d and groupAddr = ecbState.groupAddr),arg=(select arg from ecbScene where scene = %d and groupAddr = ecbState.groupAddr)", scene, scene);	  
	ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");
	
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "select groupAddr,way,arg from ecbScene where scene = %d", scene);
	ret = ecb_sqlite3_get_table(db, sqlInfo.sql, &sqlInfo.result, &sqlInfo.row, &sqlInfo.col, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");
	
	for (index = 3; index < (sqlInfo.row + 1) * sqlInfo.col; index += 3)
	{
		if (checkSqlData(sqlInfo.result, index, 3) < 0)
		{
			DEBUG_ERROR("checkSqlData error\n");
			continue;
		}
		ecbInfo.group = atoi(sqlInfo.result[index + 0]);
		ecbInfo.way   = atoi(sqlInfo.result[index + 1]);
		ecbInfo.para  = atoi(sqlInfo.result[index + 2]);
		
		memset(data_buf, 0, sizeof(data_buf));
		data_buf[0] = ((unsigned char*)&ecbInfo.group)[1];
		data_buf[1] = ((unsigned char*)&ecbInfo.group)[0];
		data_buf[2] = ((unsigned char*)&ecbInfo.way)[0];
		data_buf[3] = ((unsigned char*)&ecbInfo.para)[1];
		data_buf[4] = ((unsigned char*)&ecbInfo.para)[0];
		dstr_append(&g_group, (char *)data_buf, 5); 
	}

	ret = compose_packet_send_app(NET_GROUP_STATUS_QUERY_ACK);
	CHECK_RET(ret, "NET_GROUP_STATUS_QUERY_ACK error");
	ret = compose_packet_send_app(NET_SCENE_STATUS_QUERY_ACK);	
	CHECK_RET(ret, "NET_SCENE_STATUS_QUERY_ACK error");
	
	return 0;
}

//======================================================
//** 函数名称: group_status_sync
//** 功能描述: 组地址状态同步
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int group_status_sync(uartPacket *packet)
{
	int ret;
	int index;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	unsigned char data_buf[BUFF_SIZE];

	ecbInfo.group = getDataNaBytes(packet->buf, 9, 2);
	ecbInfo.way = getDataNaBytes(packet->buf, 11, 1);
	ecbInfo.para = getDataNaBytes(packet->buf, 12, 2);
	update_group_status(&ecbInfo);
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "update ecbState set way = %d, arg = %d where groupAddr = %d", ecbInfo.way, ecbInfo.para, ecbInfo.group);		   
	ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);
	CHECK_RET(ret, "ecb_sqlite3_exec error");
	memset(data_buf, 0, sizeof(data_buf));
	data_buf[0] = ((unsigned char*)&ecbInfo.group)[1];
	data_buf[1] = ((unsigned char*)&ecbInfo.group)[0];
	data_buf[2] = ((unsigned char*)&ecbInfo.way)[0];
	data_buf[3] = ((unsigned char*)&ecbInfo.para)[1];
	data_buf[4] = ((unsigned char*)&ecbInfo.para)[0];
	dstr_append(&g_group, (char *)data_buf, 5); 

	ret = compose_packet_send_app(NET_GROUP_STATUS_QUERY_SINGLE_ACK);
	CHECK_RET(ret, "NET_GROUP_STATUS_QUERY_SINGLE_ACK error");

}

//======================================================
//** 函数名称: ecb_search_gw
//** 功能描述: 搜索网关
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int ecb_search_gw(uartPacket *packet)
{
	unsigned char uartSndData[SER_MAX_LENGTH];
	memset(uartSndData, 0, sizeof(uartSndData));
	uartSndData[SER_DATA_HEAD0] = 0x5A;
	uartSndData[SER_DATA_HEAD1] = 0xA5;
	uartSndData[SER_DATA_LENGTH]= 6+10;
	uartSndData[SER_DATA_SRCH]  = 0xFF;
	uartSndData[SER_DATA_SRCL]  = 0xFF;
	uartSndData[SER_DATA_DESTH] = 0xFF;
	uartSndData[SER_DATA_DESTL] = 0xFF;
	uartSndData[SER_DATA_CMDH]  = 0x01;
	uartSndData[SER_DATA_CMDL]  = 0x02;
	uartSndData[SER_DATA_PARA0]  = packet->buf[9];
	uartSndData[SER_DATA_PARA1] = *(unsigned char *)(((char *)(&g_mcuId))+3);//device id
	uartSndData[SER_DATA_PARA2] = *(unsigned char *)(((char *)(&g_mcuId))+2);
	uartSndData[SER_DATA_PARA3] = *(unsigned char *)(((char *)(&g_mcuId))+1);
	uartSndData[SER_DATA_PARA4] = *(unsigned char *)(((char *)(&g_mcuId))+0);
	uartSndData[SER_DATA_PARA5] = 0x30; //device type
	uartSndData[SER_DATA_PARA6] = 0x0F;
	uartSndData[SER_DATA_PARA7] = g_mcuVer[1];//device version
	uartSndData[SER_DATA_PARA8] = g_mcuVer[2];
	uartCheck(&uartSndData[SER_DATA_LENGTH]);
	SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH]+3);
	
	return 0;
}

//======================================================
//** 函数名称: ecb_status_sync
//** 功能描述: 状态同步
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int ecb_status_sync(uartPacket *packet)
{
	int ret;
	unsigned char ctrl_way;;

	ctrl_way = getDataNaBytes(packet->buf, 11, 1);
	if (DEV_SCENE_CTRL == ctrl_way)
		ret = scene_status_sync(packet);
	else
		ret = group_status_sync(packet);	
	CHECK_RET(ret, "ecb_status_sync error");
	
	return 0;
}

//======================================================
//** 函数名称: ecb_report_status
//** 功能描述: 状态上报
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int ecb_report_status(uartPacket *packet)
{
	int ret;
	SQL_INFO sqlInfo;
	ECB_INFO ecbInfo;
	unsigned char chan;
	unsigned short addr;
	unsigned char data_buf[BUFF_SIZE];
	unsigned char uartSndData[BUFF_SIZE];
	
	addr = getDataNaBytes(packet->buf, 3, 2);
	chan = getDataNaBytes(packet->buf, 9, 1);
	ecbInfo.group = getDataNaBytes(packet->buf, 10, 2);
	ecbInfo.way = getDataNaBytes(packet->buf, 12, 1);
	ecbInfo.para = getDataNaBytes(packet->buf, 13, 2);		
	memset(uartSndData, 0, sizeof(uartSndData));
	uartSndData[SER_DATA_HEAD0] = 0x5A;
	uartSndData[SER_DATA_HEAD1] = 0xA5;
	uartSndData[SER_DATA_SRCH]	= 0xFF;
	uartSndData[SER_DATA_SRCL]	= 0xFF;
	uartSndData[SER_DATA_DESTH] = ((unsigned char*)&addr)[1];
	uartSndData[SER_DATA_DESTL] = ((unsigned char*)&addr)[0];
	uartSndData[SER_DATA_CMDH]	= 0x03;
	uartSndData[SER_DATA_CMDL]	= 0x92;	
	uartSndData[SER_DATA_LENGTH]= 6+2;
	uartSndData[SER_DATA_PARA0]= ((unsigned char*)&chan)[0];	
	uartCheck(&uartSndData[SER_DATA_LENGTH]);
	SendDataToUart(uartSndData, uartSndData[SER_DATA_LENGTH] + 3);	
	update_group_status(&ecbInfo);
	memset(sqlInfo.sql, 0, sizeof(sqlInfo.sql));
	sprintf(sqlInfo.sql, "update ecbState set way = %d, arg = %d where groupAddr = %d", ecbInfo.way, ecbInfo.para, ecbInfo.group);
	ret = ecb_sqlite3_exec(db, sqlInfo.sql, NULL, NULL, NULL);	
	CHECK_RET(ret, "ecb_sqlite3_exec error");
	
	memset(data_buf, 0, sizeof(data_buf));
	data_buf[0] = ((unsigned char*)&ecbInfo.group)[1];
	data_buf[1] = ((unsigned char*)&ecbInfo.group)[0];
	data_buf[2] = ((unsigned char*)&ecbInfo.way)[0];
	data_buf[3] = ((unsigned char*)&ecbInfo.para)[1];
	data_buf[4] = ((unsigned char*)&ecbInfo.para)[0];
	dstr_append(&g_group, (char *)data_buf, 5); 

	ret = compose_packet_send_app(NET_GROUP_STATUS_QUERY_SINGLE_ACK);
	CHECK_RET(ret, "NET_GROUP_STATUS_QUERY_SINGLE_ACK error");

	return 0;
}

/******************************************************************************/
/*
 *   Data is transmitted to app, the gateway does nothing
 */
//======================================================
//** 函数名称: uart_transmit_prc
//** 功能描述: 数据透传处理
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int uart_transmit_prc(uartPacket *packet)
{
	netResponse spn;
	
	if (packet->cmd  == ECB_SEARCH_DEVICE_ACK || 
		packet->cmd  == ECB_DEV_DATA_CLEAR_ACK)
		return 0;

	spn.flag = 0;
	spn.cmd = NET_TRANSMIT_DATA_ACK;
	spn.bufLen = packet->bufLen - 2;//Remove(5a+a5)
	spn.buf = &(packet->buf[2]);
	packet->buf[2] = packet->bufLen - 3;//Remove(5a+a5+len1B)
	uartCheck(&packet->buf[2]);
	if (NULL != g_packet.processResult)
		g_packet.processResult(&spn, &(g_packet.userData));
	
	return 0;
}

/*
 *   Network instruction type and function pointer mapping table
 */
netCmdUtil netCmdUtils[] = 	
{
	{0x0616, net_scene_ctrl},
	{0x061B, net_group_ctrl},
	{0x0620, net_group_query},
	{0x0621, net_scene_query},
	{0x0622, net_group_sync},
	{0x0624, net_scene_sync},
	{0x0626, net_group_query_single},
	{0x062A, net_push_config},
	{0x062C, net_search_dev},
	{0x062D, net_set_sensor},
	{0x0692, net_transmit_data},
	{0x0627, net_air_data_query},
	{0x062F, net_replace_gw_data},
	{0x0630, net_push_fail_config},
	{0x0631, net_fail_config_query},
	{0x0634, net_push_linkage_list},
	{0x0635, net_set_linkage_switch},
	{0x0636, net_set_defense_flag},
	{0x0639, net_set_timer_list}, 
	{0x063A, net_sensor_status_report},   	
	{0x0643, net_extension_report_alarm},
	{0x0644, net_extension_defense_flag},
	{0x0647, net_set_time_sync}, 
};

/*
 *   ECB answer instruction and function pointer mapping table
 */
ecbCmdUtil ecbCmdUtils[] = 	
{
	{0x0101, ecb_search_gw},
	{0x0202, ecb_status_sync},
	{0x0391, ecb_report_status},
	{0x0350, ecb_sensor_ctrl},
	{0x0330, ecb_devtime_sync}, 
};

/******************************************************************************/
/*
 *   Network data processing function
 */
//======================================================
//** 函数名称: net_data_ctrl
//** 功能描述: 网络数据分类处理
//** 输　入: packet
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int net_data_ctrl(netPacket *packet)
{
	int ret;
	int index;

	netRecvMesToLog(packet, (const unsigned char *)packet->buf, packet->bufLen, packet->cmd);
	
	for (index = 0; index < sizeof(netCmdUtils)/sizeof(netCmdUtil); index++)
	{
		if (netCmdUtils[index].netType == packet->cmd)
		{			
			ret = netCmdUtils[index].netCmdPrc(packet);
			return ret;
		}
	}
	
	return -1;
}

/******************************************************************************/
/*
 *   The serial data is classified and sent to different queues
 *	You can parse multiple frames at the same time
 */
//======================================================
//** 函数名称: uart_data_prc
//** 功能描述: 串口数据处理
//** 输　入: recvData msgLen
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int uart_data_prc(char *recvData, int msgLen)
{
	int ret;
	int index;
	int pos = 0;
	uartPacket uart_packet = {0};
	unsigned char * recvbuf = (unsigned char *)recvData;

	printf("\n%04x : ", (recvData[7] << 8 | recvData[8]));
	for(int i = 0; i < msgLen; i++)
		printf("%02x ", *(recvData + i));
	printf("\n");
	
	while (true)
	{	
		if (UART_DATA_HEAD1 == recvbuf[pos] && UART_DATA_HEAD2 == recvbuf[pos + 1])
		{
			uart_packet.cmd = getDataNaBytes(recvbuf, pos + 7, 2);
			uart_packet.bufLen = recvbuf[pos + 2] + 3;
			uart_packet.buf = (unsigned char *)malloc((uart_packet.bufLen) * sizeof(unsigned char));		
			if (NULL == uart_packet.buf)
				continue;			
			memset(uart_packet.buf, 0, uart_packet.bufLen);
			memcpy(uart_packet.buf, recvbuf + pos, uart_packet.bufLen);
			uartRecvMesToLog(uart_packet.buf, uart_packet.bufLen, uart_packet.cmd);
			for (index = 0; index < sizeof(ecbCmdUtils)/sizeof(ecbCmdUtil); index++)
			{
				if (ecbCmdUtils[index].ecbType == uart_packet.cmd)
				{
					ret = ecbCmdUtils[index].ecbCmdPrc(&uart_packet);
					break;
				}
			}

			if (0 == compare_data_is_pass(&uart_packet))
				data_send_to_queue(&uart_packet);
				
			uart_transmit_prc(&uart_packet);
			wrt_free_memory(uart_packet.buf);/* free mem */
		}
		if (++pos > msgLen)
			break;
		usleep(10 * 1000);
	}
	
	return ret;
}

/******************************************************************************/
/*
 *   Receive serial data, Send to data processing function
 */
//======================================================
//** 函数名称: wrt_uartRecvHandler_thread
//** 功能描述: 串口数据接收处理线程
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
void *wrt_uartRecvHandler_thread(void *arg)
{
	int len;
	unsigned char arr[BUFF_SIZE * 4];
	while (true)
	{
		usleep(10*1000);
		memset(arr, 0, sizeof(arr));
		len = uartread(serial_fd, arr, sizeof(arr));
		
		if (len > 0)
			uart_data_prc((char *)arr, len);
	}
	
	return NULL;
}

