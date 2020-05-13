#include "CCtrlModules.h"

class CPrjUpgradeApp : public CAppBase
{
public:
	CPrjUpgradeApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPrjUpgradeApp()
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:
			break;
		case TOUCH_MESSAGE:
			if(wParam == m_idBack)
			{
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			if(wParam == m_idLoad)
			{
				
			}
			break;
		}
		return TRUE;	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prj_upgrade.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("load", &m_idLoad);
		m_pVersion = (CDPStatic *)GetCtrlByName("version");

		char buf[64];
		DWORD version = GetVersion();
		sprintf(buf, "%s A%d.%d.%d.%d", GetStringByID(18000), (version >> 12) & 0xF, (version >> 8) & 0xF, (version >> 4) & 0xF, (version) & 0xF);		// µ±Ç°°æ±¾ºÅ:
		m_pVersion->SetSrc(buf);
		m_pVersion->Show(TRUE);

		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idLoad;
	CDPStatic* m_pVersion;
};

CAppBase* CreatePrjUpgradeApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjUpgradeApp* pApp = new CPrjUpgradeApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}