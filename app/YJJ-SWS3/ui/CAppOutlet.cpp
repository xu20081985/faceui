#include "CCtrlModules.h"
#include "SmartConfig.h"

class COutletApp : public CAppBase
{
public:
	COutletApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~COutletApp()
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:
			if(m_dwTimeOut > 0)
			{
				m_dwTimeOut--;
				if(m_dwTimeOut == 0)
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}
			if(m_dwTick > 0)
			{
				if(DPGetTickCount() > m_dwTick)
				{
					m_pPng->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2, GetUIConfig(DEV_FAIL_PNG_TOP));
					m_pPng->SetSrc("tip_fail_big.png");
					m_pPng->Show(TRUE);

					m_pText->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2, GetUIConfig(DEV_FAIL_TEXT_TOP)); 
					m_pText->SetTextColor(GetUIConfig(COLOR_TEXT_NORMAL));
					m_pText->SetSrc(GetStringByID(100));	// 设备控制失败，请检查网络链接！
					m_pText->Show(TRUE);

					m_dwTick = 0;
					m_dwTimeOut = TIP_TIMEOUT_FAIL;
				}
			}
			break;
		case MSG_BROADCAST:
			if(wParam == SMART_STATUS_ACK)
			{
#ifndef DPCE
				if(lParam == m_pSmartDev->addr)
#endif
				{
					char buf[128];
					char szPng[128];
					if(m_pSmartDev->status == 0)
					{
						strcpy(szPng, "frame_outlet.png");
						sprintf(buf, "%s%s", m_pSmartDev->name, GetStringByID(101));	// 关闭成功！
					}
					else
					{
						strcpy(szPng, "frame_outletp.png");
						sprintf(buf, "%s%s", m_pSmartDev->name, GetStringByID(102));	// 开启成功！
					}

					m_pText->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2, GetUIConfig(LIGHT_TEXT_TOP)); 
					m_pText->SetSrc(buf);
					m_pText->Show(TRUE);

					// 移动坐标，显示笑脸
					m_pTextPng->SetSrc("tip_ok.png");
					m_pTextPng->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2 + m_pText->GetWidth() / 2, GetUIConfig(LIGHT_TEXT_PNG_TOP));
					m_pTextPng->Show(TRUE);

					// 显示灯光图标
					m_pPng->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2, GetUIConfig(LIGHT_PNG_TOP));
					m_pPng->SetSrc(szPng);
					m_pPng->Show(TRUE);

					m_dwTick = 0;
					m_dwTimeOut = TIP_TIMEOUT_OK;
				}
			}
			break;
		}
		return TRUE;	
	}

	void OnCreate(SmartDev* pSmartDev)
	{
		if(pSmartDev->status > 0)
			SendSmartCmd(pSmartDev->addr, SCMD_CLOSE, 0);
		else
			SendSmartCmd(pSmartDev->addr, SCMD_OPEN, 0);

		m_dwTimeOut = 0;
		m_pSmartDev = pSmartDev;
		m_dwTick = DPGetTickCount() + TIP_TIMEOUT_ACK * 1000;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("outlet.xml");
		m_pPng = (CDPStatic *)GetCtrlByName("tip_png");
		m_pText = (CDPStatic *)GetCtrlByName("tip_text");
		m_pTextPng = (CDPStatic *)GetCtrlByName("tip_text_png");

		OnCreate((SmartDev *)lParam);
		return TRUE;
	}

private:
	DWORD m_dwTimeOut;
	CDPStatic* m_pText;
	CDPStatic* m_pTextPng;
	CDPStatic* m_pPng;

	SmartDev* m_pSmartDev;
	DWORD m_dwTick;
};

CAppBase* CreateOutletApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	COutletApp* pApp = new COutletApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}