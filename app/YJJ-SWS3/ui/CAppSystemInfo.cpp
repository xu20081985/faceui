#include "CCtrlModules.h"

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
		}
		return TRUE;	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prj_info.xml");
		return TRUE;
	}

private:
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