#include "roomlib.h"
#include "SmartConfig.h"
#include "dpcom.h"

#ifdef DPCE
const char SMART_COM[] = "COM2:";

#else
const char SMART_COM[] = "/dev/ttyS1";
#endif

static StaticLock g_CS;
static BOOL		g_bRun;
static HANDLE	g_hCom;
static BYTE		g_lastSeq;
static WORD		g_Version;
static DWORD	g_SmartDevID;
static DWORD	g_SoftVersion;
static WORD		g_SmartDevAddr;
static WORD		g_SmartDevType;
static HANDLE	g_hSmartServer;


int G_MUSIC_PAUSE_FLAG = 0;		//音乐暂停标志

int Ctl_Flag = 0;            // 2018.2.24添加，用于修改在一级界面APP控制

int TIME_FLAG = 0;
extern int Heat_Flag;
WORD status1;                // 2018.2.26添加，用于刷空调等的状态
int  G_Temp = 0;         //用于存放获取的温度

WORD CO2 = 0; 			//用于存放新风的CO2
WORD PM25= 0; 			//用于存放新风的PM24

//======================================================
//** 函数名称: GetVersion
//** 功能描述: 获取版本号
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
DWORD GetVersion()
{
	return g_Version;
}

//======================================================
//** 函数名称: GetDevType
//** 功能描述: 获取设备类型
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
DWORD GetDevType()
{
	return g_SmartDevType;
}


//======================================================
//** 函数名称: GetSoftVer
//** 功能描述: 获取软件版本号
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
DWORD GetSoftVer()
{
	return g_SoftVersion;
}

//======================================================
//** 函数名称: SmartPrintf
//** 功能描述: 打印调试信息
//** 输　入: pdata len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void SmartPrintf(char* pdata, DWORD len)
{
	// 打印
	if (len > 64)
		len = 64;

	int buf_len = 0;
	char buf[256] = {0};
	for(int i = 0; i < len; i++)
	{
		buf_len += sprintf(buf + buf_len, "%02x ", (BYTE)pdata[i]);  
	}
	printf("%s\r\n", buf);
	//printf("%s\n",SMART_COM);
}

//======================================================
//** 函数名称: SendCommand
//** 功能描述: 发送串口命令
//** 输　入: hCom buf len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void SendCommand(HANDLE hCom, char* buf, int len)
{
	SmartPrintf(buf, len);                    // 打印发送的数据

	g_CS.lockon();
	SendComm(hCom, buf, len);                 // 向文件中写数据
	g_CS.lockoff();
}

//======================================================
//** 函数名称: GetCheckNum
//** 功能描述: 获取校验和
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static BYTE GetCheckNum(BYTE* buf, int len)   // 校验和函数
{
	BYTE check_num = buf[0];
	for(int i = 1; i < len; i++)
	{
		check_num ^= buf[i];
	}
	return check_num;
}

//======================================================
//** 函数名称: SmartReportID
//** 功能描述: 家居上报ID
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SmartReportID()                                                    // 上报设备ID
{
	char buf[64];
	ECB_DATA* pData = (ECB_DATA *)buf;
	pData->head = ECB_HEAD;
	pData->length = 15;
	pData->src = 0x0000;
	pData->dst = 0xFFFF;
	pData->type = ECB_TYPE_ATTR;
	pData->cmd = ECB_REPORT_INFO;
	memcpy(&pData->data[0], &g_SmartDevID, 4);
	memcpy(&pData->data[4], &g_SmartDevType, 2);
	memcpy(&pData->data[6], &g_Version, 2);
	pData->data[8] = GetCheckNum(&pData->length, pData->length);		// 校验和
	SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: HandleInit
//** 功能描述: 处理初始化
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleInit()
{
	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;
	pReply->length = 7;
	pReply->src = 0x0000;
	pReply->dst = 0xFFFF;
	pReply->type = ECB_TYPE_ALL;
	pReply->cmd = ECB_INIT_ALL_ACK;
	pReply->data[0] = GetCheckNum(&pReply->length, pReply->length);		// 校验和

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	// 清空之前的设备配置
	ResetSmartDev();
	DeleteSmartTimer();   // 删除定时事件
	DPPostMessage(MSG_PRIVATE, MAIN_APPID, MSG_SMART_UPDATE, 0);
}

//======================================================
//** 函数名称: HandleReset
//** 功能描述: 处理初始化
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleReset()             
{
	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;
	pReply->length = 7;
	pReply->src = 0xFFFF;
	pReply->dst = 0xFFFF;
	pReply->type = ECB_TYPE_ALL;
	pReply->cmd = ECB_RESET_ALL_ACK;
	pReply->data[0] = GetCheckNum(&pReply->length, pReply->length);		// 校验和

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	// 清空之前的设备配置
	ResetSmartDev();
	DPPostMessage(MSG_PRIVATE, MAIN_APPID, MSG_SMART_UPDATE, 0);
}

//======================================================
//** 函数名称: HandleSearchDev
//** 功能描述: 处理搜索设备
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleSearchDev(ECB_DATA* pECB)     //
{
	if(pECB->data[0] == g_lastSeq)
		return;
	g_lastSeq = pECB->data[0];

	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;
	pReply->length = 16;
	pReply->src = 0x0000;
	pReply->dst = 0xFFFF;
	pReply->type = ECB_TYPE_ATTR;
	pReply->cmd = ECB_SEARCH_DEV_ACK;
	pReply->data[0] = pECB->data[0];				// 序号
	memcpy(&pReply->data[1], &g_SmartDevID, 4);		// 器件ID
	memcpy(&pReply->data[5], &g_SmartDevType, 2);	// 器件类型
	memcpy(&pReply->data[7], &g_Version, 2);		// 器件版本
	pReply->data[9] = GetCheckNum(&pReply->length, pReply->length);		// 校验和

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: HandleReportAck
//** 功能描述: 处理上报应答
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleReportAck(ECB_DATA* pECB)
{
	// 上报ID回复
	if(0 != memcmp(&g_SmartDevID, &pECB->data[0], 4))
		return;

	g_SmartDevAddr = pECB->dst;
	SetSmartAddr(g_SmartDevAddr);                    //上报成功后更新了一次地址。

	// 通知上层应用，上报成功
	DPPostMessage(MSG_PRIVATE, PRJ_REPORT_APPID, 0, 0);
}

//======================================================
//** 函数名称: HandleSetAddr
//** 功能描述: 处理设置物理地址
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleSetAddr(ECB_DATA* pECB)
{
	if(0 != memcmp(&g_SmartDevID, &pECB->data[0], 4))
		return;
	g_SmartDevAddr = pECB->dst;
	SetSmartAddr(g_SmartDevAddr);

	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;
	pReply->length = 7;
	pReply->src = g_SmartDevAddr;
	pReply->dst = 0xFFFF;
	pReply->type = ECB_TYPE_ATTR;
	pReply->cmd = ECB_SET_ADDR_ACK;
	pReply->data[0] = GetCheckNum(&pReply->length, pReply->length);		// 校验和

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: HandleSetGroupAddr
//** 功能描述: 处理设置组地址
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleSetGroupAddr(ECB_DATA* pECB)
{
	// 组地址设置
	if(pECB->dst == g_SmartDevAddr)
	{
		char buf[64];
		ECB_DATA* pReply = (ECB_DATA *)buf;
		pReply->head = ECB_HEAD;
		pReply->length = 8;
		pReply->src = g_SmartDevAddr;
		pReply->dst = 0xFFFF;
		pReply->type = ECB_TYPE_ATTR;
		pReply->cmd = ECB_GET_GROUP_ADDR_ACK;
		pReply->data[0] = pECB->data[0];		// 编号
		pReply->data[1] = GetCheckNum(&pReply->length, pReply->length);		// 校验和

		SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
	}
	else
	{
		AddSmartAddrList(pECB);	
	}
}

//======================================================
//** 函数名称: HandleSetDevConfig
//** 功能描述: 处理设备配置
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleSetDevConfig(ECB_DATA* pECB)
{
	if(pECB->dst != g_SmartDevAddr)
		return;

	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;
	pReply->length = 9;
	pReply->src = g_SmartDevAddr;
	pReply->dst = 0xFFFF;
	pReply->type = ECB_TYPE_ATTR;
	pReply->cmd = ECB_SET_DEV_CFG_ACK;
	pReply->data[0] = pECB->data[0];		// 编号
	pReply->data[1] = pECB->data[1];		// 序号
	pReply->data[2] = GetCheckNum(&pReply->length, pReply->length);		// 校验和

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	if(pECB->data[1] != 0)
	{
		AddSmartDev(pECB);
		DPPostMessage(MSG_PRIVATE, MAIN_APPID, MSG_SMART_UPDATE, 0);
	}
}

//======================================================
//** 函数名称: HandleSetScene
//** 功能描述: 处理设置情景
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleSetScene(ECB_DATA* pECB)
{
	if(pECB->dst != g_SmartDevAddr)
		return;

	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;
	pReply->length = 8;
	pReply->src = g_SmartDevAddr;
	pReply->dst = 0xFFFF;
	pReply->type = ECB_TYPE_ATTR;
	pReply->cmd = ECB_SET_SCENE_ACK;
	pReply->data[0] = pECB->data[0];		// 编号
	pReply->data[1] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	// 场景控制列表
	AddSceneCtrlList(pECB);
}

//======================================================
//** 函数名称: HandleSetTimer
//** 功能描述: 处理设置时间
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleSetTimer(ECB_DATA* pECB)
{
	if(pECB->dst != g_SmartDevAddr)
		return;

	char buf[64];
	BYTE temp = 0;
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;
	pReply->length = 9;
	pReply->src = g_SmartDevAddr;
	pReply->dst = 0xFFFF;
	pReply->type = ECB_TYPE_ATTR;
	pReply->cmd = ECB_SET_TIMER_ACK;
	pReply->data[0] = pECB->data[0];		// 编号
	pReply->data[1] = pECB->data[1];		// 序号
	pReply->data[2] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	if(temp != pECB->data[2]) {

		temp = pECB->data[2];

		if(pECB->data[1] == 1) {
			
			DeleteSmartTimer();
		}

		if(pECB->data[1] != 0)
		{
			AddSmartTimer(pECB);                // 添加定时事件
		}
	}
	
}

//======================================================
//** 函数名称: Get_Local_Time
//** 功能描述: 获取本地时间
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void Get_Local_Time()
{
	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;
	pReply->length = 8;
	pReply->src = g_SmartDevAddr;
	pReply->dst = 0xFFFF;
	pReply->type = ECB_TYPE_SYNC;
	pReply->cmd = ECB_SYNC_TIME;
	pReply->data[0] = 1;		// 编号
	pReply->data[1] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

}

//======================================================
//** 函数名称: HandleSetTime
//** 功能描述: 设置时间
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleSetTime(ECB_DATA* pECB)
{
	if(pECB->dst != g_SmartDevAddr)
		return;

	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;
	pReply->length = 8;
	pReply->src = g_SmartDevAddr;
	pReply->dst = 0xFFFF;
	pReply->type = ECB_TYPE_ATTR;
	pReply->cmd = ECB_SET_TIME_ACK;
	pReply->data[0] = pECB->data[0];		// 编号
	pReply->data[1] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
	
	// 设置时间
	SYSTEMTIME systime = {0};
	memcpy(&systime.wYear, &pECB->data[1], 2);
	systime.wYear		= htons(systime.wYear);
	systime.wMonth		= pECB->data[3];
	systime.wDay		= pECB->data[4];
	systime.wDayOfWeek	= pECB->data[5] % 7;
	systime.wHour		= pECB->data[6];
	systime.wMinute		= pECB->data[7];
	systime.wSecond		= pECB->data[8];
	TIME_FLAG = 1;
#ifndef _DEBUG
	DPSetLocalTime(&systime);
#endif
}

//======================================================
//** 函数名称: HandleSetTime222
//** 功能描述: 设置时间
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleSetTime222(ECB_DATA* pECB)
{
	if(pECB->dst != g_SmartDevAddr)
		return;

	
	// 设置时间
	SYSTEMTIME systime = {0};
	memcpy(&systime.wYear, &pECB->data[1], 2);
	systime.wYear		= htons(systime.wYear);
	systime.wMonth		= pECB->data[3];
	systime.wDay		= pECB->data[4];
	systime.wDayOfWeek	= pECB->data[5] % 7;
	systime.wHour		= pECB->data[6];
	systime.wMinute		= pECB->data[7];
	systime.wSecond		= pECB->data[8];
	TIME_FLAG = 1;
#ifndef _DEBUG
	DPSetLocalTime(&systime);
#endif
}

static void HandleSetDevCfg(ECB_DATA* pECB)
{

	
}

//======================================================
//** 函数名称: HandleControl
//** 功能描述: 处理控制数据
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleControl(ECB_DATA* pECB)
{
	
	if(pECB->data[0] == SCMD_SCENE)
	{
		// 其他面板进行情景控制, 不用管回复，直接刷新状态
		SetStatusByScene(pECB->data[2]);
	}

	if(pECB->data[0] == SCMD_AC ||pECB->data[0] ==SCMD_MUSIC) {

		if(pECB->data[0] == SCMD_AC)         // 2018.2.24添加

			Ctl_Flag = AIR_TYPE;

		else if(pECB->data[0] ==SCMD_MUSIC)  // 2018.2.24添加

			Ctl_Flag = MUSIC_TYPE;

		WORD status = (pECB->data[1]<<8 | pECB->data[2]);
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC ,pECB->dst ,status); 
	}
	
	// APP在一级界面控制图标亮灭
	else if(pECB->data[0] == 0x01) {         

		Ctl_Flag = ALL_TYPE;
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_S ,pECB->dst ,0x01); 	
	}
	
	// APP在一级界面控制图标亮灭
	else if(pECB->data[0] == 0x03) {         
	
		Ctl_Flag = ALL_TYPE;
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_S ,pECB->dst ,0x03); 
	}
}

//======================================================
//** 函数名称: HandleControlAck
//** 功能描述: 处理控制响应
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleControlAck(ECB_DATA* pECB)
{
	ECB_CTRL_ACK* pAck = (ECB_CTRL_ACK *)pECB->data;
	if(pAck->cmd == SCMD_SCENE)
	{
		// 如果是场景控制，需要查找场景列表来获取控制命令
		// 2017.09.13 张广说不用根据回复判断，直接刷新状态即可
		// SetStatusByList(pECB->src, pAck->param >> 8);
	}
	else
	{
		SetStatusByAck(pAck->addr, pAck->cmd, htons(pAck->param));

		DPPostMessage(MSG_BROADCAST, SMART_STATUS_ACK, pAck->addr, 0);	
	}
}

//======================================================
//** 函数名称: HandleStatusSync
//** 功能描述: 处理状态同步
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleStatusSync(ECB_DATA* pECB)
{

	WORD status;
	ECB_CTRL_ACK* pAck = (ECB_CTRL_ACK *)&pECB->data[1];
	SetStatusBySync(pAck->addr, pAck->cmd, htons(pAck->param));
	
	if(pAck->cmd == SCMD_AC) {

		Ctl_Flag = AIR_TYPE;
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, pAck->addr, htons(pAck->param));
	}

	else {

		DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, pAck->addr, 0);
	}
	
/*   	if(Heat_Flag == 1) {   // 2018.1.10只有当地暖界面打开，数据才同步

		status = (pECB->data[4]<<8 | pECB->data[5]);
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, pAck->addr, status);
   	} */	
}

//======================================================
//** 函数名称: HandleGetStatus
//** 功能描述: 处理获取状态--普通设备
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleGetStatus(ECB_DATA* pECB)
{
	if(pECB->dst != g_SmartDevAddr)
		return;

	ECB_CTRL_ACK* pAck = (ECB_CTRL_ACK *)&pECB->data[1];
	WORD status = htons(pAck->param);
	switch(pAck->cmd)
	{
	case SCMD_OPEN:
		if(status == 0)
			status = 100;
		
		break;
	case SCMD_CLOSE:
	case SCMD_CURTAIN_CLOSE:
		status = 0;
		break;
	case SCMD_DIMMER_OPEN:
	case SCMD_CURTAIN_OPEN:
		break;
	case SCMD_MUSIC:
		G_MUSIC_PAUSE_FLAG = 1;
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, pAck->addr, status);
		break;
	default:
		return;
	}

	DPPostMessage(MSG_BROADCAST, SMART_STATUS_GET, pAck->addr, status);
}

// 2018.2.26添加
//======================================================
//** 函数名称: HandleGetStatus_Air
//** 功能描述: 处理获取状态--空调
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleGetStatus_Air(ECB_DATA* pECB)
{
	//AC_DATA *status;
	WORD status = 0;
	BYTE MIDparam;
	if(pECB->dst != g_SmartDevAddr)
		return;

	ECB_CTRL_ACK_AIR* pAck = (ECB_CTRL_ACK_AIR *)&pECB->data[1];
//	WORD status = htons(pAck->param);
	// 1.1 取开关状态:
	MIDparam = (pAck->param1 & 0xf0 );

	if(MIDparam == 0x10)
		status |= 0x4000;
	
	else if(MIDparam == 0x20)
		status |= 0x8000;

	// 1.2 取模式状态:
	MIDparam = (pAck->param1 & 0x0f );
	status  |= (MIDparam<<10);

	// 2 扫风位默认为缺省模式即可，其次判断风速
	MIDparam = (pAck->param2 & 0x0f );
	status  |= (MIDparam<<5);

	// 3 室内当前温度(这个就需要另一个变量来接收了)
	G_Temp = (pAck->param3 - 128);
	//获取新风的CO2
	if(pECB->length >= 24)  //当数据长度大于  ，说明该是新风，只能这样判断了
	{
		CO2 = (pAck->param7<<8) | (pAck->param8); 
		PM25 =  (pAck->param11<<8) | (pAck->param12); 
	}
		
	// 4 设置温度
	MIDparam = (pAck->param4 - 128) - 9;

	if(MIDparam < 6)
		MIDparam = 6;
	
	status  |= MIDparam&0x1f;
		
	DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, pAck->addr, status);
} 

//======================================================
//** 函数名称: HandleAttribute
//** 功能描述: 配置数据协议
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleAttribute(ECB_DATA* pECB)
{
	switch(pECB->cmd)
	{
		case ECB_REPORT_INFO_ACK:              //上报ID后的回复，是成功还是失败。
			HandleReportAck(pECB);
			break;
		case ECB_SEARCH_DEV:                   //搜索总线器件。（这里貌似是APP自动搜索ECB总线上的设备）
			HandleSearchDev(pECB);
			break;
		case ECB_SET_ADDR:                     //设置器件物理地址
			HandleSetAddr(pECB);
			break;
		case ECB_SET_GROUP_ADDR:               //设置器件之通路组地址
			HandleSetGroupAddr(pECB);
			break;
		case ECB_SET_DEV_CFG:                  //设置器件之控制信息
			HandleSetDevConfig(pECB);
			break;
		case ECB_SET_SCENE:                    //设置场景模式控制
			HandleSetScene(pECB);
			break;
		case ECB_SET_TIMER:                    //设置定时控制
			HandleSetTimer(pECB);
			break;
		case ECB_SET_TIME:                     //设置器件时间
			HandleSetTime(pECB);
			break;
			break;

	}
}

//======================================================
//** 函数名称: HandleContrl
//** 功能描述: 控制数据协议
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleContrl(ECB_DATA* pECB)
{
	switch(pECB->cmd)
	{
	case ECB_CTRL_DEV:
		HandleControl(pECB);
		break;
	case ECB_CTRL_DEV_ACK:
		HandleControlAck(pECB);
		break;

	}
}

//======================================================
//** 函数名称: HandleSync
//** 功能描述: 同步数据协议
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void HandleSync(ECB_DATA* pECB)
{
	switch(pECB->cmd)  
	{
	case ECB_SYNC_STATUS:
		HandleStatusSync(pECB);
		break;
	case ECB_SYNC_TIME_ACK:
		HandleSetTime222(pECB);
		break;
	case ECB_GET_STATUS_ACK:
		HandleGetStatus(pECB);
		break;
	case ECB_GET_STATUS_AIR_ACK:
		HandleGetStatus_Air(pECB);
		break;
	}
}

//======================================================
//** 函数名称: SmartDataProc
//** 功能描述: 家居协议数据处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void SmartDataProc(ECB_DATA* pECB)
{
	switch(pECB->type)              //判断这个命令的类型。
	{
		case ECB_TYPE_ATTR:         //属性。
			HandleAttribute(pECB);
			break;
		case ECB_TYPE_CTRL:         //控制命令ECB总线->液晶面板。
			HandleContrl(pECB);
			break;
		case ECB_TYPE_SYNC:         //同步命令ECB总线->液晶面板。  
			HandleSync(pECB);
			break;
		case ECB_TYPE_ALL:
			if(ECB_INIT_ALL == pECB->cmd)
			{
				HandleInit();
			}
			else if(ECB_RESET_ALL == pECB->cmd)
			{
				HandleReset();
				
			}
			break;
		default:
			DBGMSG(DPINFO, "SmartDataProc recv error type:%x\r\n", pECB->type);
			break;
	}
}

//======================================================
//** 函数名称: SmartRecvProc
//** 功能描述: 家居服务接收处理
//** 输　入: buf len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void SmartRecvProc(char* buf, int len)        //获得协议帧。
{
	int ptr = 0;
	ECB_DATA* pECB = NULL;

	// 寻找 0x5a 0xa5
	while(ptr + ECB_DATA_MIN_LEN <= len)
	{
		pECB = (ECB_DATA *)(buf + ptr);                     
		if(pECB->head != ECB_HEAD)                    //获取头指针。
		{
			ptr++;
			continue;
		}

		if(pECB->length + ECB_HEAD_LEN_LENGTH > len - ptr)
		{
			printf("SmartRecvProc error length:%d, recv len:%d\r\n", pECB->length, len - ptr);
			ptr++;
			continue;
		}

#ifndef DPCE
		if(pECB->data[pECB->length - ECB_DATA_HEAD] != GetCheckNum(&pECB->length, pECB->length))
		{
			printf("SmartRecvProc CheckNum fail\r\n");		
			ptr++;
			continue;
		}
#endif

		SmartDataProc(pECB);                   //获得协议帧后，进行数据的处理。 
		ptr += pECB->length + ECB_HEAD_LEN_LENGTH;
	}
}

//======================================================
//** 函数名称: SmartServer
//** 功能描述: 家居服务
//** 输　入: pParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static DWORD SmartServer(HANDLE pParam)
{
	int len;
	char buf[1024];

	DBGMSG(DPINFO, "SmartServer start\r\n");

	if(g_SmartDevAddr != INVALID_ECB_ADDR)
	{
		// 先同步一次时间
		SmartSyncTime();                       //经过测试发现，貌似这里开机后，时间与北京时间不符。
	}

	while(g_bRun)
	{
		len = ReadComm(g_hCom, buf, 1023);     //从ECB总线上读取数据的长度。
		if(len < 0)
			break;
		else if(len == 0)
			continue;

		SmartPrintf(buf, len);                 //打印出从ECB总线上读出的数据
		SmartRecvProc(buf, len);               //经过一些列处理取得协议帧及长度。
	}

	DBGMSG(DPINFO, "SmartServer Stop\r\n");
	return 0;
}

//======================================================
//** 函数名称: SmartServerPC
//** 功能描述: 家居调试服务-pc
//** 输　入: pParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static DWORD SmartServerPC(HANDLE pParam)
{
	int len;
	char buf[1024];

	InitNetwork();
	SOCKET sockfd = UdpCreate(15680);

	while(g_bRun)
	{
		len = UdpRecv(sockfd, buf, 1024, 1000);
		if(len > 0)
		{
			SmartRecvProc(buf, len);
		}
	}

	return 0;
}

//======================================================
//** 函数名称: ReadDevInfo
//** 功能描述: 读取设备信息
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void ReadDevInfo()
{
	char fileName[MAX_PATH];
	sprintf(fileName, "%s/%s", USERDIR, "Version.dat");
	FILE* pFile = fopen(fileName, "rb");
	if(NULL == pFile)
	{
		sprintf(fileName, "%s/%s", FLASHDIR, "Version.dat");
		pFile = fopen(fileName, "rb");
	}

	if(pFile)
	{
		char buf[1024];
		fread(&g_Version, 1, 2, pFile);
		fseek(pFile, 0, SEEK_SET);
		fread(&g_SoftVersion, 1, 4, pFile);
		g_SoftVersion = htonl(g_SoftVersion);
		//fread(&g_SmartDevType, 1, 2, pFile);
		g_SmartDevType = 0x0228;
		fclose(pFile);
	}
	else
	{
		g_Version = 0x0020;
		g_SmartDevType = 0x0228;
		g_SoftVersion = 0x6000000;
		DBGMSG(DPERROR, "SmartServer open fail\r\n");
	}
}

//======================================================
//** 函数名称: SmartSyncTime
//** 功能描述: 同步时间
//** 输　入: addr
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SmartSyncTime()
{
	char buf[64];
	ECB_DATA* pData = (ECB_DATA *)buf;
	pData->head = ECB_HEAD;
	pData->length = 8;
	pData->src = g_SmartDevAddr;
	pData->dst = 0xFFFF;
	pData->type = ECB_TYPE_SYNC;
	pData->cmd = ECB_SYNC_TIME;
	pData->data[0] = 1;		// 编号
	pData->data[1] = GetCheckNum(&pData->length, pData->length);		// 校验和
	SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: SmartGetStatus
//** 功能描述: 获取状态--普通设备
//** 输　入: addr
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SmartGetStatus(WORD addr)
{
	char buf[64];
	ECB_DATA* pData = (ECB_DATA *)buf;
	pData->head = ECB_HEAD;
	pData->length = 10;
	pData->src = g_SmartDevAddr;
	pData->dst = addr;
	pData->type = ECB_TYPE_SYNC;
	pData->cmd = ECB_GET_STATUS;
	pData->data[0] = 1;		// 编号
	memcpy(&pData->data[1], &addr, 2);
	pData->data[3] = GetCheckNum(&pData->length, pData->length);		// 校验和
	SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

// 2018.2.26添加，用于同步空调的查询状态
//======================================================
//** 函数名称: SmartGetStatus_Air
//** 功能描述: 获取状态--空调
//** 输　入: addr
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SmartGetStatus_Air(WORD addr)
{
	char buf[64];
	ECB_DATA* pData = (ECB_DATA *)buf;
	pData->head = ECB_HEAD;
	pData->length = 10;
	pData->src = g_SmartDevAddr;
	pData->dst = addr;
	pData->type = ECB_TYPE_SYNC;
	pData->cmd = ECB_GET_STATUS_AIR;
	pData->data[0] = 1;		// 编号
	memcpy(&pData->data[1], &addr, 2);
	pData->data[3] = GetCheckNum(&pData->length, pData->length);		// 校验和
	SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: SendSmartCmd
//** 功能描述: 发送家居命令
//** 输　入: addr cmd param
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SendSmartCmd(WORD addr, BYTE cmd, WORD param)
{
	param = htons(param);

	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;                                            // 发送的头指针
	pReply->length = 10;                                                // 发送数据长度
	pReply->src = g_SmartDevAddr;                                       // 发送的源地址
	pReply->dst = addr;                                                 // 发送的目标地址
	pReply->type = ECB_TYPE_CTRL;                                       // 设备控制
	pReply->cmd = ECB_CTRL_DEV;
	pReply->data[0] = cmd;                                              // 控制命令类型
	memcpy(&pReply->data[1], &param, 2);
	pReply->data[3] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: StartSmartServer
//** 功能描述: 启动家居服务
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void StartSmartServer()
{
	if(g_bRun)
		return;

	g_hCom = OpenComm(SMART_COM, 115200, 8, 1000);
	if(g_hCom == NULL)
	{
		DBGMSG(DPINFO, "SmartServer OpenComm fail\r\n");
		return;
	}

	// 读取版本号,设备类型
	ReadDevInfo();   

	g_bRun = TRUE;
	g_SmartDevID = ReadCpuID();
	g_SmartDevAddr = GetSmartAddr();                                       
	g_hSmartServer = DPThreadCreate(0x4000, SmartServer, NULL, TRUE, 5);   //开启了智能家居服务协议线程。

#ifdef DPCE	
	g_SmartDevID = htonl(0xafbd52b6);
	g_SmartDevID = 0x4c91830a;
	DPThreadCreate(0x4000, SmartServerPC, NULL, TRUE, 5);	
#endif
}

//======================================================
//** 函数名称: StopSmartServer
//** 功能描述: 停止家居服务
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void StopSmartServer()
{
	g_bRun = FALSE;
	if(g_hSmartServer)
	{
		DPThreadJoin(g_hSmartServer);
		g_hSmartServer = NULL;
	}
	if(g_hCom)
	{
		CloseComm(g_hCom);
		g_hCom = NULL;
	}
}