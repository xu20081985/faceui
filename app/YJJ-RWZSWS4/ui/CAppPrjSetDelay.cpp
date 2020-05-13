#include "CCtrlModules.h"

class CPrjSetDelayApp : public CAppBase
{
public:
    CPrjSetDelayApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjSetDelayApp()
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
        InitFrame("prj_setdelay.xml");
        return TRUE;
    }

private:
};

CAppBase *CreatePrjSetDelayApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjSetDelayApp *pApp = new CPrjSetDelayApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}