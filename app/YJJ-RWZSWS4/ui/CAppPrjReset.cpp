#include "CCtrlModules.h"

class CPrjResetApp : public CAppBase
{
public:
    CPrjResetApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjResetApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idOK)
                {
                    DPPostMessage(MSG_SYSTEM, RESET_MACH, 0, 0);
                }
                else if(wParam == m_idCancel)
                {
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("reset.xml");
        GetCtrlByName("ok", &m_idOK);
        GetCtrlByName("cancel", &m_idCancel);
        return TRUE;
    }

private:
    DWORD m_idOK;
    DWORD m_idCancel;
};

CAppBase *CreatePrjResetApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjResetApp *pApp = new CPrjResetApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}