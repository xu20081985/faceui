#include "CCtrlModules.h"

BOOL Select_Open = FALSE;
BOOL Select_Close = FALSE;
char *p_action;               // 开标志位

extern BOOL E_Tim;
extern pNode p_Edit;          //定时事件编辑时参数的结构体指针变量

extern char DW[];
class CTimerActionApp : public CAppBase
{
public:
	CTimerActionApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTimerActionApp()
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
			if(wParam == m_idBack)
			{
				if(!E_Tim) {
					
					DPPostMessage(MSG_START_APP, TIMER_OBJECT_APPID, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}

				else {

					if(NULL !=p_action)                            //控制设备字符串赋值
						strcpy(p_Edit->CTL_Type, p_action);        
					else
		 				strcpy(p_Edit->CTL_Type, DW);

					p_Edit->CTL_OPEN = Select_Open;
					p_Edit->CTL_CLOSE = Select_Close;

				    Select_Open = FALSE;
				    Select_Close = FALSE;
					p_action = NULL;
					
					DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_EDIT, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
            else if(wParam == m_idAction_1) {

				if(Select_Open) {

					Select_Open = FALSE;
					p_action = NULL;
				}
				else {

					Select_Open = TRUE;
					Select_Close = FALSE;
					p_action = GetStringByID(13401);
				}
			}
			else if(wParam == m_idAction_2) {
				
				if(Select_Close) {

					Select_Close = FALSE;
					p_action = NULL;
				}
				else {

					Select_Close = TRUE;
					Select_Open = FALSE;
					p_action = GetStringByID(13402);
				}
			}

			OnPage(0);
			break;
		
		}
		return TRUE;	
	}

	void OnPage(DWORD dwPage)
	{
		if(dwPage == 0)
		{
			m_pAction[0]->SetSrc(GetStringByID(13401));		// 开
			m_pAction[0]->Show(TRUE);
			m_pAction[1]->SetSrc(GetStringByID(13402));		// 关
			m_pAction[1]->Show(TRUE);		
	
			m_pSelect[0]->Show(Select_Open);                // 开选择项
			m_pSelect[1]->Show(Select_Close);               // 关选择项

		}

		m_dwPage = dwPage;
	}

	void OnCreate()
	{
		OnPage(0);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("timer_action.xml");

		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("ctl_action1", &m_idAction_1);
		GetCtrlByName("ctl_action2", &m_idAction_2);
		
		m_pAction[0] = (CDPStatic *)GetCtrlByName("action_1");
		m_pAction[1] = (CDPStatic *)GetCtrlByName("action_2");
		m_pAction[2] = (CDPStatic *)GetCtrlByName("action_3");
		m_pAction[3] = (CDPStatic *)GetCtrlByName("action_4");

		m_pSelect[0] = (CDPStatic *)GetCtrlByName("select_1");
	    m_pSelect[1] = (CDPStatic *)GetCtrlByName("select_2");

		if(E_Tim) {

			Select_Open = p_Edit->CTL_OPEN;
			Select_Close = p_Edit->CTL_CLOSE;

			if(Select_Open)
				p_action = GetStringByID(13401);
			else if(Select_Close)
				p_action = GetStringByID(13402);
			else if(Select_Open == FALSE && Select_Close == FALSE)
				p_action = NULL;
		}

		OnCreate(); 
		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idAction_1;                    //动作控制变量1
	DWORD m_idAction_2;                    //动作控制变量2
	CDPStatic* m_pAction[4];
	CDPStatic* m_pSelect[2];               //勾选动作

	DWORD m_dwPage;
};

CAppBase* CreateTimerActionApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTimerActionApp* pApp = new CTimerActionApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}