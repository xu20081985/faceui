#include "CCtrlModules.h"

#define YJJ_SWS3	"YJJ-SWS3-X1"

class CPrjInfoApp : public CAppBase
{
public:
	CPrjInfoApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPrjInfoApp()
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
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
		}
		return TRUE;		
	}

	void OnCreate()
	{
		char buf[256];
		sprintf(buf, "%s: %s", GetStringByID(12002), YJJ_SWS3);		// ÐÍºÅ£ºYJJ-SWS3-X1
		m_pText[0]->SetSrc(buf);
		m_pText[0]->Show(TRUE);

		DWORD softVer = GetSoftVer();
		sprintf(buf, "%s: %d.%02d.%02d", GetStringByID(12001), (softVer >> 24) & 0xFF, (softVer >> 8) & 0xFF, softVer & 0xFF);		// °æ±¾ºÅ£º1.00.00
		m_pText[1]->SetSrc(buf);
		m_pText[1]->Show(TRUE);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prj_info.xml");
		GetCtrlByName("back", &m_idBack);
		m_pText[0] = (CDPStatic *)GetCtrlByName("soft");
		m_pText[1] = (CDPStatic *)GetCtrlByName("version");

		OnCreate();
		return TRUE;
	}

private:
	DWORD m_idBack;
	CDPStatic* m_pText[2];
};

CAppBase* CreatePrjInfoApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjInfoApp* pApp = new CPrjInfoApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}