#include "CCtrlModules.h"
#include "SmartConfig.h"

class CPrjReportApp : public CAppBase
{
public:
    CPrjReportApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjReportApp()
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
                    SmartReportID();
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
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
        InitFrame("report.xml");
        GetCtrlByName("ok", &m_idOK);
        GetCtrlByName("cancel", &m_idCancel);
        return TRUE;
    }

private:
    DWORD m_idOK;
    DWORD m_idCancel;
};

CAppBase *CreatePrjReportApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjReportApp *pApp = new CPrjReportApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}