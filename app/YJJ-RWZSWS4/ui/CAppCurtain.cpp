#include "CCtrlModules.h"

class CCurtainApp : public CAppBase
{
public:
    CCurtainApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CCurtainApp()
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
                else if (wParam == m_idOpen)
                {
                    m_pSmartDev->status = 100;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    UpdateStatus();
                    SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_OPEN, m_pSmartDev->status);
                }
                else if(wParam == m_idHalf)
                {
                    m_pSmartDev->status = 50;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    UpdateStatus();
                    SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_OPEN, m_pSmartDev->status);
                }
                else if(wParam == m_idClose)
                {
                    m_pSmartDev->status = 0;
                    m_pSmartDev->cmd = SCMD_CLOSE;
                    UpdateStatus();
                    SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_CLOSE, m_pSmartDev->status);
                }
                else if (wParam == m_idCurtains)
                {
                    if (m_pSmartDev->status > 0)
                    {
                        m_pSmartDev->status -= 10;
                        m_pSmartDev->cmd = SCMD_OPEN;
						if (m_pSmartDev->status <= 0)
                        {
                            m_pSmartDev->status = 0;
                            m_pSmartDev->cmd = SCMD_CLOSE;
                        }
                    }
                    else
                    {
                        m_pSmartDev->status = 0;
                        m_pSmartDev->cmd = SCMD_CLOSE;
                    }
                    UpdateStatus();
                    if (m_pSmartDev->status > 0)
                        SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_OPEN, m_pSmartDev->status);
                    else
                        SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_CLOSE, m_pSmartDev->status);
                }
                else if (wParam == m_idCurtaina)
                {
                    if (m_pSmartDev->status < 100)
                    {
                        m_pSmartDev->status += 10;
                        if (m_pSmartDev->status >= 100)
                            m_pSmartDev->status = 100;
                        m_pSmartDev->cmd = SCMD_OPEN;
                    }
                    else
                    {
                        m_pSmartDev->status = 100;
                        m_pSmartDev->cmd = SCMD_OPEN;
                    }
                    UpdateStatus();
                    SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_OPEN, m_pSmartDev->status);
                }
                else if (wParam == m_idProgress)
                {
                    if (zParam == 1)
                    {
                        m_pProgress->SetProgressCur(lParam);
                        m_pProgress->Show();
                        break;
                    }
                    m_pSmartDev->status = lParam;
                    if (m_pSmartDev->status > 0)
                        m_pSmartDev->cmd = SCMD_OPEN;
                    else
                        m_pSmartDev->cmd = SCMD_CLOSE;
                    if (m_pSmartDev->status == 0
                            || m_pSmartDev->status == 50
                            || m_pSmartDev->status == 100)
                    {
                        m_showStatus = TRUE;
                        UpdateStatus();
                    }
                    if (m_pSmartDev->status > 0
                            && m_pSmartDev->status < 100
                            && m_pSmartDev->status != 50)
                    {
                        if (m_showStatus == TRUE)
                        {
                            UpdateStatus();
                        }
                    }
                    if (m_pSmartDev->status > 0)
                        SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_OPEN, m_pSmartDev->status);
                    else
                        SendSmartCmd(&m_pSmartDev->device, SCMD_CURTAIN_CLOSE, m_pSmartDev->status);
                }
                break;
        }
        return TRUE;
    }

    void UpdateStatus()
    {
		if (m_pSmartDev->cmd == SCMD_OPEN && m_pSmartDev->status == 0)
		{
			m_pSmartDev->status = 100;
		}
		
        if (m_pSmartDev->status == 0)
        {
            m_pClose->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_ALL_CLOSE, ICON_ON_NORMAL));
            m_pClose->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_ALL_CLOSE, ICON_ON_LITTLE));
            m_pHalf->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_HALF_OPEN, ICON_OFF_NORMAL));
            m_pHalf->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_HALF_OPEN, ICON_OFF_LITTLE));
            m_pOpen->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_ALL_OPEN, ICON_OFF_NORMAL));
            m_pOpen->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_ALL_OPEN, ICON_OFF_LITTLE));
            m_showStatus = TRUE;
        }
        else if (m_pSmartDev->status == 50)
        {
            m_pClose->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_ALL_CLOSE, ICON_OFF_NORMAL));
            m_pClose->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_ALL_CLOSE, ICON_OFF_LITTLE));
            m_pHalf->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_HALF_OPEN, ICON_ON_NORMAL));
            m_pHalf->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_HALF_OPEN, ICON_ON_LITTLE));
            m_pOpen->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_ALL_OPEN, ICON_OFF_NORMAL));
            m_pOpen->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_ALL_OPEN, ICON_OFF_LITTLE));
            m_showStatus = TRUE;
        }
        else if (m_pSmartDev->status == 100)
        {
            m_pClose->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_ALL_CLOSE, ICON_OFF_NORMAL));
            m_pClose->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_ALL_CLOSE, ICON_OFF_LITTLE));
            m_pHalf->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_HALF_OPEN, ICON_OFF_NORMAL));
            m_pHalf->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_HALF_OPEN, ICON_OFF_LITTLE));
            m_pOpen->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_ALL_OPEN, ICON_ON_NORMAL));
            m_pOpen->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_ALL_OPEN, ICON_ON_LITTLE));
            m_showStatus = TRUE;
        }
        else
        {
            m_pClose->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_ALL_CLOSE, ICON_OFF_NORMAL));
            m_pClose->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_ALL_CLOSE, ICON_OFF_LITTLE));
            m_pHalf->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_HALF_OPEN, ICON_OFF_NORMAL));
            m_pHalf->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_HALF_OPEN, ICON_OFF_LITTLE));
            m_pOpen->SetSrcpng(STATUS_NORMAL, GetSmartPngCurtain(CURTAIN_ALL_OPEN, ICON_OFF_NORMAL));
            m_pOpen->SetSrcpng(STATUS_PRESSED, GetSmartPngCurtain(CURTAIN_ALL_OPEN, ICON_OFF_LITTLE));
            m_showStatus = FALSE;
        }
        m_pClose->Show(STATUS_NORMAL);
        m_pHalf->Show(STATUS_NORMAL);
        m_pOpen->Show(STATUS_NORMAL);

        m_pProgress->SetProgressCur(m_pSmartDev->status);
        m_pProgress->Show();
    }

    void OnCreate(SmartDev *pSmartDev)
    {
        m_pSmartDev = pSmartDev;
        m_pTitle->SetSrc(m_pSmartDev->name);
        m_pTitle->Show(TRUE);
        UpdateStatus();
        SmartGetStatus(&m_pSmartDev->device);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("curtain.xml");

        GetCtrlByName("back", &m_idBack);
        m_pTitle = (CDPStatic *)GetCtrlByName("title");
        m_pPercent = (CDPStatic *)GetCtrlByName("percent");
        m_pProgress = (CDPProgress *)GetCtrlByName("progress", &m_idProgress);

        m_pClose = (CDPButton *)GetCtrlByName("close", &m_idClose);
        m_pHalf = (CDPButton *)GetCtrlByName("half", &m_idHalf);
        m_pOpen = (CDPButton *)GetCtrlByName("open", &m_idOpen);
        m_pCurtaina = (CDPButton *)GetCtrlByName("add", &m_idCurtaina);
        m_pCurtains = (CDPButton *)GetCtrlByName("sub", &m_idCurtains);

        OnCreate((SmartDev *)lParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idHalf;
    DWORD m_idOpen;
    DWORD m_idClose;
    DWORD m_idProgress;
    DWORD m_idCurtaina;
    DWORD m_idCurtains;

    CDPStatic *m_pTitle;
    CDPStatic *m_pPercent;
    CDPButton *m_pHalf;
    CDPButton *m_pOpen;
    CDPButton *m_pClose;
    CDPButton *m_pCurtaina;
    CDPButton *m_pCurtains;
    CDPProgress *m_pProgress;

    SmartDev *m_pSmartDev;
    BOOL m_showStatus;
    char m_buf[64];
};

CAppBase *CreateCurtainApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CCurtainApp *pApp = new CCurtainApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
