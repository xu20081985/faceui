#include "CCtrlModules.h"

BOOL  Light_select = FALSE;
char* p_Light = NULL;

extern BOOL E_Tim;
extern pNode p_Edit;       //定时事件编辑时参数的结构体指针变量

extern char DW[];
class CTimerDeviceApp : public CAppBase
{
public:
	CTimerDeviceApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTimerDeviceApp()
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
					
					if(NULL !=p_Light)                            //控制设备字符串赋值
						strcpy(p_Edit->Device, p_Light);        
					else
		 				strcpy(p_Edit->Device, DW);
		 	
		 			p_Edit->Device_Status = Light_select;         //控制设备勾选标记
		 			
					p_Light = NULL;                               //将指针清空
					Light_select =FALSE;
					
					DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_EDIT, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
			else if(wParam == m_idDevice_1) {  //选择了设备1

				if(Light_select) {

					 Light_select = FALSE;
					 p_Light = NULL;
				}

				else {

					Light_select = TRUE;
					p_Light = GetStringByID(13209);
				}
		    }
			OnPage(0);
			break;
		}
		return TRUE;	
	}

	void OnPage(DWORD dwPage)
	{
		if(!E_Tim) {
	
			if(dwPage == 0)
			{
				m_pDevice[0]->SetSrc(GetStringByID(13209)); 	// 灯光
				m_pDevice[0]->Show(TRUE);
				m_pSelect->Show(Light_select);
			}
		}

		else {

			m_pDevice[0]->SetSrc(GetStringByID(13209)); 	    // 灯光
			m_pDevice[0]->Show(TRUE);
			m_pSelect->Show(Light_select);
		}

		m_dwPage = dwPage;
	}

	void OnCreate()
	{
		OnPage(0);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("timer_device.xml");
		GetCtrlByName("ctl_device1", &m_idDevice_1);
		GetCtrlByName("back", &m_idBack);
		m_pDevice[0] = (CDPStatic *)GetCtrlByName("dev_1");
		m_pDevice[1] = (CDPStatic *)GetCtrlByName("dev_2");
		m_pDevice[2] = (CDPStatic *)GetCtrlByName("dev_3");
		m_pDevice[3] = (CDPStatic *)GetCtrlByName("dev_4");
		m_pSelect = (CDPStatic *)GetCtrlByName("select");

		if(E_Tim) {

			Light_select = p_Edit->Device_Status;

			if(Light_select)
				p_Light = GetStringByID(13209);
			else
				p_Light = NULL;				
		}
		OnCreate(); 
		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idDevice_1;                        //设备选择控制变量
	CDPStatic* m_pDevice[4];
	CDPStatic* m_pSelect;

	DWORD m_dwPage;
};

CAppBase* CreateTimerDeviceApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTimerDeviceApp* pApp = new CTimerDeviceApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}
