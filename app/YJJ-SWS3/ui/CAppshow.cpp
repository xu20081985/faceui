#include "CCtrlModules.h"
#include "SmartConfig.h"

BOOL Flag_Show[4] = {FALSE};                     // 定义勾选项的标志位数组

class CShow : public CAppBase
{
public:
	CShow(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CShow()
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

			if(wParam == m_idBack) {

				SetPrjShow(Flag_Show);
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idselect_1) {

				if(Flag_Show[0] == FALSE) {
				
					Flag_Show[0] = TRUE;

					Flag_Show[1] = FALSE;
					Flag_Show[2] = FALSE;
					Flag_Show[3] = FALSE;
				}
					

				else if(Flag_Show[0] ==TRUE)
					Flag_Show[0] = FALSE;
			}

			else if(wParam == m_idselect_2) {

				if(Flag_Show[1] == FALSE) {

					Flag_Show[1] = TRUE;

					Flag_Show[0] = FALSE;
					Flag_Show[2] = FALSE;
					Flag_Show[3] = FALSE;
				}
					

				else if(Flag_Show[1] == TRUE)
					Flag_Show[1] = FALSE;
			}

			else if(wParam == m_idselect_3) {

				if(Flag_Show[2] == FALSE) {

					Flag_Show[2] = TRUE;

					Flag_Show[0] = FALSE;
					Flag_Show[1] = FALSE;
					Flag_Show[3] = FALSE;

				}

				else if(Flag_Show[2] == TRUE)
					Flag_Show[2] = FALSE;
			}

			else if(wParam == m_idselect_4) {

				if(Flag_Show[3] == FALSE) {

					Flag_Show[3] = TRUE;

					Flag_Show[0] = FALSE;
					Flag_Show[1] = FALSE;
					Flag_Show[2] = FALSE;
				}
					

				else if(Flag_Show[3] == TRUE)
					Flag_Show[3] = FALSE;
			}
			
			OnCreate();
			break;
		}
		return TRUE;	
	}

	void OnCreate()
	{
		m_pWeek[0]->SetSrc(GetStringByID(1042));		// 1分钟
		m_pWeek[0]->Show(TRUE);
		m_pWeek[1]->SetSrc(GetStringByID(1043));		// 5分钟
		m_pWeek[1]->Show(TRUE);
		m_pWeek[2]->SetSrc(GetStringByID(1044));		// 10分钟
		m_pWeek[2]->Show(TRUE);
		m_pWeek[3]->SetSrc(GetStringByID(1045));		// 永不
		m_pWeek[3]->Show(TRUE);
		m_pSelect[0]->Show(Flag_Show[0]);
		m_pSelect[1]->Show(Flag_Show[1]);
		m_pSelect[2]->Show(Flag_Show[2]);
		m_pSelect[3]->Show(Flag_Show[3]);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{	
		InitFrame("show.xml");
		GetCtrlByName("back", &m_idBack);
		GetPrjShow(Flag_Show);
        /*  锁屏时间  */
		m_pWeek[0] = (CDPStatic *)GetCtrlByName("time_1");
		m_pWeek[1] = (CDPStatic *)GetCtrlByName("time_2");
		m_pWeek[2] = (CDPStatic *)GetCtrlByName("time_3");
		m_pWeek[3] = (CDPStatic *)GetCtrlByName("time_4");
        /*  勾选项  */
		m_pSelect[0] = (CDPStatic *)GetCtrlByName("select_1");
		m_pSelect[1] = (CDPStatic *)GetCtrlByName("select_2");
		m_pSelect[2] = (CDPStatic *)GetCtrlByName("select_3");
		m_pSelect[3] = (CDPStatic *)GetCtrlByName("select_4");
        /*  按键选项  */
		GetCtrlByName("ctl_data1", &m_idselect_1);                 
		GetCtrlByName("ctl_data2", &m_idselect_2);
		GetCtrlByName("ctl_data3", &m_idselect_3);
		GetCtrlByName("ctl_data4", &m_idselect_4);

		OnCreate();
		return TRUE;
	}

private:
	DWORD m_idBack;        // 返回按键
	DWORD m_idselect_1;    // 第一行勾选
	DWORD m_idselect_2;    // 第二行勾选
	DWORD m_idselect_3;    // 第三行勾选
	DWORD m_idselect_4;    // 第四行勾选

	CDPStatic* m_pWeek[4];
	CDPStatic* m_pSelect[4];
	
};

CAppBase* CreatePrjShow(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CShow* pApp = new CShow(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}