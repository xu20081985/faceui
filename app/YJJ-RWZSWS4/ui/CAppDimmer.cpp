#include "CCtrlModules.h"

class CDimmerApp : public CAppBase
{
public:
    CDimmerApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CDimmerApp()
    {
    }
    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch (uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                break;
            case MSG_BROADCAST:
                if (wParam == SMART_STATUS_SYNC)
                {
                    UpdateStatus();
                }
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if (wParam == m_idAdd)
                {
                    m_pSmartDev->cmd = SCMD_OPEN;
                    m_pSmartDev->status = m_pSmartDev->param1;
                    show();
                    SendSmartCmd(&m_pSmartDev->device, SCMD_OPEN, 0);
                }
                else if (wParam == m_idSub)
                {
                    m_pSmartDev->cmd = SCMD_CLOSE;
                    m_pSmartDev->status = 0;
                    show();
                    SendSmartCmd(&m_pSmartDev->device, SCMD_CLOSE, 0);
                }
                else if (wParam == m_idProgress)
                {
                    if (zParam == 1)
                    {
                        sprintf(m_buf, "%d", lParam);
                        m_pPercent->SetSrc(m_buf);
                        m_pPercent->Show(TRUE);
                        m_pProgress->SetProgressCur(lParam);
                        m_pProgress->Show();
                        break;
                    }
                    if (lParam > 0)
                    {
                        m_pSmartDev->cmd = SCMD_OPEN;
                        m_pSmartDev->status = lParam;
                        m_pSmartDev->param1 = lParam;
                    }
                    else
                    {
                        m_pSmartDev->cmd = SCMD_CLOSE;
                        m_pSmartDev->status = 0;
                    }
                    show();
                    if (m_pSmartDev->status > 0)
                        SendSmartCmd(&m_pSmartDev->device, SCMD_DIMMER_OPEN, lParam);
                    else
                        SendSmartCmd(&m_pSmartDev->device, SCMD_CLOSE, 0);
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void UpdateStatus()
    {
		if (m_pSmartDev->param1 == 0)
		{
			m_pSmartDev->param1 = 100;
		}
		
        if (m_pSmartDev->cmd == SCMD_CLOSE)
        {
            m_pSmartDev->status = 0;
        }
        else if (m_pSmartDev->cmd == SCMD_OPEN)
        {
            if (m_pSmartDev->status == 0)
                m_pSmartDev->status = m_pSmartDev->param1;
			else
				m_pSmartDev->param1 = m_pSmartDev->status;
        }
		else
		{
			 m_pSmartDev->status = m_pSmartDev->param1;
		}
        show();
    }

    void show()
    {
        sprintf(m_buf, "%d", m_pSmartDev->status);
        m_pPercent->SetSrc(m_buf);
        m_pPercent->Show(TRUE);

        m_pProgress->SetProgressCur(m_pSmartDev->status);
        m_pProgress->Show();
    }

    void OnCreate(SmartDev *pSmartDev)
    {
        m_pSmartDev = pSmartDev;
        m_pTitle->SetSrc(pSmartDev->name);
        m_pTitle->Show(TRUE);
        UpdateStatus();
        SmartGetStatus(&pSmartDev->device);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("dimmer.xml");

        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("add", &m_idAdd);
        GetCtrlByName("sub", &m_idSub);

        m_pTitle = (CDPStatic *)GetCtrlByName("title");
        m_pPercent = (CDPStatic *)GetCtrlByName("percent");
        m_pProgress = (CDPProgress *)GetCtrlByName("progress", &m_idProgress);

        OnCreate((SmartDev *)lParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idAdd;
    DWORD m_idSub;
    DWORD m_idProgress;

    CDPStatic *m_pTitle;
    CDPStatic *m_pPercent;
    CDPProgress *m_pProgress;

    SmartDev *m_pSmartDev;
    char m_buf[32];
};

CAppBase *CreateDimmerApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CDimmerApp *pApp = new CDimmerApp(wParam);
    if (!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}