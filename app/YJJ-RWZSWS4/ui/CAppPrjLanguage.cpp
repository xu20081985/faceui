#include "CCtrlModules.h"

class CPrjLanguageApp : public CAppBase
{
public:
    CPrjLanguageApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjLanguageApp()
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
        InitFrame("prj_language.xml");
        return TRUE;
    }

private:
};

CAppBase *CreatePrjLanguageApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjLanguageApp *pApp = new CPrjLanguageApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}