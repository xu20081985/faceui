#include "CCtrlModules.h"
#include "SmartConfig.h"

class CTvApp : public CAppBase
{
public:
    CTvApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CTvApp()
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
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if(wParam == m_idButton[0])       // 开关机
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[0]);
                }
                else if(wParam == m_idButton[1])       // 频道+
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[1]);
                }
                else if(wParam == m_idButton[2])       // 频道-
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[2]);
                }
                else if(wParam == m_idButton[3])       // 声音+
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[3]);
                }
                else if(wParam == m_idButton[4])       // 声音-
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[4]);
                }
                else if(wParam == m_idButton[5])       // TV/AV
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[5]);
                }
                break;
        }

        return TRUE;
    }

    void OnCreate(SmartDev *pSmartDev)
    {
        m_pTitle->SetSrc(pSmartDev->name);
        m_pTitle->Show(TRUE);

        m_pSmartDev = pSmartDev;
    }


    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("ir_tv.xml");
        GetCtrlByName("back", &m_idBack);
        m_pTitle = (CDPStatic *)GetCtrlByName("title");
        m_pButton[0] = (CDPButton *)GetCtrlByName("power", &m_idButton[0]);
        m_pButton[1] = (CDPButton *)GetCtrlByName("chanadd", &m_idButton[1]);
        m_pButton[2] = (CDPButton *)GetCtrlByName("chansub", &m_idButton[2]);
        m_pButton[3] = (CDPButton *)GetCtrlByName("voladd", &m_idButton[3]);
        m_pButton[4] = (CDPButton *)GetCtrlByName("volsub", &m_idButton[4]);
        m_pButton[5] = (CDPButton *)GetCtrlByName("mode", &m_idButton[5]);

        // 1开关 2频道+ 3频道- 4音量+ 5音量- 6模式
        for (int i = 0; i < 6; i++)
        {
            m_irAcCode[i] = GetIR_TV_CODE(m_irAcCode[i], i);
        }

        OnCreate((SmartDev *)lParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    CDPStatic *m_pTitle;

    DWORD m_idButton[10];
    CDPButton *m_pButton[10];
    SmartDev *m_pSmartDev;
    WORD    m_irAcCode[7];
};



CAppBase *CreateTVApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CTvApp *pApp = new CTvApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
