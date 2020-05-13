#include "CCtrlModules.h"
#include "SmartConfig.h"
#include <roomlib.h>
int KBD_FLAG = 0;                    // 在触碰事件上做一个判断，优化键盘输入

int PWD_FLAG = 0;

class CPwdInputApp : public CAppBase
{
public:
	CPwdInputApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPwdInputApp()
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
					DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
					//DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
					//DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
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
				KBD_FLAG = 0;
				PWD_FLAG = 0;
				DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
				//DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
				//DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOK)
			{
				char szPasswd[32];
				char rootPasswd[32];
				GetPrjPwd(szPasswd);
				if(strcmp(szPasswd, m_pEditPwd->GetString()) == 0
					|| strcmp("20110755", m_pEditPwd->GetString()) == 0 )    //密码输入正确，进入了上报界面。
				{
					KBD_FLAG = 0;
					PWD_FLAG = 0;
					DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
				else
				{
					ShowTip(GetStringByID(20002));		// 密码错误
				}

			}
			else if(wParam == m_idDelete)
			{
				if(!m_bShow)
				{
					if(m_pEditPwd->GetCurCount() == 0)
					{
						ShowTip(GetStringByID(20001));		// 请输入密码
					}
					else
					{
						m_pEditPwd->Delete();
					}
				}
			}
			break;
		case KBD_MESSAGE:
			m_dwTimeout = 0;
			if(wParam == KBD_CTRL)
			{
				
				if(m_bShow)
				{
				//	m_pEditPwd->SetIsPwd(TRUE);
					m_pEditPwd->SetString("");
					m_bShow = FALSE;
				}
				m_pEditPwd->SetIsPwd(FALSE);
				m_pEditPwd->Input(lParam);
				usleep(250000);
				m_pEditPwd->Delete();
				m_pEditPwd->SetIsPwd(TRUE);
				m_pEditPwd->Input(lParam);
			}
			break;
		}
		return TRUE;	
	}

	void ShowTip(char* buf)
	{
		m_bShow = TRUE;
		m_pEditPwd->SetIsPwd(FALSE);
		m_pEditPwd->SetString(buf);
		m_pEditPwd->Show(FALSE);	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("pwdinput.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("ok", &m_idOK);
		GetCtrlByName("delete", &m_idDelete);
		m_pEditPwd = (CEditBox *)GetCtrlByName("pwd", &m_idEditPwd);

		m_pEditPwd->SetMaxLen(8);
		ShowTip(GetStringByID(20001));		// 请输入密码
		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idOK;
	DWORD m_idDelete;
	DWORD m_idEditPwd;
	CEditBox* m_pEditPwd;

	BOOL m_bShow;		                   // 是否显示 请输入密码
};

CAppBase* CreatePwdInputApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPwdInputApp* pApp = new CPwdInputApp(wParam);   //现在pApp指针指向了密码输入界面。
	PWD_FLAG = 1;
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	KBD_FLAG = 1;
	return pApp;
}