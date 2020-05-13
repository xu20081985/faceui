#include "CCtrlModules.h"
#include "SmartConfig.h"

class CMechine : public CAppBase
{
public:
	CMechine(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CMechine()
	{
	}

	void ResumeAck(void)
	{
		m_dwTimeout = 0;
		OnPage();
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
			if(wParam == m_idButton[0] || wParam == m_idButton[5])     {   //密码设置

				DPPostMessage(MSG_START_APP, PRJ_SET_PWD_APPID, 0, 0);		
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idButton[1]|| wParam == m_idButton[6]) {  //时钟设置

				DPPostMessage(MSG_START_APP, PRJ_SET_DATE_APPID, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);  
			} 

			else if(wParam == m_idButton[2] || wParam == m_idButton[7]) {  //背景设置

				DPPostMessage(MSG_START_APP, BKGD_SET, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idButton[3] || wParam == m_idButton[8]) {  //显示设置

				DPPostMessage(MSG_START_APP, PRJ_SHOW_APPID, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idButton[9] || wParam == m_idButton[10]) {  //温度校准

				DPPostMessage(MSG_START_APP, PRJ_TEMP_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idButton[4]) {  //返回按键

				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
		}
		return TRUE;	
	}

	void OnPage()
	{

		m_pButton[0]->SetSrcpng("lock.png");
		m_pButton[0]->SetSrcpng(STATUS_NORMAL, "lock.png");
		m_pButton[0]->Show(STATUS_NORMAL);

		m_pButton[1]->SetSrcpng("timen.png");
		m_pButton[1]->SetSrcpng(STATUS_NORMAL, "timen.png");
		m_pButton[1]->Show(STATUS_NORMAL);

		m_pButton[2]->SetSrcpng("bkgroud.png");
		m_pButton[2]->SetSrcpng(STATUS_NORMAL, "bkgroud.png");
		m_pButton[2]->Show(STATUS_NORMAL);

		m_pButton[3]->SetSrcpng("showid.png");
		m_pButton[3]->SetSrcpng(STATUS_NORMAL, "showid.png");
		m_pButton[3]->Show(STATUS_NORMAL);

		m_preport[0]->SetSrc(GetStringByID(4001));     // 密码
		m_preport[0]->Show(TRUE);

		m_preport[1]->SetSrc(GetStringByID(4008));     // 时钟
		m_preport[1]->Show(TRUE);

		m_preport[2]->SetSrc(GetStringByID(4003));     // 背景
		m_preport[2]->Show(TRUE);

		m_preport[3]->SetSrc(GetStringByID(4009));     // 显示
		m_preport[3]->Show(TRUE);

		
		m_pButton[10]->SetSrcpng("showid.png");
		m_pButton[10]->SetSrcpng(STATUS_NORMAL, "showid.png");
		m_pButton[10]->Show(STATUS_NORMAL);

		m_preport[4]->SetSrc(GetStringByID(4007));    // 温度校准
		m_preport[4]->Show(TRUE);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("project1.xml");
		m_pButton[0] = (CDPButton *)GetCtrlByName("button1", &m_idButton[0]);
		m_pButton[1] = (CDPButton *)GetCtrlByName("button2", &m_idButton[1]);
		m_pButton[2] = (CDPButton *)GetCtrlByName("button3", &m_idButton[2]);   //点击任意一处都有效果
		m_pButton[3] = (CDPButton *)GetCtrlByName("button4", &m_idButton[3]);
		m_pButton[4] = (CDPButton *)GetCtrlByName("back", &m_idButton[4]);
		m_pButton[10] = (CDPButton *)GetCtrlByName("button5", &m_idButton[10]);

		m_pButton[5] = (CDPButton *)GetCtrlByName("next1", &m_idButton[5]);
		m_pButton[6] = (CDPButton *)GetCtrlByName("next2", &m_idButton[6]);
		m_pButton[7] = (CDPButton *)GetCtrlByName("next3", &m_idButton[7]);
		m_pButton[8] = (CDPButton *)GetCtrlByName("next4", &m_idButton[8]);
		m_pButton[9] = (CDPButton *)GetCtrlByName("next5", &m_idButton[9]);
		
		m_preport[0] = (CDPStatic *)GetCtrlByName("report1");
		m_preport[1] = (CDPStatic *)GetCtrlByName("report2");
		m_preport[2] = (CDPStatic *)GetCtrlByName("report3");
		m_preport[3] = (CDPStatic *)GetCtrlByName("report4");
		m_preport[4] = (CDPStatic *)GetCtrlByName("report5");
		OnPage();
		return TRUE;
	}

private:

	DWORD m_nPage;     // 页数
	
	DWORD m_dwCount;
	DWORD m_idButton[11];
	CDPButton* m_pButton[11];
	CDPStatic* m_preport[5];
};

CAppBase* CreateMechineApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CMechine* pApp = new CMechine(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}
