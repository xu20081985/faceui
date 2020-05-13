#include "CCtrlModules.h"
#include "SmartConfig.h"


class CShow : public CAppBase
{
public:
    CShow(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CShow()
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
                if (wParam == m_idBack)    									// 返回
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[0])   						// 屏保时间
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_LOCK_TIME_APPID, 0, 0);
                }
                else if (wParam == m_idButton[1])    						// 屏幕亮度
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_SHOW_BRIGHT_APPID, 0, 0);
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

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("show.xml");
        GetCtrlByName("back", &m_idBack);
        m_pButton[0] = (CDPButton *)GetCtrlByName("next1", &m_idButton[0]);
        m_pButton[1] = (CDPButton *)GetCtrlByName("next2", &m_idButton[1]);

        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idButton[10];
    CDPButton *m_pButton[10];
};

CAppBase *CreatePrjShow(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CShow *pApp = new CShow(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}