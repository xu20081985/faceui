#include "CCtrlModules.h"
#include "SmartConfig.h"

class CIr_Air : public CAppBase
{
public:
    CIr_Air(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CIr_Air()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if(m_dwTimeout++ == 30)
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
                else if (wParam == m_idButton[0])
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[7]);// 风速高
                }
                else if (wParam == m_idButton[1])
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[6]);// 风速中
                }
                else if (wParam == m_idButton[2])
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[5]);// 风速低
                }
                else if (wParam == m_idButton[3])
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[4]);// 通风
                }
                else if (wParam == m_idButton[4])
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[3]);// 制冷
                }
                else if (wParam == m_idButton[5])
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[2]);// 制热
                }
                else if (wParam == m_idButton[6])
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[0]);// 开
                }
                else if (wParam == m_idButton[7])
                {
                    SendSmartCmd(&m_pSmartDev->device, SCDM_INFRARED, m_irAcCode[1]);// 关
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
        InitFrame("ir_airc.xml");
        GetCtrlByName("back", &m_idBack);
        m_pTitle = (CDPStatic *)GetCtrlByName("title");
        m_pButton[0] = (CDPButton *)GetCtrlByName("speed_h", &m_idButton[0]);
        m_pButton[1] = (CDPButton *)GetCtrlByName("speed_m", &m_idButton[1]);
        m_pButton[2] = (CDPButton *)GetCtrlByName("speed_l", &m_idButton[2]);
        m_pButton[3] = (CDPButton *)GetCtrlByName("wind", &m_idButton[3]);
        m_pButton[4] = (CDPButton *)GetCtrlByName("cold", &m_idButton[4]);
        m_pButton[5] = (CDPButton *)GetCtrlByName("hot", &m_idButton[5]);
        m_pButton[6] = (CDPButton *)GetCtrlByName("poweron", &m_idButton[6]);
        m_pButton[7] = (CDPButton *)GetCtrlByName("poweroff", &m_idButton[7]);

        // 1开 2关 3制热 4制冷 5通风 6风速低 7风速中 8风速高
        for (int i = 0; i < 8; i++)
        {
            m_irAcCode[i] = GetIR_AIR_CODE(m_irAcCode[i], i);
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
    WORD    m_irAcCode[8];
};

CAppBase *CreateIr_AirApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CIr_Air *pApp = new CIr_Air(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
