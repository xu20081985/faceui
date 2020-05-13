#include "CCtrlModules.h"
#include "SmartConfig.h"
int cc = 0;
#define _DEBUG 1;
//char bufv[] = "V1.01.07";
class CProjectApp : public CAppBase
{
public:
	CProjectApp(DWORD hWnd) : CAppBase(hWnd)
	{

	}

	~CProjectApp()
	{

	}

	void ResumeAck(void)
	{
		m_dwTimeout = 0;
		OnPage(m_nPage);
		return CAppBase::ResumeAck();
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
					DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);			
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
			break;
		case TOUCH_SLIDE:
			m_dwTimeout = 0;
			break;			
#if 1
		case TOUCH_MESSAGE:
			m_dwTimeout = 0;

			if(wParam == m_idButton[0] || wParam == m_idButton[5])      { 	//定时设置

				DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_APPID, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idButton[1]|| wParam == m_idButton[6]) {  //本机设置

				DPPostMessage(MSG_START_APP, MECHINE_SET, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				
			}

			else if(wParam == m_idButton[2]|| wParam == m_idButton[7]) {  //上报ID设置

				DPPostMessage(MSG_START_FROM_OVER, PRJ_REPORT_APPID, 0, 0);
			}

			else if(wParam == m_idButton[3]|| wParam == m_idButton[8]) {  //恢复出厂设置

				DPPostMessage(MSG_START_FROM_OVER, PRJ_RESET_APPID, 0, 0);
			}

			else if(wParam == m_idButton[4]) {  //返回按键

				DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);	
			}
			break; 
#endif

#if 0 
		case TOUCH_MESSAGE:
				if(wParam == m_idEmpty)
			{
				cc++;
				if(cc == 8)
					cc =0;
				
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
#endif
		return TRUE;	
		}
	}
	void OnPage(DWORD nPage)
	{
		
#if 1	
	    // 定时设置
		m_pButton[0]->SetSrcpng("timer.png");
		m_pButton[0]->SetSrcpng(STATUS_NORMAL, "timer.png");
		m_pButton[0]->Show(STATUS_NORMAL);

		// 关于本机
		m_pButton[1]->SetSrcpng("setmechine.png");
		m_pButton[1]->SetSrcpng(STATUS_NORMAL, "setmechine.png");
		m_pButton[1]->Show(STATUS_NORMAL);
			
		// 上报ID
		m_pButton[2]->SetSrcpng("reportID.png");
		m_pButton[2]->SetSrcpng(STATUS_NORMAL, "reportID.png");
		m_pButton[2]->Show(STATUS_NORMAL);

		// 恢复出厂
		m_pButton[3]->SetSrcpng("restart.png");
		m_pButton[3]->SetSrcpng(STATUS_NORMAL, "restart.png");
		m_pButton[3]->Show(STATUS_NORMAL);

		m_preport[0]->SetSrc(GetStringByID(4011));
		m_preport[0]->Show(TRUE);

		m_preport[1]->SetSrc(GetStringByID(4010));
		m_preport[1]->Show(TRUE);

		m_preport[2]->SetSrc(GetStringByID(4000));
		m_preport[2]->Show(TRUE);

		m_preport[3]->SetSrc(GetStringByID(4005));
		m_preport[3]->Show(TRUE);

		m_preport[4]->SetSrc(GetStringByID(12001));
		m_preport[4]->Show(TRUE);

		char buf[256];
		DWORD softVer = GetSoftVer();
		sprintf(buf, "V%d.%02d.%02d", (softVer >> 24) & 0xFF, (softVer >> 16) & 0xFF, (softVer >> 8) & 0xFF);

		m_preport[5]->SetSrc(buf);
		m_preport[5]->Show(TRUE);
		
#endif
			 
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
#if 0		
		if(cc == 0)
			InitFrame("project.xml");
		else if(cc == 1)
			InitFrame("project0-1.xml");
		else if(cc == 2)
			InitFrame("project1-0.xml");
		else if(cc == 3)
			InitFrame("project1-1.xml");
		else if(cc == 4)
			InitFrame("project2-0.xml");
		else if(cc == 5)
			InitFrame("project2-1.xml");
		else if(cc == 6)
			InitFrame("project2-2.xml");
		else if(cc == 7)
			InitFrame("project2-3.xml");
		GetCtrlByName("ebutton", &m_idEmpty); 
#endif		
		/* 这里改成为五个按键的控制 */
#if 1
		InitFrame("project.xml");
		m_pButton[0] = (CDPButton *)GetCtrlByName("button1", &m_idButton[0]);
		m_pButton[1] = (CDPButton *)GetCtrlByName("button2", &m_idButton[1]);
		m_pButton[2] = (CDPButton *)GetCtrlByName("button3", &m_idButton[2]);
		m_pButton[3] = (CDPButton *)GetCtrlByName("button4", &m_idButton[3]);
	    m_pButton[4] = (CDPButton *)GetCtrlByName("back", &m_idButton[4]);

		m_pButton[5] = (CDPButton *)GetCtrlByName("next1", &m_idButton[5]);
		m_pButton[6] = (CDPButton *)GetCtrlByName("next2", &m_idButton[6]);
		m_pButton[7] = (CDPButton *)GetCtrlByName("next3", &m_idButton[7]);
		m_pButton[8] = (CDPButton *)GetCtrlByName("next4", &m_idButton[8]);
	//	m_pButton[5] = (CDPButton *)GetCtrlByName("button5", &m_idButton[5]);

		m_preport[0] = (CDPStatic *)GetCtrlByName("report1");
		m_preport[1] = (CDPStatic *)GetCtrlByName("report2");
		m_preport[2] = (CDPStatic *)GetCtrlByName("report3");
		m_preport[3] = (CDPStatic *)GetCtrlByName("report4");
		m_preport[4] = (CDPStatic *)GetCtrlByName("report5");
		m_preport[5] = (CDPStatic *)GetCtrlByName("report6");
		OnPage(lParam);
		m_dwTimeout = 0;
#endif	
		return TRUE;
	}

private:
	int m_nPage;
	
	
	DWORD m_idEmpty;
	DWORD m_dwCount;
	DWORD m_idButton[10];
	CDPButton* m_pButton[10];
	CDPStatic* m_preport[6];
};

CAppBase* CreateProjectApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CProjectApp* pApp = new CProjectApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}