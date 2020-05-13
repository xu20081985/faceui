#include "CCtrlModules.h"

class CPrjSetUIApp : public CAppBase
{
public:
	CPrjSetUIApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPrjSetUIApp()
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
		InitFrame("prj_setui.xml");
		return TRUE;
	}

private:
};

CAppBase* CreatePrjSetUIApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjSetUIApp* pApp = new CPrjSetUIApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}