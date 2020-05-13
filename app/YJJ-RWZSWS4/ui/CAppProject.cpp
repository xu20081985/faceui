#include "CCtrlModules.h"
#include "SmartConfig.h"

class CProjectApp : public CAppBase
{
public:
    CProjectApp(DWORD hWnd) : CAppBase(hWnd)
    {

    }

    ~CProjectApp()
    {

    }

    void ResumeAck(void)
    {
        m_dwTimeout = 0;
        OnPage(m_nPage);
        return CAppBase::ResumeAck();
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if(m_dwTimeout++ == 30)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                }
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)    									// 返回
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                }
                else if (wParam == m_idButton[0])   						// 定时
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_TIMER_APPID, 0, 0);
                }
                else if (wParam == m_idButton[1])    						// 本机设置
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[2])    						// 上报ID
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_REPORT_APPID, 0, 0);
                }
                else if (wParam == m_idButton[3])    						// 恢复出厂
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_RESET_APPID, 0, 0);
                }
                else if (wParam == m_idButton[4])    						// 关于本机
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_ABOUT_APPID, 0, 0);
                }
                else
                {
                }
                break;
            default:
                break;
        }

        return TRUE;
    }

    void OnPage(DWORD nPage)
    {

    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("project.xml");

        GetCtrlByName("back", &m_idBack);

        m_pButton[0] = (CDPButton *)GetCtrlByName("next1", &m_idButton[0]);
        m_pButton[1] = (CDPButton *)GetCtrlByName("next2", &m_idButton[1]);
        m_pButton[2] = (CDPButton *)GetCtrlByName("next3", &m_idButton[2]);
        m_pButton[3] = (CDPButton *)GetCtrlByName("next4", &m_idButton[3]);
        m_pButton[4] = (CDPButton *)GetCtrlByName("next5", &m_idButton[4]);

        OnPage(lParam);

        return TRUE;
    }

private:
    DWORD m_nPage;

    DWORD m_idBack;

    //DWORD m_idEmpty;
    //DWORD m_dwCount;
    DWORD m_idButton[10];
    CDPButton *m_pButton[10];
};

CAppBase *CreateProjectApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CProjectApp *pApp = new CProjectApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}