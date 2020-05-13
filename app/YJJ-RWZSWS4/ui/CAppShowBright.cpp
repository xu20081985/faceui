#include "CCtrlModules.h"

class CShowBright : public CAppBase
{
public:
    CShowBright(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CShowBright()
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
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_SHOW_APPID, 0, 0);
                }
                else if (wParam == m_idProgress)
                {
					#if 0
                    if (zParam == 1)
                    {
                        sprintf(m_buf, "%d", lParam);
                        m_pPercent->SetSrc(m_buf);
                        m_pPercent->Show(TRUE);
                        m_pProgress->SetProgressCur(lParam);
                        m_pProgress->Show();
                        break;
                    }
					#endif
                    SetShowBright(lParam);
                    show();
                    if (m_showBright < 1)
                        AdjustScreen(1, 50, 50);
                    else
                        AdjustScreen(m_showBright, 50, 50);
                }
                else if (wParam == m_idAdd)
                {
                    if (m_showBright < 90)
                        m_showBright += 10;
                    else
                        m_showBright = 100;
                    SetShowBright(m_showBright);
                    show();
                    AdjustScreen(m_showBright, 50, 50);
                }
                else if (wParam == m_idSub)
                {
                    if (m_showBright > 10)
                        m_showBright -= 10;
                    else
                        m_showBright = 0;
                    SetShowBright(m_showBright);
                    show();
                    if (m_showBright < 1)
                        AdjustScreen(1, 50, 50);
                    else
                        AdjustScreen(m_showBright, 50, 50);
                }
                break;
        }
        return TRUE;
    }

    void show()
    {
        m_showBright = GetShowBright();
        sprintf(m_buf, "%d", m_showBright);
        m_pPercent->SetSrc(m_buf);
        m_pPercent->Show(TRUE);
        m_pProgress->SetProgressCur(m_showBright);
        m_pProgress->Show();
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("show_bright.xml");

        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("add", &m_idAdd);
        GetCtrlByName("sub", &m_idSub);

        m_pPercent = (CDPStatic *)GetCtrlByName("percent");
        m_pProgress = (CDPProgress *)GetCtrlByName("progress", &m_idProgress);

        show();
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idAdd;
    DWORD m_idSub;
    DWORD m_idProgress;

    CDPStatic *m_pPercent;
    CDPProgress *m_pProgress;
    DWORD m_showBright;
    char m_buf[32];
};

CAppBase *CreateShowBright(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CShowBright *pApp = new CShowBright(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}