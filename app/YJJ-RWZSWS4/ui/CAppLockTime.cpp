#include "CCtrlModules.h"
#include "SmartConfig.h"


class CLockTime : public CAppBase
{
public:
    CLockTime(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CLockTime()
    {
    }
    void ResumeAck(void)
    {
        m_dwTimeout = 0;
        return CAppBase::ResumeAck();
    }
    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                }
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
                else if (wParam == m_idButton[0])
                {
                    SetPrjShow(m_Show[0]);
                }
                else if (wParam == m_idButton[1])
                {
                    SetPrjShow(m_Show[1]);
                }
                else if (wParam == m_idButton[2])
                {
                    SetPrjShow(m_Show[2]);
                }
                else if (wParam == m_idButton[3])
                {
                    SetPrjShow(m_Show[3]);
                }
                else
                {

                }
                OnCreate();
                break;
        }
        return TRUE;
    }

    void OnCreate()
    {
        m_screenoff = GetPrjShow();
        for (int i = 0; i < 4; i++)
        {
            if (m_Show[i] == m_screenoff)
            {
                m_pButton[i]->SetSrcpng(GetSmartPngSelect(1));
                m_pButton[i]->Show(STATUS_NORMAL);
            }
            else
            {
                m_pButton[i]->SetSrcpng(GetSmartPngSelect(0));
                m_pButton[i]->Show(STATUS_NORMAL);
            }
        }
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("lock_time.xml");
        GetCtrlByName("back", &m_idBack);

        m_pButton[0] = (CDPButton *)GetCtrlByName("select1", &m_idButton[0]);
        m_pButton[1] = (CDPButton *)GetCtrlByName("select2", &m_idButton[1]);
        m_pButton[2] = (CDPButton *)GetCtrlByName("select3", &m_idButton[2]);
        m_pButton[3] = (CDPButton *)GetCtrlByName("select4", &m_idButton[3]);

        m_Show[0] = 60;
        m_Show[1] = 300;
        m_Show[2] = 600;
        m_Show[3] = 0;

        OnCreate();
        return TRUE;
    }

private:

    DWORD m_idBack;
    DWORD m_idButton[4];
    CDPButton *m_pButton[4];
    DWORD m_Show[4];
};

CAppBase *CreateLockTime(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CLockTime *pApp = new CLockTime(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}