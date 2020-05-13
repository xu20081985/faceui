#include "CCtrlModules.h"
#include "SmartConfig.h"

class CPrjCalendarApp : public CAppBase
{
public:
    CPrjCalendarApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjCalendarApp()
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
        InitFrame("prj_calendar.xml");
        return TRUE;
    }

private:
};

CAppBase *CreatePrjCalendarApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjCalendarApp *pApp = new CPrjCalendarApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}