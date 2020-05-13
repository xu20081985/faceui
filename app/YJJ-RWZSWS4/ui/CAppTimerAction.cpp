#include "CCtrlModules.h"

class CTimerActionApp : public CAppBase
{
public:
    CTimerActionApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CTimerActionApp()
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

                }


                break;

        }
        return TRUE;
    }

    void OnPage(DWORD dwPage)
    {


        m_dwPage = dwPage;
    }

    void OnCreate()
    {
        OnPage(0);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("timer_action.xml");

        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("ctl_action1", &m_idAction_1);
        GetCtrlByName("ctl_action2", &m_idAction_2);

        m_pAction[0] = (CDPStatic *)GetCtrlByName("action_1");
        m_pAction[1] = (CDPStatic *)GetCtrlByName("action_2");
        m_pAction[2] = (CDPStatic *)GetCtrlByName("action_3");
        m_pAction[3] = (CDPStatic *)GetCtrlByName("action_4");

        m_pSelect[0] = (CDPStatic *)GetCtrlByName("select_1");
        m_pSelect[1] = (CDPStatic *)GetCtrlByName("select_2");

        OnCreate();
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idAction_1;                    //动作控制变量1
    DWORD m_idAction_2;                    //动作控制变量2
    CDPStatic *m_pAction[4];
    CDPStatic *m_pSelect[2];               //勾选动作

    DWORD m_dwPage;
};

CAppBase *CreateTimerActionApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CTimerActionApp *pApp = new CTimerActionApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}