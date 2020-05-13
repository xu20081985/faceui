#include "CCtrlModules.h"

class CLightInit : public CAppBase
{
public:
    CLightInit(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CLightInit()
    {
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
                if (wParam == m_idOK)
                {
                    Show();
                    DPSleep(2000);
                    plightStudy = GetLightStudy(m_dwChan);
                    memset(plightStudy, 0, sizeof(LightStudy));
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if(wParam == m_idCancel)
                {
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void Show()
    {
        m_pStatic[1]->Show(FALSE);
        m_pStatic[0]->SetTextSize(42);
        m_pStatic[0]->SetSrc(GetStringByID(4259));
        m_pStatic[0]->Show(TRUE);
        m_pButton[0]->Hide();
        m_pButton[1]->Hide();
    }

    void OnCreate(DWORD i)
    {
        m_pStatic[0]->SetSrc(GetStringByID(4260 + i));
        m_pStatic[0]->Show(TRUE);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("light_init.xml");
        m_pButton[0] = (CDPButton *)GetCtrlByName("ok", &m_idOK);
        m_pButton[1] = (CDPButton *)GetCtrlByName("cancel", &m_idCancel);

        m_pStatic[0] = (CDPStatic *)GetCtrlByName("text1");
        m_pStatic[1] = (CDPStatic *)GetCtrlByName("icon1");
        m_dwChan = lParam;
        OnCreate(lParam);
        return TRUE;
    }

private:
    DWORD m_idOK;
    DWORD m_idCancel;
    CDPStatic *m_pStatic[2];
    CDPButton *m_pButton[2];
    DWORD m_dwChan;
    LightStudy *plightStudy;
};

CAppBase *CreateLightInitApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CLightInit *pApp = new CLightInit(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}