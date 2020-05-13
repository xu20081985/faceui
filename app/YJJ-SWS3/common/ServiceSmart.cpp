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


int G_MUSIC_PAUSE_FLAG = 0;		//������ͣ��־

int Ctl_Flag = 0;            // 2018.2.24��ӣ������޸���һ������APP����

int TIME_FLAG = 0;
extern int Heat_Flag;
WORD status1;                // 2018.2.26��ӣ�����ˢ�յ��ȵ�״̬
int  G_Temp = 0;         //���ڴ�Ż�ȡ���¶�

WORD CO2 = 0; 			//���ڴ���·��CO2
WORD PM25= 0; 			//���ڴ���·��PM24

//======================================================
//** ��������: GetVersion
//** ��������: ��ȡ�汾��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
DWORD GetVersion()
{
	return g_Version;
}

//======================================================
//** ��������: GetDevType
//** ��������: ��ȡ�豸����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
DWORD GetDevType()
{
	return g_SmartDevType;
}


//======================================================
//** ��������: GetSoftVer
//** ��������: ��ȡ����汾��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
DWORD GetSoftVer()
{
	return g_SoftVersion;
}

//======================================================
//** ��������: SmartPrintf
//** ��������: ��ӡ������Ϣ
//** �䡡��: pdata len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void SmartPrintf(char* pdata, DWORD len)
{
	// ��ӡ
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
//** ��������: SendCommand
//** ��������: ���ʹ�������
//** �䡡��: hCom buf len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void SendCommand(HANDLE hCom, char* buf, int len)
{
	SmartPrintf(buf, len);                    // ��ӡ���͵�����

	g_CS.lockon();
	SendComm(hCom, buf, len);                 // ���ļ���д����
	g_CS.lockoff();
}

//======================================================
//** ��������: GetCheckNum
//** ��������: ��ȡУ���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static BYTE GetCheckNum(BYTE* buf, int len)   // У��ͺ���
{
	BYTE check_num = buf[0];
	for(int i = 1; i < len; i++)
	{
		check_num ^= buf[i];
	}
	return check_num;
}

//======================================================
//** ��������: SmartReportID
//** ��������: �Ҿ��ϱ�ID
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void SmartReportID()                                                    // �ϱ��豸ID
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
	pData->data[8] = GetCheckNum(&pData->length, pData->length);		// У���
	SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: HandleInit
//** ��������: �����ʼ��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pReply->data[0] = GetCheckNum(&pReply->length, pReply->length);		// У���

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	// ���֮ǰ���豸����
	ResetSmartDev();
	DeleteSmartTimer();   // ɾ����ʱ�¼�
	DPPostMessage(MSG_PRIVATE, MAIN_APPID, MSG_SMART_UPDATE, 0);
}

//======================================================
//** ��������: HandleReset
//** ��������: �����ʼ��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pReply->data[0] = GetCheckNum(&pReply->length, pReply->length);		// У���

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	// ���֮ǰ���豸����
	ResetSmartDev();
	DPPostMessage(MSG_PRIVATE, MAIN_APPID, MSG_SMART_UPDATE, 0);
}

//======================================================
//** ��������: HandleSearchDev
//** ��������: ���������豸
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pReply->data[0] = pECB->data[0];				// ���
	memcpy(&pReply->data[1], &g_SmartDevID, 4);		// ����ID
	memcpy(&pReply->data[5], &g_SmartDevType, 2);	// ��������
	memcpy(&pReply->data[7], &g_Version, 2);		// �����汾
	pReply->data[9] = GetCheckNum(&pReply->length, pReply->length);		// У���

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: HandleReportAck
//** ��������: �����ϱ�Ӧ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void HandleReportAck(ECB_DATA* pECB)
{
	// �ϱ�ID�ظ�
	if(0 != memcmp(&g_SmartDevID, &pECB->data[0], 4))
		return;

	g_SmartDevAddr = pECB->dst;
	SetSmartAddr(g_SmartDevAddr);                    //�ϱ��ɹ��������һ�ε�ַ��

	// ֪ͨ�ϲ�Ӧ�ã��ϱ��ɹ�
	DPPostMessage(MSG_PRIVATE, PRJ_REPORT_APPID, 0, 0);
}

//======================================================
//** ��������: HandleSetAddr
//** ��������: �������������ַ
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pReply->data[0] = GetCheckNum(&pReply->length, pReply->length);		// У���

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: HandleSetGroupAddr
//** ��������: �����������ַ
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void HandleSetGroupAddr(ECB_DATA* pECB)
{
	// ���ַ����
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
		pReply->data[0] = pECB->data[0];		// ���
		pReply->data[1] = GetCheckNum(&pReply->length, pReply->length);		// У���

		SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
	}
	else
	{
		AddSmartAddrList(pECB);	
	}
}

//======================================================
//** ��������: HandleSetDevConfig
//** ��������: �����豸����
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pReply->data[0] = pECB->data[0];		// ���
	pReply->data[1] = pECB->data[1];		// ���
	pReply->data[2] = GetCheckNum(&pReply->length, pReply->length);		// У���

	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	if(pECB->data[1] != 0)
	{
		AddSmartDev(pECB);
		DPPostMessage(MSG_PRIVATE, MAIN_APPID, MSG_SMART_UPDATE, 0);
	}
}

//======================================================
//** ��������: HandleSetScene
//** ��������: ���������龰
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pReply->data[0] = pECB->data[0];		// ���
	pReply->data[1] = GetCheckNum(&pReply->length, pReply->length);		// У���
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	// ���������б�
	AddSceneCtrlList(pECB);
}

//======================================================
//** ��������: HandleSetTimer
//** ��������: ��������ʱ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pReply->data[0] = pECB->data[0];		// ���
	pReply->data[1] = pECB->data[1];		// ���
	pReply->data[2] = GetCheckNum(&pReply->length, pReply->length);		// У���
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

	if(temp != pECB->data[2]) {

		temp = pECB->data[2];

		if(pECB->data[1] == 1) {
			
			DeleteSmartTimer();
		}

		if(pECB->data[1] != 0)
		{
			AddSmartTimer(pECB);                // ��Ӷ�ʱ�¼�
		}
	}
	
}

//======================================================
//** ��������: Get_Local_Time
//** ��������: ��ȡ����ʱ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pReply->data[0] = 1;		// ���
	pReply->data[1] = GetCheckNum(&pReply->length, pReply->length);		// У���
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);

}

//======================================================
//** ��������: HandleSetTime
//** ��������: ����ʱ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pReply->data[0] = pECB->data[0];		// ���
	pReply->data[1] = GetCheckNum(&pReply->length, pReply->length);		// У���
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
	
	// ����ʱ��
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
//** ��������: HandleSetTime222
//** ��������: ����ʱ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void HandleSetTime222(ECB_DATA* pECB)
{
	if(pECB->dst != g_SmartDevAddr)
		return;

	
	// ����ʱ��
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
//** ��������: HandleControl
//** ��������: �����������
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void HandleControl(ECB_DATA* pECB)
{
	
	if(pECB->data[0] == SCMD_SCENE)
	{
		// �����������龰����, ���ùܻظ���ֱ��ˢ��״̬
		SetStatusByScene(pECB->data[2]);
	}

	if(pECB->data[0] == SCMD_AC ||pECB->data[0] ==SCMD_MUSIC) {

		if(pECB->data[0] == SCMD_AC)         // 2018.2.24���

			Ctl_Flag = AIR_TYPE;

		else if(pECB->data[0] ==SCMD_MUSIC)  // 2018.2.24���

			Ctl_Flag = MUSIC_TYPE;

		WORD status = (pECB->data[1]<<8 | pECB->data[2]);
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC ,pECB->dst ,status); 
	}
	
	// APP��һ���������ͼ������
	else if(pECB->data[0] == 0x01) {         

		Ctl_Flag = ALL_TYPE;
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_S ,pECB->dst ,0x01); 	
	}
	
	// APP��һ���������ͼ������
	else if(pECB->data[0] == 0x03) {         
	
		Ctl_Flag = ALL_TYPE;
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_S ,pECB->dst ,0x03); 
	}
}

//======================================================
//** ��������: HandleControlAck
//** ��������: ���������Ӧ
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void HandleControlAck(ECB_DATA* pECB)
{
	ECB_CTRL_ACK* pAck = (ECB_CTRL_ACK *)pECB->data;
	if(pAck->cmd == SCMD_SCENE)
	{
		// ����ǳ������ƣ���Ҫ���ҳ����б�����ȡ��������
		// 2017.09.13 �Ź�˵���ø��ݻظ��жϣ�ֱ��ˢ��״̬����
		// SetStatusByList(pECB->src, pAck->param >> 8);
	}
	else
	{
		SetStatusByAck(pAck->addr, pAck->cmd, htons(pAck->param));

		DPPostMessage(MSG_BROADCAST, SMART_STATUS_ACK, pAck->addr, 0);	
	}
}

//======================================================
//** ��������: HandleStatusSync
//** ��������: ����״̬ͬ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	
/*   	if(Heat_Flag == 1) {   // 2018.1.10ֻ�е���ů����򿪣����ݲ�ͬ��

		status = (pECB->data[4]<<8 | pECB->data[5]);
		DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, pAck->addr, status);
   	} */	
}

//======================================================
//** ��������: HandleGetStatus
//** ��������: �����ȡ״̬--��ͨ�豸
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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

// 2018.2.26���
//======================================================
//** ��������: HandleGetStatus_Air
//** ��������: �����ȡ״̬--�յ�
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	// 1.1 ȡ����״̬:
	MIDparam = (pAck->param1 & 0xf0 );

	if(MIDparam == 0x10)
		status |= 0x4000;
	
	else if(MIDparam == 0x20)
		status |= 0x8000;

	// 1.2 ȡģʽ״̬:
	MIDparam = (pAck->param1 & 0x0f );
	status  |= (MIDparam<<10);

	// 2 ɨ��λĬ��Ϊȱʡģʽ���ɣ�����жϷ���
	MIDparam = (pAck->param2 & 0x0f );
	status  |= (MIDparam<<5);

	// 3 ���ڵ�ǰ�¶�(�������Ҫ��һ��������������)
	G_Temp = (pAck->param3 - 128);
	//��ȡ�·��CO2
	if(pECB->length >= 24)  //�����ݳ��ȴ���  ��˵�������·磬ֻ�������ж���
	{
		CO2 = (pAck->param7<<8) | (pAck->param8); 
		PM25 =  (pAck->param11<<8) | (pAck->param12); 
	}
		
	// 4 �����¶�
	MIDparam = (pAck->param4 - 128) - 9;

	if(MIDparam < 6)
		MIDparam = 6;
	
	status  |= MIDparam&0x1f;
		
	DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, pAck->addr, status);
} 

//======================================================
//** ��������: HandleAttribute
//** ��������: ��������Э��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void HandleAttribute(ECB_DATA* pECB)
{
	switch(pECB->cmd)
	{
		case ECB_REPORT_INFO_ACK:              //�ϱ�ID��Ļظ����ǳɹ�����ʧ�ܡ�
			HandleReportAck(pECB);
			break;
		case ECB_SEARCH_DEV:                   //��������������������ò����APP�Զ�����ECB�����ϵ��豸��
			HandleSearchDev(pECB);
			break;
		case ECB_SET_ADDR:                     //�������������ַ
			HandleSetAddr(pECB);
			break;
		case ECB_SET_GROUP_ADDR:               //��������֮ͨ·���ַ
			HandleSetGroupAddr(pECB);
			break;
		case ECB_SET_DEV_CFG:                  //��������֮������Ϣ
			HandleSetDevConfig(pECB);
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
			break;

	}
}

//======================================================
//** ��������: HandleContrl
//** ��������: ��������Э��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
//** ��������: HandleSync
//** ��������: ͬ������Э��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
//** ��������: SmartDataProc
//** ��������: �Ҿ�Э�����ݴ���
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void SmartDataProc(ECB_DATA* pECB)
{
	switch(pECB->type)              //�ж������������͡�
	{
		case ECB_TYPE_ATTR:         //���ԡ�
			HandleAttribute(pECB);
			break;
		case ECB_TYPE_CTRL:         //��������ECB����->Һ����塣
			HandleContrl(pECB);
			break;
		case ECB_TYPE_SYNC:         //ͬ������ECB����->Һ����塣  
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
//** ��������: SmartRecvProc
//** ��������: �Ҿӷ�����մ���
//** �䡡��: buf len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void SmartRecvProc(char* buf, int len)        //���Э��֡��
{
	int ptr = 0;
	ECB_DATA* pECB = NULL;

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

		SmartDataProc(pECB);                   //���Э��֡�󣬽������ݵĴ��� 
		ptr += pECB->length + ECB_HEAD_LEN_LENGTH;
	}
}

//======================================================
//** ��������: SmartServer
//** ��������: �Ҿӷ���
//** �䡡��: pParam
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static DWORD SmartServer(HANDLE pParam)
{
	int len;
	char buf[1024];

	DBGMSG(DPINFO, "SmartServer start\r\n");

	if(g_SmartDevAddr != INVALID_ECB_ADDR)
	{
		// ��ͬ��һ��ʱ��
		SmartSyncTime();                       //�������Է��֣�ò�����￪����ʱ���뱱��ʱ�䲻����
	}

	while(g_bRun)
	{
		len = ReadComm(g_hCom, buf, 1023);     //��ECB�����϶�ȡ���ݵĳ��ȡ�
		if(len < 0)
			break;
		else if(len == 0)
			continue;

		SmartPrintf(buf, len);                 //��ӡ����ECB�����϶���������
		SmartRecvProc(buf, len);               //����һЩ�д���ȡ��Э��֡�����ȡ�
	}

	DBGMSG(DPINFO, "SmartServer Stop\r\n");
	return 0;
}

//======================================================
//** ��������: SmartServerPC
//** ��������: �Ҿӵ��Է���-pc
//** �䡡��: pParam
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
//** ��������: ReadDevInfo
//** ��������: ��ȡ�豸��Ϣ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
//** ��������: SmartSyncTime
//** ��������: ͬ��ʱ��
//** �䡡��: addr
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pData->data[0] = 1;		// ���
	pData->data[1] = GetCheckNum(&pData->length, pData->length);		// У���
	SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: SmartGetStatus
//** ��������: ��ȡ״̬--��ͨ�豸
//** �䡡��: addr
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pData->data[0] = 1;		// ���
	memcpy(&pData->data[1], &addr, 2);
	pData->data[3] = GetCheckNum(&pData->length, pData->length);		// У���
	SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

// 2018.2.26��ӣ�����ͬ���յ��Ĳ�ѯ״̬
//======================================================
//** ��������: SmartGetStatus_Air
//** ��������: ��ȡ״̬--�յ�
//** �䡡��: addr
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
	pData->data[0] = 1;		// ���
	memcpy(&pData->data[1], &addr, 2);
	pData->data[3] = GetCheckNum(&pData->length, pData->length);		// У���
	SendCommand(g_hCom, buf, pData->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: SendSmartCmd
//** ��������: ���ͼҾ�����
//** �䡡��: addr cmd param
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void SendSmartCmd(WORD addr, BYTE cmd, WORD param)
{
	param = htons(param);

	char buf[64];
	ECB_DATA* pReply = (ECB_DATA *)buf;
	pReply->head = ECB_HEAD;                                            // ���͵�ͷָ��
	pReply->length = 10;                                                // �������ݳ���
	pReply->src = g_SmartDevAddr;                                       // ���͵�Դ��ַ
	pReply->dst = addr;                                                 // ���͵�Ŀ���ַ
	pReply->type = ECB_TYPE_CTRL;                                       // �豸����
	pReply->cmd = ECB_CTRL_DEV;
	pReply->data[0] = cmd;                                              // ������������
	memcpy(&pReply->data[1], &param, 2);
	pReply->data[3] = GetCheckNum(&pReply->length, pReply->length);		// У���
	SendCommand(g_hCom, buf, pReply->length + ECB_HEAD_LEN_LENGTH);
}

//======================================================
//** ��������: StartSmartServer
//** ��������: �����Ҿӷ���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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

	// ��ȡ�汾��,�豸����
	ReadDevInfo();   

	g_bRun = TRUE;
	g_SmartDevID = ReadCpuID();
	g_SmartDevAddr = GetSmartAddr();                                       
	g_hSmartServer = DPThreadCreate(0x4000, SmartServer, NULL, TRUE, 5);   //���������ܼҾӷ���Э���̡߳�

#ifdef DPCE	
	g_SmartDevID = htonl(0xafbd52b6);
	g_SmartDevID = 0x4c91830a;
	DPThreadCreate(0x4000, SmartServerPC, NULL, TRUE, 5);	
#endif
}

//======================================================
//** ��������: StopSmartServer
//** ��������: ֹͣ�Ҿӷ���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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