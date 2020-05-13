#ifndef __WRT_DEVHANDLER_H__
#define __WRT_DEVHANDLER_H__

#include "sqlite3.h"
#include "wrt_common.h"

#define SER_MAX_LENGTH    						(38)
#define UART_DATA_HEAD1 						(0x5A)
#define UART_DATA_HEAD2 						(0xA5)

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#define ECB_DEV_DATA_CLEAR						(0x0A01)
#define ECB_DEV_DATA_CLEAR_ACK					(0x0A02)

#define ECB_DEV_STATUS_GROUP_QUERY 				(0x0380)
#define ECB_DEV_STATUS_GROUP_QUERY_ACK 		 	(0x0381)

#define ECB_AIR_COND_STATUS_QUERY 				(0x0382)
#define ECB_AIR_COND_STATUS_QUERY_ACK 		 	(0x0383)

#define ECB_DEV_AUTO_REPORT 					(0x0103)
#define ECB_DEV_AUTO_REPORT_ACK 		 		(0x0104)

#define ECB_DEV_CONFIG_DATA 					(0x0105)
#define ECB_DEV_CONFIG_DATA_ACK 		 		(0x0106)

#define ECB_SEARCH_DEVICE 						(0x0101)
#define ECB_SEARCH_DEVICE_ACK 		 			(0x0102)

#define ECB_CTRL_DEVICE 						(0x0201)
#define ECB_CTRL_DEVICE_ACK 		 			(0x0202)

#define ECB_STATUS_DATA_REPORT					(0x0391)
#define ECB_STATUS_DATA_REPORT_ACK				(0x0392)

#define ECB_GET_GATEWAY_ID 						(0x0901)
#define ECB_GET_GATEWAY_ID_ACK 		 			(0x0902)

#define ECB_SENSOR_DATA_REPORT					(0x0350)
#define ECB_SENSOR_DATA_REPORT_ACK				(0x0351)

#define ECB_SENSOR_TIME_SYNC					(0x0330)
#define ECB_SENSOR_TIME_SYNC_ACK				(0x0331)


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#define ALARM_SENSOR_TYPE						(0x23)
#define MULT_ENV_SENSOR_TYPE					(0x53)
#define PM_SENSOR_TYPE 							(0x1D)
#define MULT_ENV_HEART_SENSOR_TYPE 				(0x6E)
#define BODY_LIGHT_SENSOR_TYPE 					(0x6D)

//报警推送信息
typedef struct _GW_POST_INFO{
	char uuid[13];
	char id[32];
	char eventType;
	char eventName[40];
	char time[14];
}GW_POST_INFO;

//设备信息
typedef struct _DEV_INFO{
	unsigned short addr;
	unsigned short cmd;
	unsigned char chan;	
	unsigned int devId;
	unsigned short type;
}DEV_INFO;

//传感器信息
typedef struct _SENSOR_INFO{
	unsigned int id;
	unsigned char type;
	unsigned int para1;
	unsigned int para2;
	unsigned int para3;
	unsigned int para4;
	unsigned int para5;
	unsigned int dataH;
	unsigned int dataL;
}SENSOR_INFO;

//传感器超时信息
typedef struct _SENSOR_TIMEOUT{
	unsigned int id;
	unsigned char type;
	time_t timeout;
}SENSOR_TIMEOUT;

//数据库信息
typedef struct _ECB_INFO
{	
	unsigned short group;
	unsigned char  way;
	unsigned short para;
}ECB_INFO;

//数据库信息
typedef struct _SQL_INFO
{	
	int row;
	int col;
	char **result;
	char sql[256];
}SQL_INFO;

//定时器信息
typedef struct _TIMER_INFO
{	
	unsigned char  actionNum;
	unsigned int  timerYear;
	unsigned int  timerTime;
	unsigned char  ctrlWay;
	unsigned short para1;
	unsigned short para2;
	unsigned char  linkSwitch;
	unsigned char  defSwitch;
	unsigned int  timebuf[8];
}TIMER_INFO;


//网络通信日志信息
typedef struct NET_LOG_DATA
{
	char time[30];
	char cmdType[4];
	char cmdWay[8];
	char srcName[40];
	char dstName[40];
	unsigned short cmd;
	char cmdCN[40];
	char buf[1024]; 
}netLogData;

//串口通信日志信息
typedef struct UART_LOG_DATA
{
	char time[30];
	char cmdName[4];
	unsigned int srcId;
	unsigned int dstId;
	unsigned short cmd;
	char cmdCN[40];
	char buf[1024];
}uartLogData;

//网络数据接收信息
typedef struct netCmdUtil
{
	unsigned short netType;
	int (*netCmdPrc)(netPacket *packet);
}netCmdUtil;

//串口数据接收信息
typedef struct ecbCmdUtil
{
	unsigned short ecbType;
	int (*ecbCmdPrc)(uartPacket *packet);
}ecbCmdUtil;

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

//白名单状态 1:查询 2:同步 3:恢复
enum _whitelist_status
{
	WHITE_LIST_QUERY = 1,
	WHITE_LIST_SYNC,
	WHITE_LIST_RECOV,	
};

//设备控制方式
enum _dev_ctrl_way
{
	DEV_STATUS_ON  = 0x01,
	DEV_STATUS_OFF = 0x03,
	CUR_CTRL_ON    = 0x09,
	CUR_CTRL_OFF   = 0x10,
	CUR_CTRL_STOP  = 0x11,
	DEV_CTRL_SWITCH= 0x12,
	TURN_LIGHT_ADD = 0x13,
	TURN_LIGHT_SUB = 0x14,
	DEV_SCENE_CTRL = 0x19,
	DEFENSE_STATUS = 0xA0,
};

//布撤防类型 0:布防 1:撤防 2:查询
enum _defense_type
{
	AKEY_DEFENSE_ON = 0,
	AKEY_DEFENSE_OFF,
	AKEY_DEFENSE_QUERY,
};

//报警类型 0:关闭 1:开启 2:全时
enum _alarm_type
{
	ALARM_FUNC_OFF = 0,
	ALARM_FUNC_ON,
	ALARM_FULL_TIME,
};

//联动条件方式 0:等于 1:小于 2:大于...
enum _link_condtion
{
	LINK_COND_EQUAL = 0,
	LINK_COND_LESS,
	LINK_COND_MORE,
	LINK_COND_LESSEQUAL,
	LINK_COND_MOREEQUAL,
	LINK_COND_BIT,
};


//定时器方式 1:单次 2:重复
enum __time_ctrlway
{
	SINGLE_TIMER = 1,
	REPEAT_TIMER = 2,
};


//SERIAL数据位置列表
enum _SER_DATA_POS
{
	SER_DATA_HEAD0 = 0,
	SER_DATA_HEAD1,	
	SER_DATA_LENGTH,
	SER_DATA_SRCH,
	SER_DATA_SRCL,
	SER_DATA_DESTH,
	SER_DATA_DESTL,
	SER_DATA_CMDH,
	SER_DATA_CMDL,
	SER_DATA_PARA0,
	SER_DATA_PARA1,
	SER_DATA_PARA2,
	SER_DATA_PARA3,
	SER_DATA_PARA4,
	SER_DATA_PARA5,
	SER_DATA_PARA6,
	SER_DATA_PARA7,
	SER_DATA_PARA8,
	SER_DATA_PARA9,
	SER_DATA_PARA10,
	SER_DATA_PARA11,
	SER_DATA_PARA12,
	SER_DATA_PARA13,
	SER_DATA_PARA14,
	SER_DATA_PARA15,
	SER_DATA_PARA16,
	SER_DATA_PARA17,
	SER_DATA_PARA18,
	SER_DATA_PARA19,
	SER_DATA_PARA20,
	SER_DATA_PARA21,
	SER_DATA_PARA22,
	SER_DATA_PARA23,
	SER_DATA_PARA24,
	SER_DATA_PARA25,
	SER_DATA_PARA26,
	SER_DATA_PARA27,
	SER_DATA_PARA28,
	SER_DATA_PARA29,
	SER_DATA_PARA30,
};


void InitUartDataToAppCallBack();
int setAkeyDefense(int data);
int getAkeyDefense();
int uartSetIsSearching(int data);
int uartGetIsSearching();
int uartSetIsConfiging(int data);
int uartGetIsConfiging();
int uartSetMcuId(unsigned int id);
unsigned int uartGetMcuId();
int setWhiteListCount(int count);
int getWhileListCount();
unsigned char uartGetSeqnumUp();
int get_gw_version();
int init_global_queue();
int init_defense_state();
int sqlite3_load_init();
int sqlite3_table_init(const char buf[][256], int size);
int uartSetVarScene(unsigned char Hdata, unsigned char Ldata);
unsigned char *uartGetVarScene();
int ecb_sqlite3_exec(sqlite3 *db, const char *sql, void *aa, void *bb, void *cc);
int ecb_sqlite3_get_table( sqlite3 *db, const char *zSql, char ***pazResult, int *pnRow, int *pnColumn, char **pzErrmsg);
int checkSqlData(char **sqlData, int pos, int size);
int composeSendPacket( const netResponse *response, char **sendbuf, int *send_len, bool rc4_md5_flag);
int uartDataToAppCallBack(netResponse *response, void *userData);
int json_post_message(GW_POST_INFO *post_params);
int SendDataToUart(unsigned char *str, unsigned char length);
int net_data_ctrl(netPacket *packet);
void fill_data_msg(unsigned char *uartSndData, unsigned short cmd, ECB_INFO *ecbInfo);
int compose_packet_send_app(unsigned short cmd);
int init_faillist_file(unsigned char *str, int num);
int exit_search_status ();
int WRTGateWayRate();
int get_id_version();


int init_dev_handler();
int uart_compare_data_packet(uartPacket *packet);
int uart_data_prc(char *recvData, int msgLen);
void *wrt_uartRecvHandler_thread();

int update_sensor_status_data(uartPacket *pdata, SENSOR_INFO *sensorInfo);
int query_sensor_linkage_data(SENSOR_INFO *sensorInfo);
int compara_sensor_linkage_data(unsigned char actionNum);
int compara_sensor_status_data(SENSOR_INFO *sensorInfo, unsigned char actionNum);
int compare_sqlite_status_data(ECB_INFO *info);
int compara_timer_linkage_data(unsigned char actionNum);
int query_sensor_sqlite_data(unsigned int linkCond, SENSOR_INFO *sensorInfo);
void *dealLinkageFunc(void *arg);
int select_linkage_action_run(unsigned char actionNum, int number);
int division_ctrlway_func(unsigned char way, unsigned short para);
void *performActionPrc(void *arg);
void *performActionPrc2(void *arg);
void *performActionPrc3(void *arg);
int push_alarm_message(unsigned char actionNum);
int ext_push_alarm_message(unsigned char *eventName);
int net_sensor_status_report(netPacket *packet);

int ecb_devtime_sync(uartPacket *packet);
int sendDevtimeTimer();

void *dealTimerFunc(void *arg);
int getTimeInfoFromTimer(struct tm *taskTime);
int timerInfoFromTimerActionPrc(TIMER_INFO *timerInfo, unsigned char *timebuf);
int timerTaskLinkActionPrc(unsigned char number);
int deleteTimerFormAction(unsigned char number);



#endif


