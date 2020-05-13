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
//** ��������: SmartConfig
//** ��������: ����״̬����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SmartPrintf
//** ��������: ���ݴ�ӡ
//** �䡡��: pdata len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void SmartPrintf(char *pdata, DWORD len)
{
    // ��ӡ
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
//** ��������: SendCommand
//** ��������: ���ʹ�������
//** �䡡��: hCom buf len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void SendCommand(HANDLE hCom, char *buf, int len)
{
    if (SmartComSend(buf, len))
        return;

    SmartPrintf(buf, len);                    // ��ӡ���͵�����

    g_CS.lockon();
    SendComm(hCom, buf, len);                 // �򴮿���д����
    g_CS.lockoff();
}

//======================================================
//** ��������: GetCheckNum
//** ��������: У����
//** �䡡��: buf len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static BYTE GetCheckNum(BYTE *buf, int len)
{
    BYTE check_num = buf[0];
    for(int i = 1; i < len; i++)
        check_num ^= buf[i];
    return check_num;
}

//======================================================
//** ��������: GetSeqNum
//** ��������: ���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static BYTE GetSeqNum()
{
    static BYTE g_seqnum;
    if (0 == g_seqnum++)
        g_seqnum = 1;
    return g_seqnum;
}

//======================================================
//** ��������: SmartSendCmd
//** ��������: ��������
//** �䡡��: buf len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SmartSendCmd(char *buf, int len)
{
    SmartPrintf(buf, len);                    // ��ӡ���͵�����

    g_CS.lockon();
    SendComm(g_hCom, buf, len);                 // �򴮿���д����
    g_CS.lockoff();
}

//======================================================
//** ��������: SmartTimeSync
//** ��������: ʱ��ͬ��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
    pReply->data[0] = 0x01;		// ͨ·���
    memcpy(&pReply->data[1], &g_SmartDevID, 4);
    memcpy(&pReply->data[5], &g_SmartDevType, 2);
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: SmartReportID
//** ��������: �ϱ�ID
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SmartReportID()                                              // �ϱ��豸ID
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
    pData->data[8] = GetCheckNum(&pData->length, pData->length);		// У���
    SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: HandleInit
//** ��������: �ָ���������
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
    pReply->data[10] = GetSeqNum();		// ���
    pReply->data[11] = GetCheckNum(&pReply->length, pReply->length); 	// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    DWORD count;
    SmartDev *pSmartDev = GetSmartDev(&count);

    if (pECB->data[7] == SCMD_OPEN)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_OPEN;
        SetLightGpioVal(pECB->data[0], FALSE);	// ��
    }
    else if (pECB->data[7] == SCMD_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// ��
    }
    else if (pECB->data[7] == SCMD_DELAY_OPEN)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_OPEN;
        SetLightGpioVal(pECB->data[0], FALSE);	// ��
    }
    else if (pECB->data[7] == SCMD_DELAY_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// ��
    }
    else if (pECB->data[7] == SCMD_DELAY_OPEN_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// ��
    }
    else if (pECB->data[7] == SCMD_OPEN_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd =
            (pSmartDev[pECB->data[0] - 1].cmd == SCMD_OPEN) ? SCMD_CLOSE : SCMD_OPEN;
        if (pSmartDev[pECB->data[0] - 1].cmd == SCMD_OPEN)
            SetLightGpioVal(pECB->data[0], FALSE);	// ��
        else
            SetLightGpioVal(pECB->data[0], TRUE);	// ��
    }
    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)&pSmartDev[pECB->data[0] - 1], 0);
}

//======================================================
//** ��������: HandleInit
//** ��������: �ָ���������
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    DPPostMessage(MSG_SYSTEM, RESET_MACH, 0, 0);
}

//======================================================
//** ��������: HandleReset
//** ��������: �����豸
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
    pReply->data[0] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
}

//======================================================
//** ��������: HandleSearchDev
//** ��������: �����豸Ӧ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
    pReply->data[0] = pECB->data[0];				// ���
    memcpy(&pReply->data[1], &g_SmartDevID, 4);		// ����ID
    memcpy(&pReply->data[5], &g_SmartDevType, 2);	// ��������
    memcpy(&pReply->data[7], &g_Version, 2);		// �����汾
    pReply->data[9] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: HandleReportAck
//** ��������: �ϱ�IDӦ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void HandleReportAck(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[0], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[4], 2))
        return;

    // ֪ͨ�ϲ�Ӧ�ã��ϱ��ɹ�
    //DPPostMessage(MSG_PRIVATE, PRJ_REPORT_APPID, 0, 0);
}

//======================================================
//** ��������: HandleSetAddrAndPwd
//** ��������: ���õ�ַ������
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void HandleSetAddrAndPwd(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    // ���ö๦�ܴ�����
    if (pECB->length == 27)
    {
        memcpy(&g_SmartEnvId, &pECB->data[11], 4);
        SetSmartEnvId(g_SmartEnvId);
    }
    else
    {
        // ���������ַ
        g_SmartDevAddr = pECB->dst;
        SetSmartAddr(g_SmartDevAddr);

        // ����ϵͳ����
        g_SmartDevPwd = pECB->pwd;
        SetSmartPwd(g_SmartDevPwd);

        // ����·��ģʽ����
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
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    if (pECB->length != 27)
        DPPostMessage(MSG_SYSTEM, RESET_ZIGBEE, 0, 0);
}

//======================================================
//** ��������: HandleSetDevice
//** ��������: �����豸����
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void HandleSetDevice(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;
	
    // ����״̬
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
    //pReply->data[7] = GetSeqNum();		// ���
    pReply->data[8] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    // �豸�б�
	AddSmartDev(pECB);		
	DPPostMessage(MSG_PRIVATE, MAIN_APPID, MSG_SMART_UPDATE, 0);
  
}

//======================================================
//** ��������: HandleSetScene
//** ��������: �����龰����
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void HandleSetScene(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    // ����״̬
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
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    // ���������б�
    AddSceneCtrlList(pECB);
}

//======================================================
//** ��������: HandleSetTime
//** ��������: ���ö�ʱ����
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void HandleSetTimer(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;

    // ����״̬
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
    //pReply->data[7] = GetSeqNum();		// ���
    pReply->data[8] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    // ��Ӷ�ʱ�¼�	
	AddSmartTimer(pECB);	
}

//======================================================
//** ��������: HandleSetTime
//** ��������: ����ʱ�䴦��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void HandleSetTime(ECB_DATA *pECB)
{
    if(memcmp(&g_SmartDevID, &pECB->data[1], 4))
        return;
    if(memcmp(&g_SmartDevType, &pECB->data[5], 2))
        return;
	
    // ����״̬
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
    pReply->data[7] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    // ��������/����/ʱ��
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
//** ��������: HandleCtrlAll
//** ��������: �뿪�ش���
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
    pReply->data[4] = GetSeqNum();		// ���
    pReply->data[5] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
#endif
	BYTE scene = (pECB->data[1] == SCMD_OPEN) ? 1 : 2;
	SetStatusByScene(scene);
    SetStatusByAll(pECB->data[1]);
}

//======================================================
//** ��������: HandleControl
//** ��������: �豸����
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
        pReply->data[10] = GetSeqNum(); 	// ���
        pReply->data[11] = GetCheckNum(&pReply->length, pReply->length);	// У���
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
//** ��������: HandleControlAck
//** ��������: �豸����Ӧ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: HandleSetTimeSync
//** ��������: �豸����ʱ��ͬ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: HandleStatusSync
//** ��������: �豸״̬ͬ������
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: HandleStatusSensor
//** ��������: ������״̬����
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: HandleGetStatus_Air
//** ��������: ��ȡ�յ�״̬����
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void HandleGetStatus_Air(ECB_DATA *pECB)
{
    WORD status = 0;
    BYTE MIDparam;

    ECB_CTRL_ACK_AIR *pAck = (ECB_CTRL_ACK_AIR *)pECB->data;

    // 1.1 ȡ����״̬:
    MIDparam = (pAck->param1 & 0xf0);
    if (MIDparam == 0x10)
        status |= 0x4000;
    else if (MIDparam == 0x20)
        status |= 0x8000;

    // 1.2 ȡģʽ״̬:
    MIDparam = (pAck->param1 & 0x0f);
    status  |= (MIDparam << 10);

    // 2 ɨ��λĬ��Ϊȱʡģʽ���ɣ�����жϷ���
    MIDparam = (pAck->param2 & 0x0f);
    status  |= (MIDparam << 5);

    // 3 ���ڵ�ǰ�¶�(�������Ҫ��һ��������������)
    if (pAck->param3 > 0)
    	g_EnvTemp = (pAck->param3 - 128);

    // 4 �����¶�
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
//** ��������: HandleGetStatus
//** ��������: ��ȡ״̬����
//** �䡡  ��: pECB
//** �䡡  ��: ��
//**
//** ����  ��: HJ
//** �ա�  ��: 2018��11��19��
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
//** ��������: HandleCfgCtrl
//** ��������: ���ÿ��ƴ���
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
    pReply->data[10] = GetSeqNum(); 	// ���
    pReply->data[11] = GetCheckNum(&pReply->length, pReply->length);	// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

    DWORD count;
    SmartDev *pSmartDev = GetSmartDev(&count);

    if (pECB->data[7] == SCMD_OPEN)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_OPEN;
        SetLightGpioVal(pECB->data[0], FALSE);	// ��
    }
    else if (pECB->data[7] == SCMD_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// ��
    }
    else if (pECB->data[7] == SCMD_DELAY_OPEN)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_OPEN;
        SetLightGpioVal(pECB->data[0], FALSE);	// ��
    }
    else if (pECB->data[7] == SCMD_DELAY_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// ��
    }
    else if (pECB->data[7] == SCMD_DELAY_OPEN_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd = SCMD_CLOSE;
        SetLightGpioVal(pECB->data[0], TRUE);	// ��
    }
    else if (pECB->data[7] == SCMD_OPEN_CLOSE)
    {
        pSmartDev[pECB->data[0] - 1].cmd =
            (pSmartDev[pECB->data[0] - 1].cmd == SCMD_OPEN) ? SCMD_CLOSE : SCMD_OPEN;
        if (pSmartDev[pECB->data[0] - 1].cmd == SCMD_OPEN)
            SetLightGpioVal(pECB->data[0], FALSE);	// ��
        else
            SetLightGpioVal(pECB->data[0], TRUE);	// ��
    }
    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)&pSmartDev[pECB->data[0] - 1], 0);
}

//======================================================
//** ��������: HandleAttribute
//** ��������: ��������
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void HandleAttribute(ECB_DATA *pECB)
{
    switch(pECB->cmd)
    {
        case ECB_REPORT_INFO_ACK:              //�ϱ�ID��Ļظ����ǳɹ�����ʧ�ܡ�
            HandleReportAck(pECB);
            break;
        case ECB_INIT_ALL:					 //��ʼ��
            HandleInit(pECB);
            break;
        case ECB_RESET_ALL:				     //��λ
            HandleReset(pECB);
            break;
        case ECB_SEARCH_DEV:                   //��������
            HandleSearchDev(pECB);
            break;
        case ECB_SET_DEV_CFG:                  //��������֮������Ϣ
            HandleSetDevice(pECB);
            break;
        case ECB_SET_SCENE:                    //���ó���ģʽ����
            HandleSetScene(pECB);
            break;
        case ECB_SET_TIMER:                    //���ö�ʱ����
            HandleSetTimer(pECB);
            break;
        case ECB_SET_TIME:                     //��������ʱ��
            HandleSetTime(pECB);
            break;
        default:
            break;
    }
}

//======================================================
//** ��������: HandleContrl
//** ��������: ���ÿ���
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: HandleSync
//** ��������: ����ͬ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: HandleSet
//** ��������: ��������
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: HandleAll
//** ��������: �뿪�ش���
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SmartDataProc
//** ��������: �Ҿ����ݴ���
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void SmartDataProc(ECB_DATA *pECB)
{
    switch(pECB->type)
    {
        case ECB_TYPE_ATTR:         //��������->Һ�����
            HandleAttribute(pECB);
            break;
        case ECB_TYPE_CTRL:         //��������->Һ�����
            HandleContrl(pECB);
            break;
        case ECB_TYPE_SYNC:         //ͬ������->Һ�����
            HandleSync(pECB);
            break;
        case ECB_TYPE_SET:		   //��������->Һ�����
            HandleSet(pECB);
            break;
        case ECB_TYPE_ALL:		   //ȫ������->Һ�����
            HandleAll(pECB);
            break;
        default:
            DBGMSG(DPINFO, "SmartDataProc recv error type:%x\r\n", pECB->type);
            break;
    }
}

//======================================================
//** ��������: SmartDataPwdProc
//** ��������: �Ҿ��������봦��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SmartRecvProc
//** ��������: �Ҿ����ݽ��մ���
//** �䡡��: buf len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void SmartRecvProc(char *buf, int len)        //���Э��֡��
{
    int ptr = 0;
    ECB_DATA *pECB = NULL;

    // ZIGBEEģ���ʼ����ϢABBCCD��ͷ
    if (0xAB == buf[0] && 0xBC == buf[1] && 0xCD == buf[2])
    {
        SmartZigbeeProc(buf, len);
        return;
    }

    // Ѱ�� 0x5a 0xa5
    while(ptr + ECB_DATA_MIN_LEN <= len)
    {
        pECB = (ECB_DATA *)(buf + ptr);
        if(pECB->head != ECB_HEAD)                    //��ȡͷָ�롣
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

        if (SmartDataPwdProc(pECB))					 // �Ƚ��豸����
        {
            ptr += pECB->length + ECB_HEAD_LEN_LENGTH;
            continue;
        }

        SmartPrintf((char *)pECB, pECB->length + 3);// ��ӡ����ȡ������

        SmartComRecv(pECB);							// ��������Ӧ����
        SmartDataProc(pECB);                   		// ���Э��֡�󣬽������ݵĴ���
        ptr += pECB->length + ECB_HEAD_LEN_LENGTH;
    }
}

//======================================================
//** ��������: SmartServer
//** ��������: �Ҿ�Э�����
//** �䡡��: pParam
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static DWORD SmartServer(HANDLE pParam)
{
    int len;
    char buf[1024];

    DBGMSG(DPINFO, "SmartServer start\r\n");

    while(g_bRun)
    {
        len = ReadComm(g_hCom, buf, sizeof(buf) - 1);   //���ڶ�ȡ������Ϣ
        if (len < 0)
            continue;
        else if (len == 0)
            continue;

        //SmartPrintf(buf, len);                 	//��ӡ����ȡ������
        SmartRecvProc(buf, len);               			//����Э��֡������
    }

    DBGMSG(DPINFO, "SmartServer Stop\r\n");
    return 0;
}

//======================================================
//** ��������: SmartServerPC
//** ��������: �Ҿ�PC���Է���
//** �䡡��: pParam
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: ReadDevInfo
//** ��������: ��ȡ�豸�汾��Ϣ�ļ�
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SmartSyncTime
//** ��������: �Ҿ��豸ͬ��ʱ��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
    pData->data[0] = 0x01;		// ���
    memcpy(&pData->data[1], &g_SmartDevID, 4);
    memcpy(&pData->data[5], &g_SmartDevType, 2);
    pData->data[7] = GetCheckNum(&pData->length, pData->length);		// У���
    SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: SmartGetStatus
//** ��������: �Ҿ��豸��ȡ״̬
//** �䡡��: device
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
    pData->data[7] = GetCheckNum(&pData->length, pData->length);		// У���
    SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: SendSmartCmd
//** ��������: ���ͼҾӿ�������
//** �䡡��: device cmd param
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SendSmartCmd(DEVICE *device, BYTE cmd, WORD param)
{
    param = htons(param);

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;                                            // ��������ͷ
    pReply->length = 22;                                                // �������ݳ���
    pReply->src = g_SmartDevAddr;                                       // ���͵�Դ��ַ
    pReply->dst = INVALID_PHY_ADDR;                                     // ���͵�Ŀ���ַ
    pReply->type = ECB_TYPE_CTRL;                                       // �豸����ָ��
    pReply->cmd = ECB_CTRL_DEV;
    pReply->pwd = g_SmartDevPwd;										// ϵͳ����
    memcpy(&pReply->data[0], device, 7);								// �豸��ʶ
    pReply->data[7] = cmd;                                              // ���Ʒ�ʽ
    memcpy(&pReply->data[8], &param, 2);								// ���Ʋ���
    pReply->data[10] = GetSeqNum();										// �������
    pReply->data[11] = GetCheckNum(&pReply->length, pReply->length);	// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: SendSceneCmd
//** ��������: �����龰��������
//** �䡡��: device cmd param
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SendSceneCmd(DEVICE *device, BYTE cmd, WORD param)
{
    param = htons(param);

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;                                            // ��������ͷ
    pReply->length = 16;                                                // �������ݳ���
    pReply->src = g_SmartDevAddr;                                       // ���͵�Դ��ַ
    pReply->dst = INVALID_PHY_ADDR;                                     // ���͵�Ŀ���ַ
    pReply->type = ECB_TYPE_ALL;                                        // �豸����ָ��
    pReply->cmd = ECB_CTRL_ALL;
    pReply->pwd = g_SmartDevPwd;										// ϵͳ����
    //memcpy(&pReply->data[0], device, 7);								// �豸��ʶ
    pReply->data[0] = 0x01;
    pReply->data[1] = cmd;                                              // ���Ʒ�ʽ
    memcpy(&pReply->data[2], &param, 2);								// ���Ʋ���
    pReply->data[4] = GetSeqNum();										// �������
    pReply->data[5] = GetCheckNum(&pReply->length, pReply->length);		// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: SendStatusCmd
//** ��������: ���ͼҾ�״̬����
//** �䡡��: device cmd param
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SendStatusCmd(DEVICE *device, BYTE cmd, WORD param)
{
    param = htons(param);

    char buf[64];
    ECB_DATA *pReply = (ECB_DATA *)buf;
    pReply->head = ECB_HEAD;                                            // ��������ͷ
    pReply->length = 23;                                                // �������ݳ���
    pReply->src = g_SmartDevAddr;                                       // ���͵�Դ��ַ
    pReply->dst = INVALID_PHY_ADDR;                                     // ���͵�Ŀ���ַ
    pReply->type = ECB_TYPE_SYNC;                                       // �豸����ָ��
    pReply->cmd = ECB_SYNC_STATUS;
    pReply->pwd = g_SmartDevPwd;										// ϵͳ����
    memcpy(&pReply->data[0], device, 7);								// �豸��ʶ
    memcpy(&pReply->data[7], &g_Version, 2);							// �豸�汾
    pReply->data[9] = cmd;                                              // ���Ʒ�ʽ
    memcpy(&pReply->data[10], &param, 2);								// ���Ʋ���
    pReply->data[12] = GetCheckNum(&pReply->length, pReply->length);	// У���
    SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: SendZigbeeCmd
//** ��������: ����ZIGBEE��������
//** �䡡��: data len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SendZigbeeCmd(char *data, const int len)
{
    SendCommand(g_hCom, data, len);
}

//======================================================
//** ��������: StartSmartServer
//** ��������: �������ܼҾӷ���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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

    // ��ȡ�汾��
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
    g_hSmartServer = DPThreadCreate(0x4000, SmartServer, NULL, TRUE, 5);   // ���ܼҾӷ���Э���߳�

#ifdef DPCE
    g_SmartDevID = htonl(0xafbd52b6);
    g_SmartDevID = 0x4c91830a;
    DPThreadCreate(0x4000, SmartServerPC, NULL, TRUE, 5);
#endif
    DPCreateSmartCom(); 		// ������Ϣ�߳�
    DPCreateSmartSend();		// ���ڷ����߳�
}


//======================================================
//** ��������: StopSmartServer
//** ��������: ֹͣ���ܼҾӷ���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
