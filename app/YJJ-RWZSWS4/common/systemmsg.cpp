#include "roomlib.h"

#define	MASSAGE_MAX		512
#define	MASSAGE_MASK	(MASSAGE_MAX - 1)
typedef struct
{
    SYS_MSG m_MsgBuf[MASSAGE_MAX];
    DWORD m_ReadPtr;
    DWORD m_WritePtr;
    DWORD m_MsgCount;
} Msg_Channal;

static Msg_Channal m_MsgChannal[MSG_MAX_TYPE];
static HANDLE m_hMsgSemp = NULL;
static StaticLock m_SysMsgCs;

//======================================================
//** ��������: InitSysMessage
//** ��������: ��ʼ����Ϣ����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void InitSysMessage(void)
{
    m_hMsgSemp = DPCreateSemaphore(0, (MASSAGE_MAX * MSG_MAX_TYPE));
    memset(m_MsgChannal, 0, sizeof(Msg_Channal) * MSG_MAX_TYPE);
    DPCreateKeyEvent();       	// ���̰����߳�
    DPCreateTouchEvent();       // ���������߳�
    DPCreateTimerEvent();       // ��ʱ�¼��߳�
}

//======================================================
//** ��������: DPPostMessage
//** ��������: ������Ϣ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
BOOL DPPostMessage(DWORD msgtype, DWORD wparam, DWORD lparam, DWORD zParam, DWORD type)
{
    Msg_Channal *pChanal;
    BOOL ret = FALSE;
    m_SysMsgCs.lockon();
    if(type < MSG_MAX_TYPE)
    {
        pChanal = &m_MsgChannal[type];
        if(pChanal->m_MsgCount < MASSAGE_MAX)
        {
            pChanal->m_MsgBuf[pChanal->m_WritePtr].msg = msgtype;       // ��Ϣ����
            pChanal->m_MsgBuf[pChanal->m_WritePtr].wParam = wparam;     // ����ID��(x����)
            pChanal->m_MsgBuf[pChanal->m_WritePtr].lParam = lparam;     // (y����)
            pChanal->m_MsgBuf[pChanal->m_WritePtr].zParam = zParam;     // Touch����
            pChanal->m_WritePtr++;
            pChanal->m_WritePtr &= MASSAGE_MASK;
            pChanal->m_MsgCount++;
            ret = TRUE;
        }
    }

    DPSetSemaphore(m_hMsgSemp);
    m_SysMsgCs.lockoff();
    return ret;
}

//======================================================
//** ��������: DPGetMessage
//** ��������: ��ȡ��Ϣ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
DWORD DPGetMessage(SYS_MSG *msg)
{
    DWORD i;
    Msg_Channal *pChanal;
    DWORD ret = MSG_MAX_TYPE;
again:
    if(DPGetSemaphore(m_hMsgSemp, 1000))
    {
        m_SysMsgCs.lockon();
        for(i = 0; i < MSG_MAX_TYPE; i++)
        {
            pChanal = &m_MsgChannal[i];
            if(pChanal->m_MsgCount > 0)
            {
                memcpy(msg, &pChanal->m_MsgBuf[pChanal->m_ReadPtr], sizeof(SYS_MSG));
                pChanal->m_MsgCount--;
                pChanal->m_ReadPtr++;
                pChanal->m_ReadPtr &= MASSAGE_MASK;
                ret = i;
                break;
            }
        }
        m_SysMsgCs.lockoff();
    }
    if(ret == MSG_MAX_TYPE)
        goto again;
    return ret;                       //������Ϣtype
}

//======================================================
//** ��������: CleanUserInput
//** ��������: �����Ϣ����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void CleanUserInput(void)
{
    m_SysMsgCs.lockon();
    memset(&m_MsgChannal[MSG_TOUCH_TYPE], 0, sizeof(Msg_Channal));
    memset(&m_MsgChannal[MSG_KEY_TYPE], 0, sizeof(Msg_Channal));
    m_SysMsgCs.lockoff();
}
