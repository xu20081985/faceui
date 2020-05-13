#include "CCtrlModules.h"
#include "SmartConfig.h"

class CSceneApp : public CAppBase
{
public:
	CSceneApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CSceneApp()
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:
		/*	if(m_dwTimeOut > 0)
			{
				m_dwTimeOut--;
				if(m_dwTimeOut == 0)
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}
			if(m_dwTick > 0)
			{
				int Xoff = GetUIConfig(FRAME_TIP_WIDTH) / 2;
				if(DPGetTickCount() > m_dwTick)
				{
					m_pPng->SetStart(Xoff, GetUIConfig(SCENE_FAIL_PNG_TOP));
					m_pPng->SetSrc("tip_fail_big.png");
					m_pPng->Show(TRUE);

					m_pText[0]->SetTextColor(GetUIConfig(COLOR_TEXT_FOCUS));
					m_pText[0]->SetStart(Xoff, GetUIConfig(SCENE_FAIL_TEXT_TOP1)); 
					m_pText[0]->SetSrc(GetStringByID(103));	// 场景控制失败！
					m_pText[0]->Show(TRUE);

					m_pText[1]->SetTextColor(GetUIConfig(COLOR_TEXT_FOCUS));
					m_pText[1]->SetStart(Xoff, GetUIConfig(SCENE_FAIL_TEXT_TOP2)); 
					m_pText[1]->SetSrc(GetStringByID(104));	// 请检查网络连接设置！
					m_pText[1]->Show(TRUE);

					m_dwTick = 0;
					m_dwTimeOut = TIP_TIMEOUT_FAIL;
				}
			}*/
			break;
		case MSG_BROADCAST:
			/*if(wParam == SMART_STATUS_SCENE)
			{
				if(zParam == htons(m_pSmartDev->addr))
				{
					// 正在显示的时候，直接退出
					if(m_dwTimeOut > 0)
						break;

					char buf[128];
					sprintf(buf, "%s%s", m_pSmartDev->name, GetStringByID(105));	// 启用成功！
					m_pText[0]->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2, GetUIConfig(LIGHT_TEXT_TOP)); 
					m_pText[0]->SetTextColor(GetUIConfig(COLOR_TEXT_FOCUS));
					m_pText[0]->SetSrc(buf);
					m_pText[0]->Show(TRUE);

					// 移动坐标，显示笑脸
					m_pTextPng->SetSrc("tip_ok.png");
					m_pTextPng->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2 + m_pText[0]->GetWidth() / 2, GetUIConfig(LIGHT_TEXT_PNG_TOP));
					m_pTextPng->Show(TRUE);

					// 显示图标
					m_pPng->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2, GetUIConfig(LIGHT_PNG_TOP));
					m_pPng->SetSrc("frame_light.png");
					m_pPng->Show(TRUE);

					m_dwTick = 0;
					m_dwTimeOut = TIP_TIMEOUT_OK;
				}
			}*/
			break;
		}
		return TRUE;	
	}

	void OnCreate(SmartDev* pSmartDev)
	{
		m_pSmartDev = pSmartDev;

		SendSmartCmd(SCENE_GRUOP_ADDR, SCMD_SCENE, htons(pSmartDev->addr));

		// 情景控制直接显示启用成功，不用管回复，张广说可能会丢失回复数据
		SetStatusByScene(htons(pSmartDev->addr));
		DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);

		// 显示启用成功
/*		char buf[128];
		sprintf(buf, "%s%s", m_pSmartDev->name, GetStringByID(105));	// 启用成功！
		m_pText[0]->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2, GetUIConfig(LIGHT_TEXT_TOP)); 
		m_pText[0]->SetTextColor(GetUIConfig(COLOR_TEXT_FOCUS));
		m_pText[0]->SetSrc(buf);
		m_pText[0]->Show(TRUE);

		// 移动坐标，显示笑脸
		m_pTextPng->SetSrc("tip_ok.png");
		m_pTextPng->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2 + m_pText[0]->GetWidth() / 2, GetUIConfig(LIGHT_TEXT_PNG_TOP));
		m_pTextPng->Show(TRUE);

		// 显示图标
		m_pPng->SetStart(GetUIConfig(FRAME_TIP_WIDTH) / 2, GetUIConfig(LIGHT_PNG_TOP));
		m_pPng->SetSrc("frame_light.png");
		m_pPng->Show(TRUE);

		m_dwTick = 0;
		m_dwTimeOut = TIP_TIMEOUT_OK;*/

		//m_dwTimeOut = 0;
		//m_dwTick = DPGetTickCount() + TIP_TIMEOUT_ACK * 1000;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
	/*	InitFrame("scene.xml");
		m_pPng = (CDPStatic *)GetCtrlByName("png");
		m_pText[0] = (CDPStatic *)GetCtrlByName("text1");
		m_pText[1] = (CDPStatic *)GetCtrlByName("text2");
		m_pTextPng = (CDPStatic *)GetCtrlByName("text_png");*/
		OnCreate((SmartDev *)lParam);
		return TRUE;
	}

private:
	CDPStatic* m_pPng;
	CDPStatic* m_pText[2];
	CDPStatic* m_pTextPng;

	DWORD m_dwTick;
	DWORD m_dwTimeOut;
	SmartDev* m_pSmartDev;
};

CAppBase* CreateSceneApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSceneApp* pApp = new CSceneApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}