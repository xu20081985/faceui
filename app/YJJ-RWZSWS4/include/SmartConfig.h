#pragma once

#include <roomlib.h>
#include "list.h"

#pragma pack(1)
typedef struct
{
	WORD head;				// 帧头  0xA55A
	BYTE length;			// 帧总长度 - sizeof(head) - sizeof(length)
	WORD src;				// 源地址
	WORD dst;				// 目标地址
	BYTE type;				// 命令类型
	BYTE cmd;				// 命令类型
	DWORD pwd;				// 系统密码
	BYTE data[];			// 帧数据
}ECB_DATA;

typedef struct
{
	DEVICE device;			// 设备标识
	BYTE cmd;				// 控制方式
	WORD param;				// 控制参数
}ECB_CTRL_ACK;


typedef struct
{
	DEVICE device;			// 设备标识
	BYTE cmd;				// 控制方式
	BYTE param1;		    // 控制参数1
	BYTE param2;			// 控制参数2
	BYTE param3;		    // 控制参数3
	BYTE param4;		    // 控制参数4
#if 0	
	BYTE param5;		    // 控制参数5
	BYTE param6;			// 控制参数6
	BYTE param7;		    // 控制参数7
	BYTE param8;		    // 控制参数8
	BYTE param9;		    // 控制参数9
	BYTE param10;			// 控制参数10
	BYTE param11;		    // 控制参数11
	BYTE param12;		    // 控制参数12
#endif	
}ECB_CTRL_ACK_AIR;

typedef struct
{
	unsigned char temp:5;	// bit0 - bit4 温度
	unsigned char speed:3;	// bit5 - bit7 风速
	unsigned char func:2;	// bit8 - bit9 功能(扫风)
	unsigned char mode:4;	// bit10 - bit13 模式
	unsigned char onoff:2;	// bit14 - bit15 状态
}AC_DATA;

typedef struct
{
	unsigned char voice:5;  // bit0 - bit4 音量
	unsigned char source:3; // bit5 - bit7 音源
	unsigned char feel:2;   // bit8 - bit9 音效
	unsigned char func:4;   // bit10 - bit13 功能
	unsigned char status:2; // bit14 - bit15 状态
}MUSIC_DATA;


typedef struct
{
	WORD type;				// 设备类型
	WORD phy_addr;			// 设备物理地址
	DEVICE device;   		// 设备唯一标识
	BYTE cmd;				// 控制方式
	WORD status;			// 设备状态
	WORD param1;			// 控制参数
	char name[32];			// 设备名称
	BOOL exist;				// 设备是否存在
	BYTE scene;
}SmartDev;

typedef struct {
	LISTOBJ lpObj;
	BYTE hour;
	BYTE minute;
	BYTE week;
	DEVICE device;
	BYTE way;
	WORD param;
	BOOL onoff;
	char timeStr[64];
	char weekStr[64];
	char devStr[64];
	char wayStr[64];
} SmartTimer,  *PSmartTimer;

typedef struct
{
	DEVICE device; 			// 设备标识
	BYTE num;				// 存储编号
 	BYTE scene;				// 场景编号
 	BYTE cmd;				// 控制方式
 	WORD param;				// 控制参数
}SceneCtrl;

typedef struct
{
	BYTE type;				// 按键学习类型
	BYTE singleNum;			// 单键学习个数
	BYTE doubleNum;			// 双键学习个数
	SENSOR senosr[16];		// 按键学习信息
}LightStudy;

#pragma pack()


// 帧头数据
const WORD ECB_HEAD				=  0xA55A;	// head = 0x5AA5
const BYTE ECB_DATA_HEAD 		=  11;		// sizeof(ECB_DATA) - sizeof(head) - sizeof(length) + 1
const BYTE ECB_HEAD_LENGTH 		=  2;		// sizeof(head)
const BYTE ECB_DATA_MIN_LEN		=  14;		// sizeof(ECB_DATA) + sizeof(check_num)
const BYTE ECB_HEAD_LEN_LENGTH 	=  3;		// sizeof(head) + sizeof(length)

// 设备类型
const WORD ECB_DEV_TYPE	        =  0xF12D;  // type 0xF12D
const WORD WIND_DEV_TYPE	    =  0xB1F3;  // type 0xB1F3

// 每帧数据最长38个字节
#define MAX_ECB_DATA_LEN			38

// 属性
#define ECB_TYPE_ATTR				0x01
// 控制
#define ECB_TYPE_CTRL				0x02
// 同步
#define ECB_TYPE_SYNC				0x03
// 设置
#define ECB_TYPE_SET				0x05
// 全部
#define ECB_TYPE_ALL				0x0A

// 设置器件定时控制功能
#define ECB_SET_TIMER				0xBA
#define ECB_SET_TIMER_ACK			0xBB

// 设置器件日期/星期/时间
#define ECB_SET_TIME				0x61
#define ECB_SET_TIME_ACK			0x62

// 设置场景模式控制同步信息
#define ECB_SET_SCENE				0x25
#define ECB_SET_SCENE_ACK			0x26

// 设置器件之控制信息
#define ECB_SET_DEV_CFG				0xBE
#define ECB_SET_DEV_CFG_ACK			0xBF

// 搜索器件
#define ECB_SEARCH_DEV				0x01
#define ECB_SEARCH_DEV_ACK			0x02

// 器件主动上报
#define ECB_REPORT_INFO				0x03
#define ECB_REPORT_INFO_ACK			0x04

// 应用控制
#define ECB_CTRL_DEV				0x05
#define ECB_CTRL_DEV_ACK			0x06

// 器件同步日期/星期/时间
#define ECB_SYNC_TIME				0x30
#define ECB_SYNC_TIME_ACK			0x31

// 传感器上报状态(多功能传感器)
#define ECB_SENSOR_REPORT			0x50
#define ECB_SENSOR_REPORT_ACK		0x51

// 查询器件开关状态
#define ECB_GET_STATUS				0x84
#define ECB_GET_STATUS_ACK			0x85

// 器件主动上报状态
#define ECB_SYNC_STATUS				0x94
#define ECB_SYNC_STATUS_ACK			0x95

// 配置控制
#define ECB_CFG_CTRL				0xA0
#define ECB_CFG_CTRL_ACK			0xA1

// 器件初始化
#define ECB_INIT_ALL				0xAE
#define ECB_INIT_ALL_ACK			0xAF
// 器件软复位
#define ECB_RESET_ALL				0xAC
#define ECB_RESET_ALL_ACK			0xAD

// 设置物理地址和系统密码
#define ECB_SET_ADDR_PWD			0x53
#define ECB_SET_ADDR_PWD_ACK		0x54

// 秒开和秒关
#define ECB_CTRL_ALL				0x0A
#define ECB_CTRL_ALL_ACK			0x0B
/////////////////////////////////////////////
/////////////////////////////////////////////

// 页数
#define MAX_PAGE_NUM				16
// 每一页的图标数量
#define MAX_ICON_NUM				5
// 最大支持设备个数
#define MAX_DEV_NUM					MAX_PAGE_NUM * MAX_ICON_NUM

// 灯光通路
#define MAX_CHAN_NUM				3
// 状态更新
#define MSG_SMART_UPDATE			1

// 初始设备密码
#define INIT_DEV_PWD				0x99999999
// 初始化物理地址
#define INIT_PHY_ADDR				0x9999

// 无效的通路 ID TYPE
#define INVALID_DEV_CHAN			0xFF
#define INVALID_DEV_ID				0xFFFFFFFF
#define INVALID_DEV_TYPE			0xFFFF
// 无效物理地址
#define INVALID_PHY_ADDR			0xFFFF

enum SMART_TYPE
{
	// 灯光
	ST_LIGHT_A = 1,
	ST_LIGHT_B = 2,
	ST_LIGHT_C = 3,
	ST_LIGHT_D = 4,
	// 调光
	ST_DIMMER_A = 11,
	ST_DIMMER_B,
	ST_DIMMER_C,
	ST_DIMMER_D,
	// 窗帘 卷帘、开合、折帘、百叶
	ST_CURTAIN_A = 21,		
	ST_CURTAIN_B,
	ST_CURTAIN_C,
	ST_CURTAIN_D,
	// 窗户 上悬、中悬、下悬、天窗
	ST_WINDOW_A = 31,
	ST_WINDOW_B,
	ST_WINDOW_C,
	ST_WINDOW_D,
	// 插座
	ST_OUTLET_A = 41,
	ST_OUTLET_B,
	ST_OUTLET_C,
	ST_OUTLET_D,
	// 排扇
	ST_FAN_A = 51,
	ST_FAN_B,
	ST_FAN_C,
	ST_FAN_D,
	// 空调
	ST_AC_A = 61,
	ST_AC_B,
	ST_AC_C,
	ST_AC_D,
	//红外空调
	ST_IRAIR_A = 66,
	ST_IRAIR_B,
	ST_IRAIR_C,
	ST_IRAIR_D,	
	// 地暖
	ST_HEAT_A = 71,
	ST_HEAT_B,
	ST_HEAT_C,
	ST_HEAT_D,
	// 新风
	ST_WIND_A = 81,
	ST_WIND_B,
	ST_WIND_C,
	ST_WIND_D,
	// 电视
	ST_TV_A = 91,
	ST_TV_B,
	ST_TV_C,
	ST_TV_D,
	// 背景音乐
	ST_MUSIC_A = 101,
	ST_MUSIC_B,
	ST_MUSIC_C,
	ST_MUSIC_D,
	// 门锁
	ST_LOCK_A = 111,
	ST_LOCK_B,
	ST_LOCK_C,
	ST_LOCK_D,	
	// 情景
	ST_SCENE_A = 201,
	ST_SCENE_B,
	ST_SCENE_C,
	ST_SCENE_D,
	ST_SCENE_E,
	ST_SCENE_F,
	ST_SCENE_G,
	ST_SCENE_H,
	ST_SCENE_I,
	ST_SCENE_J,
	ST_SCENE_K,
	ST_SCENE_L,
	ST_SCENE_M,
	ST_SCENE_N,
	ST_SCENE_O,
	ST_SCENE_P,
	ST_SCENE_Q,
	ST_SCENE_R,
	ST_SCENE_S,
	ST_SCENE_T,
	ST_SCENE_U,
	ST_SCENE_V,
	ST_SCENE_W,
	ST_SCENE_X,
	ST_SCENE_Y,
	ST_SCENE_Z,  
};

enum
{
	SCMD_OPEN = 0x01,					// 开
	SCMD_DIMMER_OPEN,					// 开(1-100%)
	SCMD_CLOSE,							// 关
	SCMD_DELAY_OPEN,					// 延时开(1～65535秒)	
	SCMD_DELAY_CLOSE,					// 延时关(1～65535秒)
	SCMD_DELAY_OPEN_CLOSE,				// 打开之后，延时关(1～65535秒)
	SCMD_GRADUALLY_OPEN,				// 渐亮(1-100%)
	SCMD_GRADUALLY_CLOSE,				// 渐灭
	SCMD_CURTAIN_OPEN,					// 窗帘开(1-100%)	
	SCMD_CURTAIN_CLOSE = 0x10,			// 窗帘关
	SCMD_CURTAIN_STOP,					// 窗帘停
	SCMD_OPEN_CLOSE,					// 开/关(同一按键控制开关， 0同步 1不同步)
	SCMD_BRIGHT_ADD,					// 调光+(1-100%)	
	SCMD_BRIGHT_SUB,					// 调光-(1-100%)	
	SCMD_TEMP_ADD,						// 调温+(1～127（－1～－127℃）128～255（0～127℃）)
	SCMD_TEMP_SUB,						// 调温-(1～127（－1～－127℃）128～255（0～127℃）)
	SCMD_SPEED_ADD,						// 调速+(1-100%)
	SCMD_SPEED_SUB,						// 调速-(1-100%)
	SCMD_SCENE,							// 场景控制
	SCDM_INFRARED = 0x20,				// 红外(1~255)
	SCMD_AC = 0x32,						// 空调
	SCMD_MUSIC = 0x33,					// 音乐
	SCMD_SAFE = 0xA0,					// 安防
	//========================================================================================
	// 空调参数说明
    //Bit15～Bit14（状态）：1：开启 2：停止 3：待机 0：保持不变
    //Bit13～Bit10（模式）：1：制热 2：制冷 3：通风 4: 睡眠 5：除湿 6：干燥 7：化霜 8：辅热 9：节能 10：换气 11：超强 12：自动运行 13：手动14：定时 0：缺省 0：保持不变
    //Bit9～Bit8（功能）：1：扫风 2：叶片方向（水平） 3：叶片方向（垂直）0：保持不变
    //Bit7～Bit5（风速）：1：极低 2：低 3：中 4：高 5：极高 6：自动档（风速）7：风速停止 0：保持不变
    //Bit4～Bit0（温度）：1～31（10℃～40℃）0：保持不变
	//新风设备参数扩展：
	//参数3～4：Bit15～Bit0 
	//设定VOC：1～65535ppm（1～65535 表示0.1～6553.5ppm）0：保持不变
	//参数5～6：Bit15～Bit0
	//设定PM2.5：1～65535ug/m3 0：保持不变
	//温湿度设备参数扩展：
	//参数3：Bit7～Bit0 设定温度：1～127（－1～－127℃）128～255（0～127℃）0：保持不变
	//参数4：Bit7～Bit0 设定湿度：1～100％ 0：保持不变
	//========================================================================================
	// 音乐参数说明
	//Bit15～Bit14（状态）：1:开启 2:停止 3:待机0：保持不变
	//Bit13～Bit10（功能）：1:播放 2:暂停 3:播放/暂停 4:上一曲 5:下一曲 6:音量+ 7:音量-0：保持不变
	//Bit9～Bit8（音效）：1:普通 2:流行 3:摇滚0：保持不变
	//Bit7～Bit5（音源）：1:CD 2:FM1 3:FM2 4:MP3 5:AUX 6:网络 7: iPhone/iPod0：保持不变
	//Bit4～Bit0（音量）： 1～31（1～100％）0：保持不变
};


enum 
{
	CURTAIN_ALL_CLOSE = 1,		// 全关
	CURTAIN_HALF_OPEN = 2,		// 半开
	CURTAIN_ALL_OPEN  = 3		// 全开	
};

enum 
{
	WIND_HIGH 		= 1,		// 高风
	WIND_MIDDLE 	= 2,		// 中风
	WIND_LOW  		= 3			// 低风	
};

/******* 空调状态 *****************************************************************************************************/
#define AC_MODE_MAX				14
#define AC_FUNC_MAX				4
#define AC_SPEED_MAX			7
#define AC_TEMPRATRUE_MAX		22
#define AC_TEMPRATURE_INDEX		14

#define AC_STATUS_ON			1
#define AC_STATUS_OFF			2
/**********************************************************************************************************************/

/******* 新风状态 *****************************************************************************************************/
#define WIND_STATUS_ON			1
#define WIND_STATUS_OFF			2
/**********************************************************************************************************************/


/******* 地暖状态 *****************************************************************************************************/
#define HEAT_MODE_MAX           1
#define HEAT_TEMPRATURE_INDEX   14
/**********************************************************************************************************************/

/*******背景音乐状态***************************************************************************************************/
#define MUSIC_STATUS_ON         1
#define MUSIC_STATUS_OFF        2

#define MUSIC_STATUS_PAUSE      3
#define MUSIC_STATUS_LAST       4
#define MUSIC_STATUS_NEXT       5
#define MUSIC_STATUS_ADD        6
#define MUSIC_STATUS_SUB        7


#define MUSIC_VOICE_MAX         31
/**********************************************************************************************************************/


void InitSmartDev();
void InitSmartTimer();

void UpdatSmartTimerSet();

void ResetSmartDev();
void ResetSmartTimer();

void DeleteSmartTimer();

void SetSmartInit(DWORD flag);

DWORD GetSmartAddr();
void SetSmartAddr(DWORD addr);

DWORD GetSmartPwd();
void SetSmartPwd(DWORD pwd);

DWORD GetSmartRoute();
void SetSmartRoute(DWORD route);

DWORD GetSmartEnvId();
void SetSmartEnvId(DWORD id);

void SetStudyChan(DWORD i);
void SetStudyNum(DWORD i);


LightStudy* GetLightStudy(int i);
void SetLightStudy(LightStudy lightstudy, int i);

SmartDev* GetSmartDev(DWORD* dwCount);
PSmartTimer GetTimerHead();
PSmartTimer GetTimerNext(LPLISTOBJ pobject);
DWORD GetTimerCount();

void GetTimerShow(PSmartTimer pItem);

void AddSmartDev(ECB_DATA* pECB);
void AddSceneCtrlList(ECB_DATA* pECB);
void AddSmartTimer(ECB_DATA* pECB);    

void SmartTimeSync();

void SetLightStudySync(ECB_DATA* pECB);
void AddLightStudy(ECB_DATA* pECB);

void SetStatusByScene(BYTE scene);
void SetStatusBySync(DEVICE *device, BYTE cmd, WORD param);
DEVICE *GetSmartDevByDev(DEVICE *device);

void SetStatusByAll(BYTE cmd);


char* GetSmartPngCurtain(DWORD type, DWORD status);
char *GetSmartPngWind(DWORD type, DWORD status);
char* GetSmartPng(DWORD type, DWORD status);
char* GetSmartPngMusic(DWORD status);
char* GetSmartPngPoint(DWORD status);
char* GetSmartPngOnOff(DWORD status);
char* GetSmartAutoOnOff(DWORD status);
char* GetSmartPngSelect(DWORD status);

void* SmartTimerThread(void* pParam);

BOOL SmartComSend(char* buf, int len);
void SmartSendCmd(char* buf, int len);
void* SmartSendThread(void* pParam);
void* SmartComThread(void* pParam);
void SmartComRecv(ECB_DATA* pECB);
void DPCreateSmartSend();
void DPCreateSmartCom();
