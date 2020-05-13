#include "CCtrlModules.h"
#include <string.h>
char Edit_Time[] = "00:00";
char Edit_Data[128];
char Edit_Device[35];
char Edit_Action[10];
extern BOOL E_Tim;

extern pNode p_Edit;       //定时事件编辑时参数的结构体指针变量
class CTimerEditApp : public CAppBase
{
public:
	CTimerEditApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTimerEditApp()
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:
			if(m_dwTimeout < 30)
			{
				m_dwTimeout++;
				if(m_dwTimeout == 30)
				{
					m_dwTimeout = 0;
					DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);			
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}			
			break;
		case TOUCH_SLIDE:
			m_dwTimeout = 0;
			break;				
		case TOUCH_MESSAGE:
			m_dwTimeout = 0;
			if(wParam == m_idCancel)
			{   //返回按键
				E_Tim = FALSE;
				UpdatSetTimer();          //更新一次
				DPPostMessage(MSG_START_APP, TIMER_SELECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			
          /*********************定时事件时钟编辑************************/
			else if(wParam == m_idTime)   //2018.1.16开始更新过 
			{   //时间编辑处
				DPPostMessage(MSG_START_APP, TIMER_TIME_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
		

		  /*******************定时事件重复时间编辑*********************/
			else if(wParam == m_idDate)
			{   //重复编辑处
				DPPostMessage(MSG_START_APP, TIMER_WEEK_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
				   

		   break;
		}
		return TRUE;	
	}

	void OnCreate()
	{
		char buf[128];
		int i;
 	/**************传参赋值******************/
		strcpy(Edit_Time,p_Edit->Time);
		strcpy(Edit_Data,p_Edit->p_Data);
		strcpy(Edit_Device,p_Edit->Device);
		strcpy(Edit_Action,p_Edit->CTL_Type);
		
	/**************显示定时时间**************/
		sprintf(buf, "%s", Edit_Time);
		m_pTime->SetSrc(buf);
		m_pTime->Show(TRUE);

	/**************显示重复时间**************/
		memset(buf,0,sizeof(buf));
		sprintf(buf, "%s", Edit_Data);
		m_pDate->SetSrc(buf);
		m_pDate->Show(TRUE);

	/*************显示控制设备***************/
		memset(buf,0,sizeof(buf));
		sprintf(buf, "%s", Edit_Device);
		m_pObject->SetSrc(buf);
		m_pObject->Show(TRUE);

	/**************显示控制方式**************/
		memset(buf,0,sizeof(buf));
		sprintf(buf, "%s", Edit_Action);
		m_pAction->SetSrc(buf);
		m_pAction->Show(TRUE);
		
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("timer_edit.xml");
	
		GetCtrlByName("back", &m_idCancel);
		GetCtrlByName("btn_time", &m_idTime);
		GetCtrlByName("btn_date", &m_idDate);

		m_pTime = (CDPStatic *)GetCtrlByName("time");
		m_pDate = (CDPStatic *)GetCtrlByName("date");
		m_pObject = (CDPStatic *)GetCtrlByName("object");
		m_pAction = (CDPStatic *)GetCtrlByName("action");

		OnCreate();
		return TRUE;
	}

private:
	DWORD m_idTime;
	DWORD m_idDate;
	DWORD m_idCancel;
	
	CDPStatic* m_pTime;
	CDPStatic* m_pDate;
	CDPStatic* m_pObject;
	CDPStatic* m_pAction;

	
};

CAppBase* CreateTimerEditApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTimerEditApp* pApp = new CTimerEditApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}
