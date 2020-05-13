#include "CCtrlModules.h"

class CTimerEditApp : public CAppBase
{
public:
    CTimerEditApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CTimerEditApp()
    {
    }

    void ResumeAck(void)
    {
        m_dwTimeout = 0;
        OnCreate(m_pItem);
        return CAppBase::ResumeAck();
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch (uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                {
                    memcpy(m_pItem, &timer, sizeof(SmartTimer));
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
                    memcpy(m_pItem, &timer, sizeof(SmartTimer));
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_TIMER_APPID, m_dwPage, 0);
                }
                else if (wParam == m_idButton[0])
                {
                    DPPostMessage(MSG_START_FROM_OVER, TIMER_TIME_APPID, (DWORD)m_pItem, 0);
                }
                else if (wParam == m_idButton[1])
                {
                    DPPostMessage(MSG_START_FROM_OVER, TIMER_WEEK_APPID, (DWORD)m_pItem, 0);
                }
                else if (wParam == m_idSave)
                {
                    UpdatSmartTimerSet();
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_TIMER_APPID, m_dwPage, 0);
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void OnCreate(PSmartTimer pItem)
    {
        m_pStatic[0]->SetSrc(pItem->timeStr);
        m_pStatic[0]->Show(TRUE);

        m_pStatic[1]->SetSrc(pItem->weekStr);
        m_pStatic[1]->Show(TRUE);

        m_pStatic[2]->SetSrc(pItem->devStr);
        m_pStatic[2]->Show(TRUE);

        m_pStatic[3]->SetSrc(pItem->wayStr);
        m_pStatic[3]->Show(TRUE);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("timer_edit.xml");

        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("save", &m_idSave);
        m_pButton[0] = (CDPButton *)GetCtrlByName("next1", &m_idButton[0]);
        m_pButton[1] = (CDPButton *)GetCtrlByName("next2", &m_idButton[1]);
        m_pButton[2] = (CDPButton *)GetCtrlByName("next3", &m_idButton[2]);
        m_pButton[3] = (CDPButton *)GetCtrlByName("next4", &m_idButton[3]);

        m_pStatic[0] = (CDPStatic *)GetCtrlByName("time");
        m_pStatic[1] = (CDPStatic *)GetCtrlByName("week");
        m_pStatic[2] = (CDPStatic *)GetCtrlByName("object");
        m_pStatic[3] = (CDPStatic *)GetCtrlByName("action");

        m_pItem = (PSmartTimer)lParam;
        m_dwPage = zParam;
        memcpy(&timer, m_pItem, sizeof(SmartTimer));
        OnCreate((PSmartTimer)lParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idSave;
    DWORD m_idButton[4];

    CDPButton *m_pButton[4];
    CDPStatic *m_pStatic[4];
    PSmartTimer m_pItem;
    DWORD m_dwPage;
    SmartTimer timer;
};

CAppBase *CreateTimerEditApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CTimerEditApp *pApp = new CTimerEditApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
