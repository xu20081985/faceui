#include "CCtrlModules.h"
#include "SmartConfig.h"

class CTipApp: public CAppBase
{
public:
	CTipApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTipApp()
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
			break;
		}
		return TRUE;	
	}

	void OnCreate(DWORD lParam)
	{
		SmartDev* pSmartDev = (SmartDev *)lParam;
		int Xoff = GetUIConfig(FRAME_TIP_WIDTH) / 2;

		switch(pSmartDev->type)
		{
			case ST_SCENE_A:
			case ST_SCENE_B:
			case ST_SCENE_C:
			case ST_SCENE_D:
			case ST_SCENE_E:
			case ST_SCENE_F:
			case ST_SCENE_G:
			case ST_SCENE_H:
			case ST_SCENE_I:
			case ST_SCENE_J:
			case ST_SCENE_K:
			case ST_SCENE_L:
			case ST_SCENE_M:
			case ST_SCENE_N:
			case ST_SCENE_O:
			case ST_SCENE_P:
			case ST_SCENE_Q:
			case ST_SCENE_R:
			case ST_SCENE_S:
			case ST_SCENE_T:
			case ST_SCENE_U:
			case ST_SCENE_V:
			case ST_SCENE_W:
			case ST_SCENE_X:
			case ST_SCENE_Y:
			case ST_SCENE_Z:
				if(pSmartDev->addr == INVALID_ECB_ADDR)
				{
					m_pPng->SetStart(Xoff, GetUIConfig(SCENE_FAIL_PNG_TOP));
					m_pPng->SetSrc("tip_fail_big.png");
					m_pPng->Show(TRUE);

					m_pText[0]->SetTextColor(GetUIConfig(COLOR_TEXT_FOCUS));
					m_pText[0]->SetStart(Xoff, GetUIConfig(SCENE_FAIL_TEXT_TOP1)); 
					m_pText[0]->SetSrc(GetStringByID(103));	// ³¡¾°¿ØÖÆÊ§°Ü£¡
					m_pText[0]->Show(TRUE);

					m_pText[1]->SetTextColor(GetUIConfig(COLOR_TEXT_FOCUS));
					m_pText[1]->SetStart(Xoff, GetUIConfig(SCENE_FAIL_TEXT_TOP2)); 
					m_pText[1]->SetSrc(GetStringByID(104));	// Çë¼ì²éÍøÂçÁ¬½ÓÉèÖÃ£¡
					m_pText[1]->Show(TRUE);
				}
				break;
			default:
				if(pSmartDev->addr == INVALID_ECB_ADDR)
				{
					m_pPng->SetStart(Xoff, GetUIConfig(DEV_FAIL_PNG_TOP));
					m_pPng->SetSrc("tip_fail_big.png");
					m_pPng->Show(TRUE);

					m_pText[0]->SetTextColor(GetUIConfig(COLOR_TEXT_NORMAL));
					m_pText[0]->SetStart(Xoff, GetUIConfig(DEV_FAIL_TEXT_TOP)); 
					m_pText[0]->SetSrc(GetStringByID(100));	// Éè±¸¿ØÖÆÊ§°Ü£¬Çë¼ì²éÍøÂçÁ´½Ó£¡
					m_pText[0]->Show(TRUE);
				}
				break;
		}

		m_dwTimeOut = TIP_TIMEOUT_OK;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("tip.xml");

		m_pPng = (CDPStatic *)GetCtrlByName("png");
		m_pText[0] = (CDPStatic *)GetCtrlByName("text1");
		m_pText[1] = (CDPStatic *)GetCtrlByName("text2");

		OnCreate(lParam);
		return TRUE;
	}

private:
	CDPStatic* m_pPng;
	CDPStatic* m_pText[2];

	DWORD m_dwTimeOut;
};

CAppBase* CreateTipApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTipApp* pApp = new CTipApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}