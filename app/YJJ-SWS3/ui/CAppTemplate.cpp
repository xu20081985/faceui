#include "CCtrlModules.h"

class CTempApp : public CAppBase
{
public:
	CTempApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTempApp()
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:
			break;
		}
		return TRUE;	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("curtain.xml");
		return TRUE;
	}

private:
};

CAppBase* CreateTempApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTempApp* pApp = new CTempApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}