#include "CCtrlModules.h"

class CPrjTextApp : public CAppBase
{
public:
    CPrjTextApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjTextApp()
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
        InitFrame("prj_text.xml");
        return TRUE;
    }

private:
};

CAppBase *CreatePrjTextApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjTextApp *pApp = new CPrjTextApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}