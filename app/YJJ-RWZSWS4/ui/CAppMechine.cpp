#include "CCtrlModules.h"
#include "SmartConfig.h"

class CMechine : public CAppBase
{
public:
    CMechine(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CMechine()
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
                if (wParam == m_idBack)    									// ·µ»Ø
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PROJECT_APPID, 0, 0);
                }
                else if (wParam == m_idButton[0])   						// ÃÜÂë
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_PWD_APPID, 0, 0);
                }
                else if (wParam == m_idButton[1])    						// Ê±ÖÓ
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_DATE_APPID, 0, 0);
                }
                else if (wParam == m_idButton[2])    						// ±³¾°
                {
                    DPPostMessage(MSG_START_FROM_ROOT, BKGD_SET, 0, 0);
                }
                else if (wParam == m_idButton[3])    						// ÏÔÊ¾
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_SHOW_APPID, 0, 0);
                }
                else if (wParam == m_idButton[4])    						// ÅäÖÃ
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_LIGHT_CFG_APPID, 0, 0);
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
        InitFrame("mechine.xml");

        GetCtrlByName("back", &m_idBack);

        m_pButton[0] = (CDPButton *)GetCtrlByName("next1", &m_idButton[0]);
        m_pButton[1] = (CDPButton *)GetCtrlByName("next2", &m_idButton[1]);
        m_pButton[2] = (CDPButton *)GetCtrlByName("next3", &m_idButton[2]);
        m_pButton[3] = (CDPButton *)GetCtrlByName("next4", &m_idButton[3]);
        m_pButton[4] = (CDPButton *)GetCtrlByName("next5", &m_idButton[4]);

        return TRUE;
    }

private:

    DWORD m_nPage;     // Ò³Êý

    DWORD m_idBack;
    DWORD m_idButton[10];
    CDPButton *m_pButton[10];
};

CAppBase *CreateMechineApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CMechine *pApp = new CMechine(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
