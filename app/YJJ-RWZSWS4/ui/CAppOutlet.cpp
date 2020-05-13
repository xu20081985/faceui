#include "CCtrlModules.h"

class COutletApp : public CAppBase
{
public:
    COutletApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~COutletApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if(m_dwTimeout++ == 30)
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                break;
            case MSG_BROADCAST:
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                break;
            default:
                break;
        }
        return TRUE;
    }

    void OnCreate(SmartDev *pSmartDev)
    {
        m_pSmartDev = pSmartDev;
        m_pSmartDev->cmd = (m_pSmartDev->cmd == SCMD_OPEN) ? SCMD_CLOSE : SCMD_OPEN;
        SendSmartCmd(&m_pSmartDev->device, m_pSmartDev->cmd, 0);
        DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        //InitFrame("outlet.xml");

        OnCreate((SmartDev *)lParam);
        return TRUE;
    }

private:
    SmartDev *m_pSmartDev;
};

CAppBase *CreateOutletApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    COutletApp *pApp = new COutletApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}