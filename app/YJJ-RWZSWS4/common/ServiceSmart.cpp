#include "roomlib.h"
#include "SmartConfig.h"
#include "dpcom.h"

#ifdef DPCE
const char SMART_COM[] = "COM2:";
#else
const char SMART_COM[] = "/dev/ttyS1";
#endif

static StaticLock 	g_CS;
static BOOL			g_bRun;
static HANDLE		g_hCom;
static WORD			g_Version;
static DWORD		g_SmartDevID;
static DWORD		g_SoftVersion;
static WORD			g_SmartDevAddr;
static WORD			g_SmartDevType;
static DWORD		g_SmartDevPwd;
static DWORD		g_SmartDevRoute;
static DWORD		g_SmartEnvId;
static DWORD		g_SmartConfig;
static HANDLE		g_hSmartServer;
static DWORD		g_LightStudy;
static DWORD		g_cfgTime;
static DWORD        g_timeSync;
static int			g_EnvTemp;
static DWORD		g_DevSeq;
static DWORD		g_TimerSeq;

DWORD GetDevID()
{
    return g_SmartDevID;
}

WORD GetVersion()
{
    return g_Version;
}

WORD GetDevType()
{
    return g_SmartDevType;
}

DWORD GetSoftVer()
{
    return g_SoftVersion;
}

int GetEnvTemp()
{
    return g_EnvTemp;
}

DWORD GetTimeSync()
{
    return g_timeSync;
}

void SetTimeSync(DWORD status)
{
    g_timeSync = status;
}

DWORD GetCfgStatus()
{
    return g_SmartConfig;
}

void SetCfgStatus(DWORD status)
{
    g_SmartConfig = status;
}

DWORD GetLightStudy()
{
    return g_LightStudy;
}

void SetLightStudy(DWORD status)
{
    g_LightStudy = status;
}

DWORD GetCfgTime()
{
    return g_cfgTime;
}

void SetCfgTime(DWORD time)
{
    g_cfgTime = time;
}


//======================================================
//** 函数名称: SmartConfig
//** 功能描述: 配置状态处理
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void SmartConfig()
{
    if (GetCfgStatus() == FALSE)
    {
        SetCfgStatus(TRUE);
        DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
        //ResetSmartDev();
        g_DevSeq = 0;
        g_TimerSeq = 0;
        SetSmartInit(TRUE);
        ResetSmartTimer();
    }
    else
    {
        SetCfgTime(0);
    }
}

//======================================================
//** 函数名称: SmartPrintf
//** 功能描述: 数据打印
//** 输　入: pdata len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void SmartPrintf(char *pdata, DWORD len)
{
    // 打印
    if (len > 256)
        len = 256;

    int buf_len = 0;
    char buf[1024] = {0};
    for(DWORD i = 0; i < len; i++)
    {
        buf_len += sprintf(buf + buf_len, "%02x ", (BYTE)pdata[i]);
    }
    printf("%s\r\n", buf);
}

//======================================================
//** 函数名称: SendCommand
//** 功能描述: 发送串口数据
//** 输　入: hCom buf len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void SendCommand(HANDLE hCom, char *buf, int len)
{
    if (SmartComSend(buf, len))
        return;

    SmartPrintf(buf, len);                    // 打印发送的数据

    g_CS.lockon();
    SendComm(hCom, buf, len);                 // 向串口中写数据
    g_CS.lockoff();
}

//======================================================
//** 函数名称: GetCheckNum
//** 功能描述: 校验码
//** 输　入: buf len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static BYTE GetCheckNum(BYTE *buf, int len)
{
    BYTE check_num = buf[0];
    for(int i = 1; i < len; i++)
        check_num ^= buf[i];
    return check_num;
}

//======================================================
//** 函数名称: GetSeqNum
//** 功能描述: 序号
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static BYTE GetSeqNum()
{
    static BYTE g_seqnum;
    if (0 == g_seqnum++)
        g_seqnum = 1;
    return g_seqnum;
}

//======================================================
//** 函数名称: SmartSendCmd
//** 功能描述: 发送命令
//** 输　入: buf len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SmartSendCmd(char *buf, int len)
{
    SmartPrintf(buf, len);                    // 打印发送的数据

    g_CS.lockon();
    SendComm(g_hCom, buf, len);                 // 向串口中写数据
    g_CS.lockoff();
}

//======================================================
//** 函数名称: SmartTimeSync
//** 功能描述: 时间同步
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SmartTimeSync()
{
    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 18;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_SYNC;
    pReply->cmd = ECB_SYNC_TIME;
    pReply->pwd = g_SmartDevPwd;
    pReply->data[0] = 0x01;		// 通路编号
    memcpy(&pReply->data[1], &g_SmartDevID, 4);
    memcpy(&pReply->data[5], &g_SmartDevType, 2);
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: SmartReportID
//** 功能描述: 上报ID
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SmartReportID()                                              // 上报设备ID
{
    char buf[64];
    ECB_DATA *pData = (ECB_DATA *)buf;
    pData->head = ECB_HEAD;
    pData->length = 19;
    pData->src = g_SmartDevAddr;
    pData->dst = INVALID_PHY_ADDR;
    pData->type = ECB_TYPE_ATTR;
    pData->cmd = ECB_REPORT_INFO;
    pData->pwd = g_SmartDevPwd;
    memcpy(&pData->data[0], &g_SmartDevID, 4);
    memcpy(&pData->data[4], &g_SmartDevType, 2);
    memcpy(&pData->data[6], &g_Version, 2);
    pData->data[8] = GetCheckNum(&pData->length, pData->length);		// 校验和
    SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: HandleInit
//** 功能描述: 恢复出厂设置
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void SmartLightCtrl(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;
    if (pECB->data[0] < 0x01 || pECB->data[0] > 0x03)
        return;

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 22;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_CTRL;
    pReply->cmd = ECB_CTRL_DEV_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 10);
    pReply->data[10] = GetSeqNum();		// 序号
    pReply->data[11] = GetCheckNum(&pReply->length, pReply->length); 	// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    DWORD count;
    SmartDev *pSmartDev = GetSmartDev(&count);

    if (pECB->data[7] == SCMD_OPEN)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_OPEN;
        SetLightGpioVal(pECB->data[0], FALSE);	// 亮
    }
    else if (pECB->data[7] == SCMD_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// 灭
    }
    else if (pECB->data[7] == SCMD_DELAY_OPEN)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_OPEN;
        SetLightGpioVal(pECB->data[0], FALSE);	// 亮
    }
    else if (pECB->data[7] == SCMD_DELAY_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// 灭
    }
    else if (pECB->data[7] == SCMD_DELAY_OPEN_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// 灭
    }
    else if (pECB->data[7] == SCMD_OPEN_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd =
            (pSmartDev[pECB->data[0] - 1].cmd == SCMD_OPEN) ? SCMD_CLOSE : SCMD_OPEN;
        if (pSmartDev[pECB->data[0] - 1].cmd == SCMD_OPEN)
            SetLightGpioVal(pECB->data[0], FALSE);	// 亮
        else
            SetLightGpioVal(pECB->data[0], TRUE);	// 灭
    }
    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)&pSmartDev[pECB->data[0] - 1], 0);
}

//======================================================
//** 函数名称: HandleInit
//** 功能描述: 恢复出厂设置
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleInit(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 18;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_ATTR;
    pReply->cmd = ECB_INIT_ALL_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 7);
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    DPPostMessage(MSG_SYSTEM, RESET_MACH, 0, 0);
}

//======================================================
//** 函数名称: HandleReset
//** 功能描述: 重启设备
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleReset(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 18;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_ATTR;
    pReply->cmd = ECB_RESET_ALL_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 7);
    pReply->data[0] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
}

//======================================================
//** 函数名称: HandleSearchDev
//** 功能描述: 搜索设备应答
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleSearchDev(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 20;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_ATTR;
    pReply->cmd = ECB_SEARCH_DEV_ACK;
    pReply->pwd = g_SmartDevPwd;
    pReply->data[0] = pECB->data[0];				// 序号
    memcpy(&pReply->data[1], &g_SmartDevID, 4);		// 器件ID
    memcpy(&pReply->data[5], &g_SmartDevType, 2);	// 器件类型
    memcpy(&pReply->data[7], &g_Version, 2);		// 器件版本
    pReply->data[9] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: HandleReportAck
//** 功能描述: 上报ID应答
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleReportAck(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[0], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[4], 2))
        return;

    // 通知上层应用，上报成功
    //DPPostMessage(MSG_PRIVATE, PRJ_REPORT_APPID, 0, 0);
}

//======================================================
//** 函数名称: HandleSetAddrAndPwd
//** 功能描述: 配置地址和密码
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleSetAddrAndPwd(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    // 设置多功能传感器
    if (pECB->length == 27)
    {
        memcpy(&g_SmartEnvId, &pECB->data[11], 4);
        SetSmartEnvId(g_SmartEnvId);
    }
    else
    {
        // 设置物理地址
        g_SmartDevAddr = pECB->dst;
        SetSmartAddr(g_SmartDevAddr);

        // 设置系统密码
        g_SmartDevPwd = pECB->pwd;
        SetSmartPwd(g_SmartDevPwd);

        // 设置路由模式开关
        g_SmartDevRoute = pECB->data[8] & 0x01;
        SetSmartRoute(g_SmartDevRoute);
    }

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 18;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_SET;
    pReply->cmd = ECB_SET_ADDR_PWD_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 7);
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    if (pECB->length != 27)
        DPPostMessage(MSG_SYSTEM, RESET_ZIGBEE, 0, 0);
}

//======================================================
//** 函数名称: HandleSetDevice
//** 功能描述: 配置设备处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleSetDevice(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;
	
    // 配置状态
    SmartConfig();

    if (pECB->data[7] > g_DevSeq)
		g_DevSeq = pECB->data[7];
	else
		return;

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 19;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_ATTR;
    pReply->cmd = ECB_SET_DEV_CFG_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 8);
    //pReply->data[7] = GetSeqNum();		// 序号
    pReply->data[8] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    // 设备列表
	AddSmartDev(pECB);		
	DPPostMessage(MSG_PRIVATE, MAIN_APPID, MSG_SMART_UPDATE, 0);
  
}

//======================================================
//** 函数名称: HandleSetScene
//** 功能描述: 配置情景处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleSetScene(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    // 配置状态
    SmartConfig();

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 18;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_ATTR;
    pReply->cmd = ECB_SET_SCENE_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 7);
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    // 场景控制列表
    AddSceneCtrlList(pECB);
}

//======================================================
//** 函数名称: HandleSetTime
//** 功能描述: 配置定时处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleSetTimer(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    // 配置状态
    SmartConfig();
	
    if (pECB->data[7] > g_TimerSeq)
		g_TimerSeq = pECB->data[7];
	else
		return;

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 19;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_ATTR;
    pReply->cmd = ECB_SET_TIMER_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 8);
    //pReply->data[7] = GetSeqNum();		// 序号
    pReply->data[8] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    // 添加定时事件	
	AddSmartTimer(pECB);	
}

//======================================================
//** 函数名称: HandleSetTime
//** 功能描述: 配置时间处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleSetTime(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;
	
    // 配置状态
    SmartConfig();

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 18;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_ATTR;
    pReply->cmd = ECB_SET_TIME_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 7);
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    // 设置日期/星期/时间
    SYSTEMTIME systime = {0};
    memcpy(&systime.wYear, &pECB->data[7], 2);
    systime.wYear		= htons(systime.wYear);
    systime.wMonth		= pECB->data[9];
    systime.wDay		= pECB->data[10];
    systime.wDayOfWeek	= pECB->data[11] % 7;
    systime.wHour		= pECB->data[12];
    systime.wMinute		= pECB->data[13];
    systime.wSecond		= pECB->data[14];

    SetTimeSync(TRUE);
#ifndef _DEBUG
    DPSetLocalTime(&systime);
#endif
}

//======================================================
//** 函数名称: HandleCtrlAll
//** 功能描述: 秒开关处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleCtrlAll(ECB_DATA *pECB)
{
#if 0
    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 16;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_ALL;
    pReply->cmd = ECB_CTRL_ALL_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 4);
    pReply->data[4] = GetSeqNum();		// 序号
    pReply->data[5] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
#endif
	BYTE scene = (pECB->data[1] == SCMD_OPEN) ? 1 : 2;
	SetStatusByScene(scene);
    SetStatusByAll(pECB->data[1]);
}

//======================================================
//** 函数名称: HandleControl
//** 功能描述: 设备控制
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleControl(ECB_DATA *pECB)
{
    ECB_CTRL_ACK *pAck = (ECB_CTRL_ACK *)pECB->data;

    if (pAck->cmd == SCMD_SCENE && htons(pAck->param) > 0)
    {
        char buf[64];
        ECB_DATA *pReply = (ECB_DATA *)buf;
        pReply->head = ECB_HEAD;
        pReply->length = 22;
        pReply->src = g_SmartDevAddr;
        pReply->dst = INVALID_PHY_ADDR;
        pReply->type = ECB_TYPE_CTRL;
        pReply->cmd = ECB_CTRL_DEV_ACK;
        pReply->pwd = g_SmartDevPwd;
        memcpy(&pReply->data[0], &pECB->data[0], 10);
        pReply->data[10] = GetSeqNum(); 	// 序号
        pReply->data[11] = GetCheckNum(&pReply->length, pReply->length);	// 校验和
        SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
    }

    pAck->param = htons(pAck->param);

    if (pAck->cmd == SCMD_SCENE)
        SetStatusByScene(pAck->param);
    else
        SetStatusBySync(&pAck->device, pAck->cmd, pAck->param);

    if (pAck->cmd != SCMD_SCENE)
        SmartLightCtrl(pECB);

    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, 0, 0);
}

//======================================================
//** 函数名称: HandleControlAck
//** 功能描述: 设备控制应答
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleControlAck(ECB_DATA *pECB)
{
#if 0
    ECB_CTRL_ACK *pAck = (ECB_CTRL_ACK *)pECB->data;
    pAck->param = htons(pAck->param);

    if (pAck->cmd == SCMD_SCENE)
        return;

    DEVICE *device = GetSmartDevByDev(&pAck->device);
    if (NULL == device)
        return;

    SetStatusBySync(&pAck->device, pAck->cmd, pAck->param);
    //DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)device, pAck->param);
#endif
}

//======================================================
//** 函数名称: HandleSetTimeSync
//** 功能描述: 设备设置时间同步
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleSetTimeSync(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    SYSTEMTIME systime = {0};
    memcpy(&systime.wYear, &pECB->data[7], 2);
    systime.wYear		= htons(systime.wYear);
    systime.wMonth		= pECB->data[9];
    systime.wDay		= pECB->data[10];
    systime.wDayOfWeek	= pECB->data[11] % 7;
    systime.wHour		= pECB->data[12];
    systime.wMinute		= pECB->data[13];
    systime.wSecond		= pECB->data[14];

    SetTimeSync(TRUE);
#ifndef _DEBUG
    DPSetLocalTime(&systime);
#endif
}

//======================================================
//** 函数名称: HandleStatusSync
//** 功能描述: 设备状态同步处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleStatusSync(ECB_DATA *pECB)
{
    ECB_CTRL_ACK *pAck = (ECB_CTRL_ACK *)pECB->data;
    memcpy(&pAck->cmd, &pECB->data[9], 3);
    pAck->param = htons(pAck->param);

    DEVICE *device = GetSmartDevByDev(&pAck->device);
    if (NULL == device)
        return;

    SetStatusBySync(&pAck->device, pAck->cmd, pAck->param);
    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)device, pAck->param);
}

//======================================================
//** 函数名称: HandleStatusSensor
//** 功能描述: 传感器状态处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleStatusSensor(ECB_DATA *pECB)
{
    if (0x6e == pECB->data[5])
    {
        if (!memcmp(&g_SmartEnvId, &pECB->data[1], 4))
        {
            g_EnvTemp = pECB->data[12] - 128;
			DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, 0, 0);
        }
    }

    if (0x72 == pECB->data[5])
    {
        if (!memcmp(&g_SmartEnvId, &pECB->data[1], 4))
        {
            g_EnvTemp = pECB->data[6] - 128;
			DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, 0, 0);
        }
    }

    if (GetLightStudy() == TRUE)
        AddLightStudy(pECB);
    else
        SetLightStudySync(pECB);

}

//======================================================
//** 函数名称: HandleGetStatus_Air
//** 功能描述: 获取空调状态处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleGetStatus_Air(ECB_DATA *pECB)
{
    WORD status = 0;
    BYTE MIDparam;

    ECB_CTRL_ACK_AIR *pAck = (ECB_CTRL_ACK_AIR *)pECB->data;

    // 1.1 取开关状态:
    MIDparam = (pAck->param1 & 0xf0);
    if (MIDparam == 0x10)
        status |= 0x4000;
    else if (MIDparam == 0x20)
        status |= 0x8000;

    // 1.2 取模式状态:
    MIDparam = (pAck->param1 & 0x0f);
    status  |= (MIDparam << 10);

    // 2 扫风位默认为缺省模式即可，其次判断风速
    MIDparam = (pAck->param2 & 0x0f);
    status  |= (MIDparam << 5);

    // 3 室内当前温度(这个就需要另一个变量来接收了)
    if (pAck->param3 > 0)
    	g_EnvTemp = (pAck->param3 - 128);

    // 4 设置温度
    if (pAck->param4 > 0)
    {
	    MIDparam = (pAck->param4 - 128) - 9;
	    if(MIDparam < 6)
	        MIDparam = 6;
		status |= MIDparam;
    }

	SetStatusBySync(&pAck->device, pAck->cmd, status);

    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, 0, 0);
}

//======================================================
//** 函数名称: HandleGetStatus
//** 功能描述: 获取状态处理
//** 输　  入: pECB
//** 输　  出: 无
//**
//** 作　  者: HJ
//** 日　  期: 2018年11月19日
//======================================================
static void HandleGetStatus(ECB_DATA *pECB)
{
    ECB_CTRL_ACK *pAck = (ECB_CTRL_ACK *)pECB->data;

    //DEVICE *device = GetSmartDevByDev(&pAck->device);
    //if (NULL == device)
    //    return;

    if (SCMD_AC == pAck->cmd || WIND_DEV_TYPE == htons(pAck->device.type))
    {
        HandleGetStatus_Air(pECB);
        return;
    }

	pAck->param = htons(pAck->param);
    SetStatusBySync(&pAck->device, pAck->cmd, pAck->param);

    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, 0, 0);

    //DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)device, pAck->param);
}

//======================================================
//** 函数名称: HandleCfgCtrl
//** 功能描述: 配置控制处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleCfgCtrl(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;
    if (pECB->data[0] < 0x01 || pECB->data[0] > 0x03)
        return;

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;
    pReply->length = 22;
    pReply->src = g_SmartDevAddr;
    pReply->dst = INVALID_PHY_ADDR;
    pReply->type = ECB_TYPE_SYNC;
    pReply->cmd = ECB_CFG_CTRL_ACK;
    pReply->pwd = g_SmartDevPwd;
    memcpy(&pReply->data[0], &pECB->data[0], 10);
    pReply->data[10] = GetSeqNum(); 	// 序号
    pReply->data[11] = GetCheckNum(&pReply->length, pReply->length);	// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    DWORD count;
    SmartDev *pSmartDev = GetSmartDev(&count);

    if (pECB->data[7] == SCMD_OPEN)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_OPEN;
        SetLightGpioVal(pECB->data[0], FALSE);	// 亮
    }
    else if (pECB->data[7] == SCMD_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// 灭
    }
    else if (pECB->data[7] == SCMD_DELAY_OPEN)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_OPEN;
        SetLightGpioVal(pECB->data[0], FALSE);	// 亮
    }
    else if (pECB->data[7] == SCMD_DELAY_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// 灭
    }
    else if (pECB->data[7] == SCMD_DELAY_OPEN_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// 灭
    }
    else if (pECB->data[7] == SCMD_OPEN_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd =
            (pSmartDev[pECB->data[0] - 1].cmd == SCMD_OPEN) ? SCMD_CLOSE : SCMD_OPEN;
        if (pSmartDev[pECB->data[0] - 1].cmd == SCMD_OPEN)
            SetLightGpioVal(pECB->data[0], FALSE);	// 亮
        else
            SetLightGpioVal(pECB->data[0], TRUE);	// 灭
    }
    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)&pSmartDev[pECB->data[0] - 1], 0);
}

//======================================================
//** 函数名称: HandleAttribute
//** 功能描述: 配置属性
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleAttribute(ECB_DATA *pECB)
{
    switch(pECB->cmd)
    {
        case ECB_REPORT_INFO_ACK:              //上报ID后的回复，是成功还是失败。
            HandleReportAck(pECB);
            break;
        case ECB_INIT_ALL:					 //初始化
            HandleInit(pECB);
            break;
        case ECB_RESET_ALL:				     //复位
            HandleReset(pECB);
            break;
        case ECB_SEARCH_DEV:                   //搜索器件
            HandleSearchDev(pECB);
            break;
        case ECB_SET_DEV_CFG:                  //设置器件之控制信息
            HandleSetDevice(pECB);
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
        default:
            break;
    }
}

//======================================================
//** 函数名称: HandleContrl
//** 功能描述: 配置控制
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleContrl(ECB_DATA *pECB)
{
    switch(pECB->cmd)
    {
        case ECB_CTRL_DEV:
            HandleControl(pECB);
            break;
        case ECB_CTRL_DEV_ACK:
            HandleControlAck(pECB);
            break;
        default:
            break;
    }
}

//======================================================
//** 函数名称: HandleSync
//** 功能描述: 配置同步
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleSync(ECB_DATA *pECB)
{
    switch(pECB->cmd)
    {
        case ECB_SYNC_STATUS:
            HandleStatusSync(pECB);
            break;
        case ECB_SENSOR_REPORT:
            HandleStatusSensor(pECB);
            break;
        case ECB_SYNC_TIME_ACK:
            HandleSetTimeSync(pECB);
            break;
        case ECB_GET_STATUS_ACK:
            HandleGetStatus(pECB);
            break;
        case ECB_CFG_CTRL:
            HandleCfgCtrl(pECB);
            break;
        default:
            break;
    }
}

//======================================================
//** 函数名称: HandleSet
//** 功能描述: 配置设置
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleSet(ECB_DATA *pECB)
{
    switch(pECB->cmd)
    {
        case ECB_SET_ADDR_PWD:
            HandleSetAddrAndPwd(pECB);
            break;
        default:
            break;
    }
}

//======================================================
//** 函数名称: HandleAll
//** 功能描述: 秒开关处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void HandleAll(ECB_DATA *pECB)
{
    switch(pECB->cmd)
    {
        case ECB_CTRL_ALL:
            HandleCtrlAll(pECB);
            break;
        default:
            break;
    }
}

//======================================================
//** 函数名称: SmartDataProc
//** 功能描述: 家居数据处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void SmartDataProc(ECB_DATA *pECB)
{
    switch(pECB->type)
    {
        case ECB_TYPE_ATTR:         //配置命令->液晶面板
            HandleAttribute(pECB);
            break;
        case ECB_TYPE_CTRL:         //控制命令->液晶面板
            HandleContrl(pECB);
            break;
        case ECB_TYPE_SYNC:         //同步命令->液晶面板
            HandleSync(pECB);
            break;
        case ECB_TYPE_SET:		   //设置命令->液晶面板
            HandleSet(pECB);
            break;
        case ECB_TYPE_ALL:		   //全部命令->液晶面板
            HandleAll(pECB);
            break;
        default:
            DBGMSG(DPINFO, "SmartDataProc recv error type:%x\r\n", pECB->type);
            break;
    }
}

//======================================================
//** 函数名称: SmartDataPwdProc
//** 功能描述: 家居数据密码处理
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static DWORD SmartDataPwdProc(ECB_DATA *pECB)
{
    if (pECB->type == ECB_TYPE_SET && pECB->cmd == ECB_SET_ADDR_PWD)
        return 0;
    if (pECB->type == ECB_TYPE_ATTR && pECB->cmd == ECB_REPORT_INFO_ACK)
        return 0;

    if (pECB->pwd != g_SmartDevPwd)
        return 1;

    return 0;
}

//======================================================
//** 函数名称: SmartRecvProc
//** 功能描述: 家居数据接收处理
//** 输　入: buf len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void SmartRecvProc(char *buf, int len)        //获得协议帧。
{
    int ptr = 0;
    ECB_DATA *pECB = NULL;

    // ZIGBEE模块初始化消息ABBCCD开头
    if (0xAB == buf[0] && 0xBC == buf[1] && 0xCD == buf[2])
    {
        SmartZigbeeProc(buf, len);
        return;
    }

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
            DBGMSG(DPINFO, "SmartRecvProc error length:%d, recv len:%d\r\n", pECB->length, len - ptr);
            ptr++;
            continue;
        }

        if(pECB->data[pECB->length - ECB_DATA_HEAD] != GetCheckNum(&pECB->length, pECB->length))
        {
            DBGMSG(DPINFO, "SmartRecvProc CheckNum fail\r\n");
            ptr++;
            continue;
        }

        if (SmartDataPwdProc(pECB))					 // 比较设备密码
        {
            ptr += pECB->length + ECB_HEAD_LEN_LENGTH;
            continue;
        }

        SmartPrintf((char *)pECB, pECB->length + 3);// 打印出读取的数据

        SmartComRecv(pECB);							// 接收数据应答处理
        SmartDataProc(pECB);                   		// 获得协议帧后，进行数据的处理
        ptr += pECB->length + ECB_HEAD_LEN_LENGTH;
    }
}

//======================================================
//** 函数名称: SmartServer
//** 功能描述: 家居协议服务
//** 输　入: pParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static DWORD SmartServer(HANDLE pParam)
{
    int len;
    char buf[1024];

    DBGMSG(DPINFO, "SmartServer start\r\n");

    while(g_bRun)
    {
        len = ReadComm(g_hCom, buf, sizeof(buf) - 1);   //串口读取数据消息
        if (len < 0)
            continue;
        else if (len == 0)
            continue;

        //SmartPrintf(buf, len);                 	//打印出读取的数据
        SmartRecvProc(buf, len);               			//解析协议帧及长度
    }

    DBGMSG(DPINFO, "SmartServer Stop\r\n");
    return 0;
}

//======================================================
//** 函数名称: SmartServerPC
//** 功能描述: 家居PC调试服务
//** 输　入: pParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static DWORD SmartServerPC(HANDLE pParam)
{
    int len;
    char buf[1024];

    InitNetwork();
    SOCKET sockfd = UdpCreate(15680);

    while (g_bRun)
    {
        len = UdpRecv(sockfd, buf, 1024, 1000);
        if (len > 0)
        {
            SmartRecvProc(buf, len);
        }
    }

    return 0;
}

//======================================================
//** 函数名称: ReadDevInfo
//** 功能描述: 读取设备版本信息文件
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void ReadDevInfo()
{
    char fileName[MAX_PATH];
    sprintf(fileName, "%s/%s", USERDIR, "Version.dat");
    FILE *pFile = fopen(fileName, "rb");
    if (NULL == pFile)
    {
        sprintf(fileName, "%s/%s", FLASHDIR, "Version.dat");
        pFile = fopen(fileName, "rb");
    }

    if (pFile)
    {
        fread(&g_Version, 1, 2, pFile);
        fseek(pFile, 0, SEEK_SET);
        fread(&g_SoftVersion, 1, 4, pFile);
        g_SoftVersion = htonl(g_SoftVersion);
        g_SmartDevType = htons(ECB_DEV_TYPE);
        fclose(pFile);
    }
    else
    {
        g_Version = 0x0020;
        g_SmartDevType = htons(ECB_DEV_TYPE);
        g_SoftVersion = 0x6000000;
        DBGMSG(DPERROR, "SmartServer open fail\r\n");
    }
}

//======================================================
//** 函数名称: SmartSyncTime
//** 功能描述: 家居设备同步时间
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SmartSyncTime()
{
    char buf[64];
    ECB_DATA *pData = (ECB_DATA *)buf;
    pData->head = ECB_HEAD;
    pData->length = 18;
    pData->src = g_SmartDevAddr;
    pData->dst = INVALID_PHY_ADDR;
    pData->type = ECB_TYPE_SYNC;
    pData->cmd = ECB_SYNC_TIME;
    pData->pwd = g_SmartDevPwd;
    pData->data[0] = 0x01;		// 编号
    memcpy(&pData->data[1], &g_SmartDevID, 4);
    memcpy(&pData->data[5], &g_SmartDevType, 2);
    pData->data[7] = GetCheckNum(&pData->length, pData->length);		// 校验和
    SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: SmartGetStatus
//** 功能描述: 家居设备获取状态
//** 输　入: device
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SmartGetStatus(DEVICE *device)
{
    char buf[64];
    ECB_DATA *pData = (ECB_DATA *)buf;
    pData->head = ECB_HEAD;
    pData->length = 18;
    pData->src = g_SmartDevAddr;
    pData->dst = INVALID_PHY_ADDR;
    pData->type = ECB_TYPE_SYNC;
    pData->cmd = ECB_GET_STATUS;
    pData->pwd = g_SmartDevPwd;
    memcpy(&pData->data[0], device, 7);
    pData->data[7] = GetCheckNum(&pData->length, pData->length);		// 校验和
    SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: SendSmartCmd
//** 功能描述: 发送家居控制命令
//** 输　入: device cmd param
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SendSmartCmd(DEVICE *device, BYTE cmd, WORD param)
{
    param = htons(param);

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;                                            // 发送数据头
    pReply->length = 22;                                                // 发送数据长度
    pReply->src = g_SmartDevAddr;                                       // 发送的源地址
    pReply->dst = INVALID_PHY_ADDR;                                     // 发送的目标地址
    pReply->type = ECB_TYPE_CTRL;                                       // 设备控制指令
    pReply->cmd = ECB_CTRL_DEV;
    pReply->pwd = g_SmartDevPwd;										// 系统密码
    memcpy(&pReply->data[0], device, 7);								// 设备标识
    pReply->data[7] = cmd;                                              // 控制方式
    memcpy(&pReply->data[8], &param, 2);								// 控制参数
    pReply->data[10] = GetSeqNum();										// 控制序号
    pReply->data[11] = GetCheckNum(&pReply->length, pReply->length);	// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: SendSceneCmd
//** 功能描述: 发送情景控制命令
//** 输　入: device cmd param
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SendSceneCmd(DEVICE *device, BYTE cmd, WORD param)
{
    param = htons(param);

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;                                            // 发送数据头
    pReply->length = 16;                                                // 发送数据长度
    pReply->src = g_SmartDevAddr;                                       // 发送的源地址
    pReply->dst = INVALID_PHY_ADDR;                                     // 发送的目标地址
    pReply->type = ECB_TYPE_ALL;                                        // 设备控制指令
    pReply->cmd = ECB_CTRL_ALL;
    pReply->pwd = g_SmartDevPwd;										// 系统密码
    //memcpy(&pReply->data[0], device, 7);								// 设备标识
    pReply->data[0] = 0x01;
    pReply->data[1] = cmd;                                              // 控制方式
    memcpy(&pReply->data[2], &param, 2);								// 控制参数
    pReply->data[4] = GetSeqNum();										// 控制序号
    pReply->data[5] = GetCheckNum(&pReply->length, pReply->length);		// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: SendStatusCmd
//** 功能描述: 发送家居状态命令
//** 输　入: device cmd param
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SendStatusCmd(DEVICE *device, BYTE cmd, WORD param)
{
    param = htons(param);

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;                                            // 发送数据头
    pReply->length = 23;                                                // 发送数据长度
    pReply->src = g_SmartDevAddr;                                       // 发送的源地址
    pReply->dst = INVALID_PHY_ADDR;                                     // 发送的目标地址
    pReply->type = ECB_TYPE_SYNC;                                       // 设备控制指令
    pReply->cmd = ECB_SYNC_STATUS;
    pReply->pwd = g_SmartDevPwd;										// 系统密码
    memcpy(&pReply->data[0], device, 7);								// 设备标识
    memcpy(&pReply->data[7], &g_Version, 2);							// 设备版本
    pReply->data[9] = cmd;                                              // 控制方式
    memcpy(&pReply->data[10], &param, 2);								// 控制参数
    pReply->data[12] = GetCheckNum(&pReply->length, pReply->length);	// 校验和
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** 函数名称: SendZigbeeCmd
//** 功能描述: 发送ZIGBEE控制命令
//** 输　入: data len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SendZigbeeCmd(char *data, const int len)
{
    SendCommand(g_hCom, data, len);
}

//======================================================
//** 函数名称: StartSmartServer
//** 功能描述: 启动智能家居服务
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void StartSmartServer()
{
    if (g_bRun)
        return;

    g_hCom = OpenComm(SMART_COM, 115200, 8, 1000);
    if (g_hCom == NULL)
    {
        DBGMSG(DPINFO, "SmartServer OpenComm fail\r\n");
        return;
    }

    // 读取版本号
    ReadDevInfo();
    g_bRun = TRUE;
    g_SmartDevID = ReadCpuID();
    g_SmartDevAddr = GetSmartAddr();
    g_SmartDevPwd = GetSmartPwd();
    g_SmartEnvId = GetSmartEnvId();
    g_timeSync = FALSE;
    g_EnvTemp = 24;
    g_SmartConfig = FALSE;
    g_cfgTime = 0;
	g_DevSeq = 0;
	g_TimerSeq = 0;
    g_hSmartServer = DPThreadCreate(0x4000, SmartServer, NULL, TRUE, 5);   // 智能家居服务协议线程

#ifdef DPCE
    g_SmartDevID = htonl(0xafbd52b6);
    g_SmartDevID = 0x4c91830a;
    DPThreadCreate(0x4000, SmartServerPC, NULL, TRUE, 5);
#endif
    DPCreateSmartCom(); 		// 串口消息线程
    DPCreateSmartSend();		// 串口发送线程
}


//======================================================
//** 函数名称: StopSmartServer
//** 功能描述: 停止智能家居服务
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
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
