#include "CCtrlModules.h"
#include "SmartConfig.h"


class CLightCfg : public CAppBase
{
public:
    CLightCfg(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CLightCfg()
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
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[0])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_STUDY_APPID, 0, 0);
                }
                else if (wParam == m_idButton[1])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_STUDY_APPID, 1, 0);
                }
                else if (wParam == m_idButton[2])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_STUDY_APPID, 2, 0);
                }
                else if (wParam == m_idButton[3])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_STUDY_APPID, 0, 1);
                }
                else if (wParam == m_idButton[4])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_STUDY_APPID, 1, 1);
                }
                else if (wParam == m_idButton[5])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_STUDY_APPID, 2, 1);
                }
                else if (wParam == m_idButton[6])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_INIT_APPID, 0, 0);
                }
                else if (wParam == m_idButton[7])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_INIT_APPID, 1, 0);
                }
                else if (wParam == m_idButton[8])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_INIT_APPID, 2, 0);
                }
                else if (wParam == m_idButton[9])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_LIST_APPID, 0, 0);
                }
                else if (wParam == m_idButton[10])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_LIST_APPID, 1, 0);
                }
                else if (wParam == m_idButton[11])
                {
                    DPPostMessage(MSG_START_FROM_OVER, PRJ_LIGHT_LIST_APPID, 2, 0);
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
        InitFrame("light_cfg.xml");
        GetCtrlByName("back", &m_idBack);

        m_pButton[0]  = (CDPButton *)GetCtrlByName("single1", &m_idButton[0]);
        m_pButton[1]  = (CDPButton *)GetCtrlByName("single2", &m_idButton[1]);
        m_pButton[2]  = (CDPButton *)GetCtrlByName("single3", &m_idButton[2]);
        m_pButton[3]  = (CDPButton *)GetCtrlByName("double1", &m_idButton[3]);
        m_pButton[4]  = (CDPButton *)GetCtrlByName("double2", &m_idButton[4]);
        m_pButton[5]  = (CDPButton *)GetCtrlByName("double3", &m_idButton[5]);
        m_pButton[6]  = (CDPButton *)GetCtrlByName("init1", &m_idButton[6]);
        m_pButton[7]  = (CDPButton *)GetCtrlByName("init2", &m_idButton[7]);
        m_pButton[8]  = (CDPButton *)GetCtrlByName("init3", &m_idButton[8]);
        m_pButton[9]  = (CDPButton *)GetCtrlByName("text1", &m_idButton[9]);
        m_pButton[10] = (CDPButton *)GetCtrlByName("text2", &m_idButton[10]);
        m_pButton[11] = (CDPButton *)GetCtrlByName("text3", &m_idButton[11]);

        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idButton[12];
    CDPButton *m_pButton[12];
};

CAppBase *CreateLightCfgApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CLightCfg *pApp = new CLightCfg(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}