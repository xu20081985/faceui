#include "CCtrlModules.h"
#include "SmartConfig.h"

class CWindowApp : public CAppBase
{
public:
    CWindowApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CWindowApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                break;
            case TOUCH_SLIDE:
                DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                break;
            case MSG_BROADCAST:
                if(wParam == SMART_STATUS_SYNC)
                {
                    UpdateStatus();
                }
                break;
            case TOUCH_MESSAGE:
                if(wParam == m_idBack)
                {
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if(wParam == m_idOpen)
                {
                    SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_OPEN, 100);
                }
                else if(wParam == m_idStop)
                {
                    SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_STOP, 0);
                }
                else if(wParam == m_idClose)
                {
                    SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_CLOSE, 0);
                }
                else if(wParam == m_idProgress)
                {
                    char buf[32];
                    sprintf(buf, "%d%%", lParam);
                    m_pPercent->SetSrc(buf);
                    m_pPercent->Show(TRUE);

                    if(lParam)
                        SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_OPEN, lParam);
                    else
                        SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_CLOSE, 0);
                }
                break;
        }
        return TRUE;
    }

    void UpdateStatus()
    {
        // 显示百分比
        char buf[32];
        sprintf(buf, "%d%%", m_pSmartDev->status);
        m_pPercent->SetSrc(buf);
        m_pPercent->Show(TRUE);

        // 显示进度条
        m_pProgress->SetProgressCur(m_pSmartDev->status);
        m_pProgress->Show();
    }

    void OnCreate(SmartDev *pSmartDev)
    {
        m_pSmartDev = pSmartDev;
        // 显示名称
        m_pTitle->SetSrc(pSmartDev->name);
        m_pTitle->Show(TRUE);
        // 更新状态
        UpdateStatus();
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("window.xml");
        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("stop", &m_idStop);
        GetCtrlByName("open", &m_idOpen);
        GetCtrlByName("close", &m_idClose);
        m_pTitle = (CDPStatic *)GetCtrlByName("title");
        m_pPercent = (CDPStatic *)GetCtrlByName("percent");
        m_pProgress = (CDPProgress *)GetCtrlByName("progress", &m_idProgress);

        OnCreate((SmartDev *)lParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idStop;
    DWORD m_idOpen;
    DWORD m_idClose;
    DWORD m_idProgress;
    CDPStatic *m_pTitle;
    CDPStatic *m_pPercent;
    CDPProgress *m_pProgress;

    SmartDev *m_pSmartDev;
};

CAppBase *CreateWindowApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CWindowApp *pApp = new CWindowApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}