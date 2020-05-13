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
//** 函数名称: InitSysMessage
//** 功能描述: 初始化系统消息
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void InitSysMessage(void)
{
	m_hMsgSemp = DPCreateSemaphore(0, (MASSAGE_MAX*MSG_MAX_TYPE));
	memset(m_MsgChannal, 0, sizeof(Msg_Channal) * MSG_MAX_TYPE);
	DPCreateKeyEvent();       // 键盘按键线程(这个线程貌似在项目中没有用到)    
	DPCreateTouchEvent();     // 触摸按键进程
	DPCreateTimeEvent();      // 这里是创建时间线程
	DPCreateTimer();          // 这里是定时事件的线程
}

//======================================================
//** 函数名称: DPPostMessage
//** 功能描述: 发送消息
//** 输　入: msgtype wparam lparam zParam type
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
BOOL DPPostMessage(DWORD msgtype, DWORD wparam, DWORD lparam, DWORD zParam, DWORD type)
{
	Msg_Channal* pChanal;
	BOOL ret = FALSE;
	m_SysMsgCs.lockon();
	if(type < MSG_MAX_TYPE)
	{
		pChanal = &m_MsgChannal[type];
		if(pChanal->m_MsgCount < MASSAGE_MAX)
		{
			pChanal->m_MsgBuf[pChanal->m_WritePtr].msg = msgtype;       // 消息类型
			pChanal->m_MsgBuf[pChanal->m_WritePtr].wParam = wparam;     // 窗口ID号(x坐标)
			pChanal->m_MsgBuf[pChanal->m_WritePtr].lParam = lparam;     // (y坐标)
			pChanal->m_MsgBuf[pChanal->m_WritePtr].zParam = zParam;     // Touch类型
			pChanal->m_WritePtr++;
			pChanal->m_WritePtr &= MASSAGE_MASK;
			pChanal->m_MsgCount++;
			ret = TRUE;
		}
	}

	DPSetSemaphore(m_hMsgSemp);  // 激活信号(此时，在所有线程中只有DPPOST,GETPOST在执行)                  
	m_SysMsgCs.lockoff();
	return ret;
}

//======================================================
//** 函数名称: DPGetMessage
//** 功能描述: 接收消息
//** 输　入: msg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
DWORD DPGetMessage(SYS_MSG* msg)
{
	DWORD i;
	Msg_Channal* pChanal;
	DWORD ret = MSG_MAX_TYPE;
again:
	if(DPGetSemaphore(m_hMsgSemp, 1000))     //这里加了个这个函数是什么原因?
	{
		m_SysMsgCs.lockon();
		for(i = 0; i < MSG_MAX_TYPE; i++)
		{
			pChanal = &m_MsgChannal[i];      //从类型0开始检测
			if(pChanal->m_MsgCount > 0)      //如果这个类型的消息里确实有消息内容，那么跳出。
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
	return ret;                       //这里返回的是消息的类型type;
}

//======================================================
//** 函数名称: CleanUserInput
//** 功能描述: 清空消息队列
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void CleanUserInput(void)
{
	m_SysMsgCs.lockon();
	memset(&m_MsgChannal[MSG_TOUCH_TYPE], 0, sizeof(Msg_Channal));
	memset(&m_MsgChannal[MSG_KEY_TYPE], 0, sizeof(Msg_Channal));
	m_SysMsgCs.lockoff();
}

