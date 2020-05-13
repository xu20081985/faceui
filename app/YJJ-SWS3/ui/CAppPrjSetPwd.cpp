#include "CCtrlModules.h"
#include "SmartConfig.h"
#include <roomlib.h>
int SET_PWD = 0;

enum
{
	STAGE_FIRST,		// 第一次输入
	STAGE_AGAIN,		// 再次输入
	STAGE_OK			// 修改完毕，等待退出	
};

class CPrjSetPwdApp : public CAppBase
{
public:
	CPrjSetPwdApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPrjSetPwdApp()
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
					SET_PWD = 0; 
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
				SET_PWD = 0;
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOK)
			{
				if(m_bShow)
					break;

				if(m_Stage == STAGE_FIRST)
				{
					m_Stage = STAGE_AGAIN;
					strcpy(m_szPasswd, m_pEditPwd->GetString());
					ShowTip(GetStringByID(15002));		   // 请重新输入您设置的密码！
				}
				else
				{
					if(strcmp(m_szPasswd, m_pEditPwd->GetString()) == 0)
					{
						m_Stage = STAGE_OK;
						SetPrjPwd(m_szPasswd);
						
						m_pLayOutTip->SwitchLay(TRUE);      // 设置成功
						usleep(2 * 1000 * 1000);
						DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
						DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);		
					}
					else
					{
						m_Stage = STAGE_FIRST;
						ShowTip(GetStringByID(20002));		// 密码错误
					}
				}
			}
			else if(wParam == m_idDelete)
			{
				if(!m_bShow)
				{
					if(m_pEditPwd->GetCurCount() == 0)
					{
						if(m_Stage == STAGE_FIRST)
							ShowTip(GetStringByID(15001));	 // 请输入您想要设置的密码！
						else
							ShowTip(GetStringByID(15002));	 // 请重新输入您设置的密码！
					}
					else
					{
						m_pEditPwd->Delete();
					}
				}
			}
			break;
		case KBD_MESSAGE:
			if(wParam == KBD_CTRL)
			{
				if(m_Stage == STAGE_OK)
					break;

				if(m_bShow)
				{
				//	m_pEditPwd->SetIsPwd(TRUE);
					m_pEditPwd->SetString("");
					m_bShow = FALSE;
				}
				if(m_pEditPwd->GetCurCount() < 8)
				{
					m_pEditPwd->SetIsPwd(FALSE);
					m_pEditPwd->Input(lParam);
					usleep(250000);
					m_pEditPwd->Delete();
					m_pEditPwd->SetIsPwd(TRUE);
					m_pEditPwd->Input(lParam);
				}
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
		InitFrame("prj_setpwd.xml");
		GetCtrlByName("back", &m_idBack);                             //返回按键
		GetCtrlByName("ok", &m_idOK);                                 //确定按键
		GetCtrlByName("delete", &m_idDelete);                         //删除按键
		m_pEditPwd = (CEditBox *)GetCtrlByName("pwd", &m_idEditPwd);
		m_pLayOutTip = (CLayOut *)GetCtrlByName("tip_frame");         //设置成功提示

		m_Stage = STAGE_FIRST;
		ShowTip(GetStringByID(15001));		                          // 请输入您想要设置的密码 
		return TRUE;
	} 

private:
	DWORD m_idBack;
	DWORD m_idOK;
	DWORD m_idDelete;
	DWORD m_idEditPwd;
	CEditBox* m_pEditPwd;
	CLayOut* m_pLayOutTip;

	BOOL m_bShow;
	DWORD m_Stage;
	char m_szPasswd[32];
};

CAppBase* CreatePrjSetPwdApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjSetPwdApp* pApp = new CPrjSetPwdApp(wParam);
	SET_PWD = 1;
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}