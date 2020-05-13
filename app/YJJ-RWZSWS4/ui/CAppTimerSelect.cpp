#include "CCtrlModules.h"

class CTimerSelectApp : public CAppBase
{
public:
    CTimerSelectApp (DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CTimerSelectApp ()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                break;
            case TOUCH_MESSAGE:

                if(wParam == m_idBack)
                {
                    DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_APPID, 0, 0);
                    DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
                }



                break;

            case TOUCH_SLIDE:
                if(wParam == SLIDE_DOWN)
                {

                    for(int i = 0; i < 4; i++)
                    {

                        m_pTime[i]->Show(FALSE);
                        m_pAction[i]->Show(FALSE);
                        m_pEdit[i]->Show(STATUS_UNACK);
                    }

                    OnCreate((m_dwPage + 3) % 4);
                }
                if(wParam == SLIDE_UPSIDE)
                {

                    for(int i = 0; i < 4; i++)
                    {

                        m_pTime[i]->Show(FALSE);
                        m_pAction[i]->Show(FALSE);
                        m_pEdit[i]->Show(STATUS_UNACK);
                    }

                    OnCreate((m_dwPage + 1) % 4);
                }
                break;
        }
        return TRUE;
    }

    void OnCreate(int page)
    {

        m_dwPage = page;
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("timer_select.xml");
        GetCtrlByName("back", &m_idBack);

        m_pTime[0] = (CDPStatic *)GetCtrlByName("time_1");
        m_pTime[1] = (CDPStatic *)GetCtrlByName("time_2");
        m_pTime[2] = (CDPStatic *)GetCtrlByName("time_3");
        m_pTime[3] = (CDPStatic *)GetCtrlByName("time_4");

        m_pAction[0] = (CDPStatic *)GetCtrlByName("action_1");
        m_pAction[1] = (CDPStatic *)GetCtrlByName("action_2");
        m_pAction[2] = (CDPStatic *)GetCtrlByName("action_3");
        m_pAction[3] = (CDPStatic *)GetCtrlByName("action_4");

        m_pEdit[0] = (CDPButton *)GetCtrlByName("switch_1", &m_idEdit[0]);
        m_pEdit[1] = (CDPButton *)GetCtrlByName("switch_2", &m_idEdit[1]);
        m_pEdit[2] = (CDPButton *)GetCtrlByName("switch_3", &m_idEdit[2]);
        m_pEdit[3] = (CDPButton *)GetCtrlByName("switch_4", &m_idEdit[3]);

        OnCreate(0);
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idEdit[4];
    DWORD m_dwPage;

    CDPStatic *m_pTime[4];          //定时事件的时间摆放位置
    CDPStatic *m_pAction[4];        //定时时间执行动作摆放位置
    CDPButton *m_pEdit[4];          //定时事件编辑
};

CAppBase *CreateTimerSelectApp (DWORD wParam, DWORD lParam, DWORD zParam)
{
    CTimerSelectApp *pApp = new CTimerSelectApp (wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
